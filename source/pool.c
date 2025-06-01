// File: src/pool.c
//
// Simple mining pool backend for Coin Archana Berry (CAB).
// - Menjalankan TCP server di port tertentu (default 3333).
// - Untuk tiap koneksi: terima "HELLO <miner_name>"
//   → Kirim job: "JOB <index> <timestamp> <prev_hash> <difficulty>\n"
//   → Terima "SOLN <nonce>\n"
//   → Verifikasi hash; jika benar, simpan blok, kirim "ACCEPT <index> <hash>\n"
//     jika salah, kirim "REJECT\n"
//   → Ulangi kirim JOB berikutnya.
// - Setiap miner dijalankan di proses terpisah (fork) agar bisa multi‐client.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include "cabcrypto.h"

#define BACKLOG      10
#define DEFAULT_PORT "3333"
#define HASH_STR_LEN 65
#define CHAIN_FILE   "data/chain.dat"
#define DATA_MAX_LEN 256
#define DIFFICULTY   2

typedef struct {
    int index;
    long timestamp;
    char data[DATA_MAX_LEN];
    char prev_hash[HASH_STR_LEN];
    int nonce;
    char hash[HASH_STR_LEN];
} Block;

// Prototipe fungsi
static void handle_client(int client_fd);
static void init_chain_if_needed();
static int get_last_block_info(int *last_index, char prev_hash_out[HASH_STR_LEN]);
static void calculate_block_hash(int index, long timestamp, const char *data,
                                 const char *prev_hash, int nonce, char out_hash[HASH_STR_LEN]);
static int hash_meets_difficulty(const char *hash);
static int append_block_to_file(int index, long timestamp, int nonce,
                                const char *data, const char *prev_hash, const char *hash);

// Main pool server
int main(int argc, char *argv[]) {
    const char *port = DEFAULT_PORT;
    if (argc == 2) {
        port = argv[1];
    }

    // Pastikan direktori data ada dan chain dibuat (jika belum ada)
    init_chain_if_needed();

    struct addrinfo hints, *res, *p;
    int sockfd, yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 atau IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;      // bind ke semua interface

    if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Bind ke address yang tersedia
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("pool: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("pool: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "pool: gagal bind ke port %s\n", port);
        return 2;
    }

    freeaddrinfo(res);

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Pool server berjalan di port %s\n", port);

    // Tangani SIGCHLD agar tidak menghasilkan zombie jika child selesai
    signal(SIGCHLD, SIG_IGN);

    // Loop menerima koneksi
    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t sin_size = sizeof client_addr;
        int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        // Fork agar tiap client di-handle oleh proses terpisah
        pid_t pid = fork();
        if (pid == 0) {
            // Proses anak: tutup listener, handle client, lalu exit
            close(sockfd);
            handle_client(client_fd);
            close(client_fd);
            exit(0);
        } else if (pid < 0) {
            perror("fork");
        }
        // Proses parent: tutup client_fd dan kembali accept
        close(client_fd);
    }

    return 0;
}

// Inisialisasi blockchain jika belum ada file chain.dat
static void init_chain_if_needed() {
    struct stat st;
    if (stat("data", &st) == -1) {
        mkdir("data", 0755);
    }
    if (stat(CHAIN_FILE, &st) == -1) {
        // File belum ada → buat genesis block
        // Proses pembuatan genesis: index=0, data="Genesis Block", prev_hash=all '0'
        Block genesis;
        genesis.index = 0;
        genesis.timestamp = time(NULL);
        strncpy(genesis.data, "Genesis Block", DATA_MAX_LEN - 1);
        genesis.data[DATA_MAX_LEN - 1] = '\0';
        memset(genesis.prev_hash, '0', HASH_STR_LEN - 1);
        genesis.prev_hash[HASH_STR_LEN - 1] = '\0';
        genesis.nonce = 0;
        do {
            genesis.nonce++;
            calculate_block_hash(genesis.index, genesis.timestamp, genesis.data,
                                 genesis.prev_hash, genesis.nonce, genesis.hash);
        } while (!hash_meets_difficulty(genesis.hash));
        // Simpan ke file
        FILE *fp = fopen(CHAIN_FILE, "w");
        if (!fp) {
            perror("Error menulis genesis ke chain.dat");
            exit(1);
        }
        fprintf(fp, "%d|%ld|%d|%s|%s|%s\n",
                genesis.index,
                genesis.timestamp,
                genesis.nonce,
                genesis.data,
                genesis.prev_hash,
                genesis.hash);
        fclose(fp);
        printf("Genesis block dibuat, hash: %s\n", genesis.hash);
    } else {
        printf("Blockchain (chain.dat) sudah ada, lanjutkan.\n");
    }
}

// Handler untuk masing-masing miner (client_fd sudah ter‐connect)
static void handle_client(int client_fd) {
    char line[512];
    char miner_name[128] = {0};

    // Baca baris pertama: harus "HELLO <miner_name>\n"
    ssize_t len = recv(client_fd, line, sizeof line - 1, 0);
    if (len <= 0) return;
    line[len] = '\0';
    if (sscanf(line, "HELLO %127s\n", miner_name) != 1) {
        // Format tidak sesuai, langsung tutup
        return;
    }
    printf("Miner '%s' terkoneksi.\n", miner_name);

    while (1) {
        // Ambil info blok terakhir
        int last_index;
        char last_hash[HASH_STR_LEN];
        get_last_block_info(&last_index, last_hash);
        int new_index = last_index + 1;
        long timestamp = time(NULL);

        // Kirim JOB: "JOB <index> <timestamp> <prev_hash> <difficulty>\n"
        char job_msg[512];
        snprintf(job_msg, sizeof job_msg,
                 "JOB %d %ld %s %d\n",
                 new_index, timestamp, last_hash, DIFFICULTY);
        if (send(client_fd, job_msg, strlen(job_msg), 0) == -1) {
            perror("send JOB");
            break;
        }

        // Tunggu SOLN: "SOLN <nonce>\n"
        len = recv(client_fd, line, sizeof line - 1, 0);
        if (len <= 0) break;
        line[len] = '\0';

        int nonce;
        if (sscanf(line, "SOLN %d\n", &nonce) != 1) {
            // Format salah → abaikan dan kirim JOB lagi
            continue;
        }

        // Verifikasi hash
        char data_field[DATA_MAX_LEN];
        snprintf(data_field, sizeof data_field, "Mined_by_%s", miner_name); 
        char computed_hash[HASH_STR_LEN];
        calculate_block_hash(new_index, timestamp, data_field, last_hash, nonce, computed_hash);

        if (hash_meets_difficulty(computed_hash)) {
            // Valid → append ke file
            if (!append_block_to_file(new_index, timestamp, nonce, data_field, last_hash, computed_hash)) {
                perror("append_block_to_file");
                break;
            }
            // Kirim ACCEPT <index> <hash>\n
            char accept_msg[512];
            snprintf(accept_msg, sizeof accept_msg,
                     "ACCEPT %d %s\n", new_index, computed_hash);
            send(client_fd, accept_msg, strlen(accept_msg), 0);
            printf("Miner '%s' berhasil menambang block #%d (hash: %s)\n",
                   miner_name, new_index, computed_hash);
        } else {
            // Tidak valid → kirim REJECT\n
            send(client_fd, "REJECT\n", 7, 0);
        }
        // Loop kembali untuk JOB berikutnya
    }

    printf("Miner '%s' terputus.\n", miner_name);
}

// Membaca index dan prev_hash dari baris terakhir file chain.dat
// Jika file tidak ada atau kosong, set *last_index=-1 dan prev_hash = "00...0"
static int get_last_block_info(int *last_index, char prev_hash_out[HASH_STR_LEN]) {
    FILE *fp = fopen(CHAIN_FILE, "r");
    if (!fp) {
        *last_index = -1;
        memset(prev_hash_out, '0', HASH_STR_LEN - 1);
        prev_hash_out[HASH_STR_LEN - 1] = '\0';
        return 0;
    }

    char line[1024], last_line[1024] = {0};
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) > 5) {
            strcpy(last_line, line);
        }
    }
    fclose(fp);

    if (strlen(last_line) == 0) {
        *last_index = -1;
        memset(prev_hash_out, '0', HASH_STR_LEN - 1);
        prev_hash_out[HASH_STR_LEN - 1] = '\0';
        return 0;
    }

    // Format: index|timestamp|nonce|data|prev_hash|hash\n
    char *token = strtok(last_line, "|");
    *last_index = atoi(token);
    token = strtok(NULL, "|");  // timestamp
    token = strtok(NULL, "|");  // nonce
    token = strtok(NULL, "|");  // data
    token = strtok(NULL, "|");  // prev_hash
    strncpy(prev_hash_out, token, HASH_STR_LEN - 1);
    prev_hash_out[HASH_STR_LEN - 1] = '\0';
    return 1;
}

// Hitung SHA-256 untuk blok
static void calculate_block_hash(int index, long timestamp, const char *data,
                                 const char *prev_hash, int nonce, char out_hash[HASH_STR_LEN]) {
    char buffer[512];
    snprintf(buffer, sizeof buffer, "%d%ld%s%s%d", index, timestamp, data, prev_hash, nonce);
    sha256(buffer, out_hash);
    out_hash[HASH_STR_LEN - 1] = '\0';
}

// Cek apakah hash memenuhi DIFFICULTY
static int hash_meets_difficulty(const char *hash) {
    for (int i = 0; i < DIFFICULTY; i++) {
        if (hash[i] != '0') return 0;
    }
    return 1;
}

// Append blok ke file chain.dat
static int append_block_to_file(int index, long timestamp, int nonce,
                                const char *data, const char *prev_hash, const char *hash) {
    struct stat st;
    if (stat("data", &st) == -1) {
        mkdir("data", 0755);
    }
    FILE *fp = fopen(CHAIN_FILE, "a");
    if (!fp) return 0;
    fprintf(fp, "%d|%ld|%d|%s|%s|%s\n",
            index, timestamp, nonce, data, prev_hash, hash);
    fclose(fp);
    return 1;
}
