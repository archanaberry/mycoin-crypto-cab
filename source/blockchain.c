// File: src/blockchain.c
//
// Blockchain module for Coin Archana Berry (CAB).
// Provides basic blockchain functionality: create/load/save chain, add blocks (mining), validate chain.
//
// Dependencies:
//   - cabcrypto.h (contains sha256() prototype)
//   - Standard C libraries (stdio, stdlib, string, time)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "cabcrypto.h"

#define MAX_BLOCKS    1024      // Maksimum jumlah block dalam rantai (prototipe sederhana)
#define DATA_MAX_LEN  256       // Maksimum panjang data per block
#define HASH_STR_LEN  65        // Panjang string hex SHA-256 + null terminator
#define CHAIN_FILE    "data/chain.dat"
#define DIFFICULTY    2         // Jumlah leading '0' yang diperlukan pada hash untuk Proof-of-Work

typedef struct {
    int index;
    long timestamp;
    char data[DATA_MAX_LEN];
    char prev_hash[HASH_STR_LEN];
    int nonce;
    char hash[HASH_STR_LEN];
} Block;

// Array in-memory untuk menyimpan rantai blok (simple fixed-size)
static Block chain[MAX_BLOCKS];
static int chain_length = 0;

/**
 * Membentuk string concatenation dari field block untuk di-hash:
 * "<index><timestamp><data><prev_hash><nonce>"
 */
static void block_to_string(const Block *blk, char *out) {
    // Pastikan buffer out cukup besar (misal, > 512 bytes)
    sprintf(out, "%d%ld%s%s%d",
            blk->index,
            blk->timestamp,
            blk->data,
            blk->prev_hash,
            blk->nonce);
}

/**
 * Hitung hash SHA-256 untuk block (berdasarkan semua field kecuali hash itu sendiri).
 * Hasil hex-encoded 64-karakter disimpan di hash_out (harus punya ukuran >= HASH_STR_LEN).
 */
static void calculate_hash(const Block *blk, char *hash_out) {
    char buffer[512];
    block_to_string(blk, buffer);
    sha256(buffer, hash_out);
    hash_out[HASH_STR_LEN - 1] = '\0';
}

/**
 * Periksa apakah hash memenuhi target difficulty (memulai dengan DIFFICULTY kali '0').
 */
static int hash_meets_difficulty(const char *hash) {
    for (int i = 0; i < DIFFICULTY; i++) {
        if (hash[i] != '0') return 0;
    }
    return 1;
}

/**
 * Buat blok pertama (genesis block) jika file chain belum ada.
 */
static Block create_genesis_block() {
    Block genesis;
    genesis.index = 0;
    genesis.timestamp = time(NULL);
    strncpy(genesis.data, "Genesis Block", DATA_MAX_LEN - 1);
    genesis.data[DATA_MAX_LEN - 1] = '\0';
    strncpy(genesis.prev_hash, "0000000000000000000000000000000000000000000000000000000000000000", HASH_STR_LEN - 1);
    genesis.prev_hash[HASH_STR_LEN - 1] = '\0';
    genesis.nonce = 0;

    // Cari nonce hingga hash memenuhi target
    do {
        genesis.nonce++;
        calculate_hash(&genesis, genesis.hash);
    } while (!hash_meets_difficulty(genesis.hash));

    return genesis;
}

/**
 * Load blockchain dari file CHAIN_FILE (text mode).
 * Format setiap baris:
 *   index|timestamp|nonce|data|prev_hash|hash\n
 * Mengembalikan 1 jika sukses, 0 jika file tidak ada (akan dibuat baru) atau error baca.
 */
static int load_chain_from_file() {
    FILE *fp = fopen(CHAIN_FILE, "r");
    if (!fp) {
        // File belum ada → chain akan diinisialisasi baru
        return 0;
    }

    char line[1024];
    chain_length = 0;

    while (fgets(line, sizeof(line), fp) != NULL && chain_length < MAX_BLOCKS) {
        Block blk;
        char *token;

        // Parse index
        token = strtok(line, "|");
        if (!token) break;
        blk.index = atoi(token);

        // Parse timestamp
        token = strtok(NULL, "|");
        if (!token) break;
        blk.timestamp = atol(token);

        // Parse nonce
        token = strtok(NULL, "|");
        if (!token) break;
        blk.nonce = atoi(token);

        // Parse data
        token = strtok(NULL, "|");
        if (!token) break;
        strncpy(blk.data, token, DATA_MAX_LEN - 1);
        blk.data[DATA_MAX_LEN - 1] = '\0';

        // Parse prev_hash
        token = strtok(NULL, "|");
        if (!token) break;
        strncpy(blk.prev_hash, token, HASH_STR_LEN - 1);
        blk.prev_hash[HASH_STR_LEN - 1] = '\0';

        // Parse hash (bersih newline jika ada)
        token = strtok(NULL, "|\n");
        if (!token) break;
        strncpy(blk.hash, token, HASH_STR_LEN - 1);
        blk.hash[HASH_STR_LEN - 1] = '\0';

        chain[chain_length++] = blk;
    }

    fclose(fp);
    return (chain_length > 0);
}

/**
 * Simpan blockchain saat ini ke file CHAIN_FILE (ditimpa).
 * Format setiap baris sama dengan load: index|timestamp|nonce|data|prev_hash|hash\n
 * Mengembalikan 1 jika sukses, 0 jika gagal.
 */
static int save_chain_to_file() {
    // Pastikan direktori "data/" sudah ada. Jika belum, coba buat.
    #ifdef _WIN32
        _mkdir("data");
    #else
        mkdir("data", 0755);
    #endif

    FILE *fp = fopen(CHAIN_FILE, "w");
    if (!fp) return 0;

    for (int i = 0; i < chain_length; i++) {
        Block *blk = &chain[i];
        // Gunakan '|' sebagai pemisah, yakin data tidak mengandung '|'
        fprintf(fp, "%d|%ld|%d|%s|%s|%s\n",
                blk->index,
                blk->timestamp,
                blk->nonce,
                blk->data,
                blk->prev_hash,
                blk->hash);
    }

    fclose(fp);
    return 1;
}

/**
 * Inisialisasi blockchain: jika file ada, load; jika tidak, buat genesis block.
 */
void init_blockchain() {
    if (!load_chain_from_file()) {
        // Tidak ada file -> buat genesis
        Block genesis = create_genesis_block();
        chain[0] = genesis;
        chain_length = 1;
        if (!save_chain_to_file()) {
            fprintf(stderr, "Error: Gagal menyimpan genesis block ke '%s'\n", CHAIN_FILE);
            exit(EXIT_FAILURE);
        }
        printf("Genesis block dibuat (index: 0, hash: %s)\n", genesis.hash);
    } else {
        printf("Blockchain dimuat, jumlah block: %d\n", chain_length);
    }
}

/**
 * Tambahkan blok baru dengan data (string). Melakukan Proof-of-Work hingga hash memenuhi difficulty.
 * Setelah selesai, simpan ke file.
 */
void add_new_block(const char *data) {
    if (chain_length >= MAX_BLOCKS) {
        fprintf(stderr, "Error: Batas maksimum block tercapai (%d)\n", MAX_BLOCKS);
        return;
    }

    Block new_blk;
    Block *last = &chain[chain_length - 1];

    new_blk.index = last->index + 1;
    new_blk.timestamp = time(NULL);

    // Salin data (pastikan tidak overflow)
    strncpy(new_blk.data, data, DATA_MAX_LEN - 1);
    new_blk.data[DATA_MAX_LEN - 1] = '\0';

    strncpy(new_blk.prev_hash, last->hash, HASH_STR_LEN - 1);
    new_blk.prev_hash[HASH_STR_LEN - 1] = '\0';

    new_blk.nonce = 0;

    // Proof-of-Work: increment nonce hingga mendapat hash dengan DIFFICULTY leading '0'
    do {
        new_blk.nonce++;
        calculate_hash(&new_blk, new_blk.hash);
    } while (!hash_meets_difficulty(new_blk.hash));

    // Tambahkan ke rantai dan simpan
    chain[chain_length++] = new_blk;
    if (!save_chain_to_file()) {
        fprintf(stderr, "Error: Gagal menyimpan block baru ke '%s'\n", CHAIN_FILE);
    } else {
        printf("Block #%d mined (nonce: %d, hash: %s)\n",
               new_blk.index, new_blk.nonce, new_blk.hash);
    }
}

/**
 * Cetak isi seluruh rantai blok ke stdout, format:
 *   Index: <i>
 *   Timestamp: <t>
 *   Data: <data>
 *   Prev Hash: <prev_hash>
 *   Nonce: <nonce>
 *   Hash: <hash>
 *   -------------------------
 */
void print_chain() {
    for (int i = 0; i < chain_length; i++) {
        Block *blk = &chain[i];
        printf("=== Block %d ===\n", blk->index);
        printf("Timestamp : %ld\n", blk->timestamp);
        printf("Data      : %s\n", blk->data);
        printf("Prev Hash : %s\n", blk->prev_hash);
        printf("Nonce     : %d\n", blk->nonce);
        printf("Hash      : %s\n", blk->hash);
        printf("-------------------------------\n");
    }
}

/**
 * Validasi integritas blockchain:
 * 1) Setiap block (kecuali genesis) prev_hash harus sama dengan hash block sebelumnya.
 * 2) Setiap hash harus sesuai perhitungan ulang dengan nonce, dan memenuhi difficulty.
 * Mengembalikan 1 jika valid, 0 jika ada yang rusak.
 */
int is_chain_valid() {
    for (int i = 1; i < chain_length; i++) {
        Block *cur = &chain[i];
        Block *prev = &chain[i - 1];

        // Cek prev_hash
        if (strcmp(cur->prev_hash, prev->hash) != 0) {
            printf("Invalid: Block %d prev_hash tidak cocok.\n", cur->index);
            return 0;
        }

        // Cek ulang hash
        char recalculated[HASH_STR_LEN];
        calculate_hash(cur, recalculated);
        if (strcmp(cur->hash, recalculated) != 0) {
            printf("Invalid: Block %d hash tidak valid (nonce/data pernah diubah?).\n", cur->index);
            return 0;
        }

        // Cek difficulty
        if (!hash_meets_difficulty(cur->hash)) {
            printf("Invalid: Block %d hash tidak memenuhi difficulty.\n", cur->index);
            return 0;
        }
    }
    return 1;
}
