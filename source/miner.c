// File: src/miner.c
//
// Dual‐mode miner untuk Coin Archana Berry (CAB):
// 1) Solo mining (tanpa pool): fungsi mine_local(...)
// 2) Pool mining: fungsi mine_pool(...)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <sys/stat.h>
#include "cabcrypto.h"
#include "miner.h"   // Deklarasi mine_local dan mine_pool

#define HASH_STR_LEN   65
#define CHAIN_FILE     "data/chain.dat"
#define DATA_MAX_LEN   256

// ===============================
// 1) SOLO MINING (mine_local)
// ===============================

// Hitung hash SHA-256 untuk blok lokal
static void calculate_hash_local(int index, long timestamp, const char *data,
                                 const char *prev_hash, int nonce, char out_hash[HASH_STR_LEN]) {
    char buffer[512];
    snprintf(buffer, sizeof buffer, "%d%ld%s%s%d", index, timestamp, data, prev_hash, nonce);
    sha256(buffer, out_hash);
    out_hash[HASH_STR_LEN - 1] = '\0';
}

// Cek apakah hash memenuhi kesulitan lokal
static int hash_meets_difficulty_local(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; i++) {
        if (hash[i] != '0') return 0;
    }
    return 1;
}

// Baca info blok terakhir (solo)
static int get_last_block_info_local(int *last_index, char prev_hash_out[HASH_STR_LEN]) {
    FILE *fp = fopen(CHAIN_FILE, "r");
    if (!fp) {
        *last_index = -1;
        memset(prev_hash_out, '0', HASH_STR_LEN - 1);
        prev_hash_out[HASH_STR_LEN - 1] = '\0';
        return 0;
    }
    char line[1024], last_line[1024] = {0};
    while (fgets(line, sizeof line, fp) != NULL) {
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

// Append blok baru ke file (solo)
static int append_block_to_file_local(int index, long timestamp, int nonce,
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

// Solo‐mining: dipanggil dari main.c saat perintah "mine <data>"
void mine_local(const char *data) {
    int last_index;
    char last_hash[HASH_STR_LEN];
    get_last_block_info_local(&last_index, last_hash);
    int new_index = last_index + 1;
    long timestamp = time(NULL);

    char prev_hash[HASH_STR_LEN];
    strncpy(prev_hash, last_hash, HASH_STR_LEN);
    prev_hash[HASH_STR_LEN - 1] = '\0';

    int difficulty = 2; // bisa diubah sesuai kebutuhan
    int nonce = 0;
    char hash[HASH_STR_LEN];

    unsigned long long hash_count = 0;
    time_t start_time = time(NULL);
    time_t last_report = start_time;
    unsigned int accept = 0, reject = 0;

    printf("Starting local mining on data: \"%s\"\n", data);
    printf("Target difficulty: %d leading zero(s)\n", difficulty);

    while (1) {
        calculate_hash_local(new_index, timestamp, data, prev_hash, nonce, hash);
        hash_count++;
        if (hash_meets_difficulty_local(hash, difficulty)) {
            accept++;
            break;
        }
        nonce++;

        time_t now = time(NULL);
        if (now - last_report >= 1) {
            double elapsed = difftime(now, last_report);
            double mhps = (hash_count / 1e6) / elapsed;
            printf("\rHashrate: %.2f MH/s | Accepted: %u | Rejected: %u", mhps, accept, reject);
            fflush(stdout);
            hash_count = 0;
            last_report = now;
        }
    }

    time_t end_time = time(NULL);
    double total_elapsed = difftime(end_time, start_time);
    double avg_hashrate = (double)nonce / total_elapsed / 1e6;
    printf("\rHashrate: %.2f MH/s | Accepted: %u | Rejected: %u\n",
           avg_hashrate, accept, reject);

    if (!append_block_to_file_local(new_index, timestamp, nonce, data, prev_hash, hash)) {
        fprintf(stderr, "Error: gagal menulis blok baru ke '%s'\n", CHAIN_FILE);
        return;
    }
    printf("Block #%d mined successfully!\n", new_index);
    printf("Nonce: %d\nHash: %s\n", nonce, hash);
}


// ===============================
// 2) POOL MINING (mine_pool)
// ===============================

// Hitung hash SHA-256 untuk blok pool
static void calculate_hash_pool(int index, long timestamp, const char *data,
                                const char *prev_hash, int nonce, char out_hash[HASH_STR_LEN]) {
    char buffer[512];
    snprintf(buffer, sizeof buffer, "%d%ld%s%s%d", index, timestamp, data, prev_hash, nonce);
    sha256(buffer, out_hash);
    out_hash[HASH_STR_LEN - 1] = '\0';
}

// Cek kesulitan untuk pool
static int hash_meets_difficulty_pool(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; i++) {
        if (hash[i] != '0') return 0;
    }
    return 1;
}

// Fungsi pool mining: dipanggil dari main.c saat perintah "minepool <name> <host> <port>"
void mine_pool(const char *miner_name, const char *host, const char *port) {
    struct addrinfo hints, *res, *p;
    int sockfd, rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo (pool): %s\n", gai_strerror(rv));
        return;
    }
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket (pool)");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect (pool)");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "Gagal koneksi ke pool %s:%s\n", host, port);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    char hello_msg[256];
    snprintf(hello_msg, sizeof hello_msg, "HELLO %s\n", miner_name);
    send(sockfd, hello_msg, strlen(hello_msg), 0);

    char buf[512];
    int difficulty;

    unsigned int accept = 0, reject = 0;
    unsigned long long hash_count = 0;

    printf("Terhubung ke pool %s:%s sebagai miner '%s'\n", host, port, miner_name);

    while (1) {
        // Terima JOB dari pool: "JOB <index> <timestamp> <prev_hash> <difficulty>\n"
        ssize_t len = recv(sockfd, buf, sizeof buf - 1, 0);
        if (len <= 0) {
            printf("Pool terputus.\n");
            break;
        }
        buf[len] = '\0';

        int index;
        long timestamp;
        char prev_hash[HASH_STR_LEN];
        if (sscanf(buf, "JOB %d %ld %64s %d\n", &index, &timestamp, prev_hash, &difficulty) != 4) {
            // Bukan JOB yang valid → abaikan
            continue;
        }

        // Siapkan data untuk PoW
        int nonce = 0;
        char data_field[DATA_MAX_LEN];
        snprintf(data_field, sizeof data_field, "Mined_by_%s", miner_name);
        char hash[HASH_STR_LEN];

        hash_count = 0;
        time_t start_time = time(NULL);
        time_t last_report = start_time;

        printf("Menerima JOB: index=%d, timestamp=%ld, difficulty=%d\n", index, timestamp, difficulty);

        // Loop PoW
        while (1) {
            calculate_hash_pool(index, timestamp, data_field, prev_hash, nonce, hash);
            hash_count++;
            if (hash_meets_difficulty_pool(hash, difficulty)) {
                accept++;
                break;
            }
            nonce++;

            time_t now = time(NULL);
            if (now - last_report >= 1) {
                double elapsed = difftime(now, last_report);
                double mhps = (hash_count / 1e6) / elapsed;
                printf("\rHashrate: %.2f MH/s | Accepted: %u | Rejected: %u", mhps, accept, reject);
                fflush(stdout);
                hash_count = 0;
                last_report = now;
            }
        }

        time_t end_time = time(NULL);
        double total_elapsed = difftime(end_time, start_time);
        double avg_hashrate = (double)nonce / total_elapsed / 1e6;
        printf("\rHashrate: %.2f MH/s | Accepted: %u | Rejected: %u\n",
               avg_hashrate, accept, reject);

        // Kirim SOLN ke pool: "SOLN <nonce>\n"
        char soln_msg[64];
        snprintf(soln_msg, sizeof soln_msg, "SOLN %d\n", nonce);
        send(sockfd, soln_msg, strlen(soln_msg), 0);

        // Tunggu response: "ACCEPT <index> <hash>\n" atau "REJECT\n"
        len = recv(sockfd, buf, sizeof buf - 1, 0);
        if (len <= 0) {
            printf("Pool terputus sebelum menerima response.\n");
            break;
        }
        buf[len] = '\0';
        if (strncmp(buf, "ACCEPT", 6) == 0) {
            // Cetak hash yang diterima pool (bagian setelah “ACCEPT <index> ”)
            char *hashptr = strchr(buf, ' ');
            if (hashptr) hashptr = strchr(hashptr + 1, ' ');
            if (hashptr) hashptr++;
            printf(">>> Pool menerima blok #%d (hash=%s)\n", index, hashptr);
        } else {
            printf(">>> Pool menolak solusi nonce=%d untuk blok #%d\n", nonce, index);
            reject++;
        }
    }

    close(sockfd);
}
