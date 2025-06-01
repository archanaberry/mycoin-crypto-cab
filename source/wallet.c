// File: src/wallet.c
//
// Wallet module for Coin Archana Berry (CAB).
// Provides basic wallet functionality: generate a new wallet (private key + address),
// save/load wallet data to/from file, and display the wallet address.
//
// Dependencies: 
//   - cabcrypto.h (contains sha256() prototype)
//   - Standard C libraries (stdio, stdlib, string, time)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cabcrypto.h"

#define PRIV_KEY_LEN 64    // 32 bytes, hex-encoded
#define ADDRESS_LEN 64     // SHA-256 hash output, hex-encoded
#define WALLET_FILE  "wallet.dat"

// Generate a random 32-byte private key, hex-encoded (64 chars + null terminator).
void generate_private_key(char *priv_key) {
    const char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < PRIV_KEY_LEN; i++) {
        priv_key[i] = hex_chars[rand() % 16];
    }
    priv_key[PRIV_KEY_LEN] = '\0';
}

// Derive a wallet address by computing SHA-256(priv_key).
void derive_address(const char *priv_key, char *address) {
    // sha256() is implemented in include/cabcrypto.h / cabcrypto.c
    sha256((char *)priv_key, address);
    address[ADDRESS_LEN] = '\0';
}

// Save private key and address to a wallet file.
// Format on disk:
//   <priv_key>\n
//   <address>\n
int save_wallet(const char *filename, const char *priv_key, const char *address) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    fprintf(fp, "%s\n%s\n", priv_key, address);
    fclose(fp);
    return 1;
}

// Load private key and address from wallet file.
// Expects exactly two lines: private key, then address.
int load_wallet(const char *filename, char *priv_key, char *address) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    if (fgets(priv_key, PRIV_KEY_LEN + 2, fp) == NULL) {
        fclose(fp);
        return 0;
    }
    priv_key[strcspn(priv_key, "\n")] = '\0';

    if (fgets(address, ADDRESS_LEN + 2, fp) == NULL) {
        fclose(fp);
        return 0;
    }
    address[strcspn(address, "\n")] = '\0';

    fclose(fp);
    return 1;
}

// Create a new wallet: generate a random private key, derive address, save to disk.
// Prints the new address on success.
void create_wallet(const char *filename) {
    char priv_key[PRIV_KEY_LEN + 1];
    char address[ADDRESS_LEN + 1];

    // Seed PRNG
    srand((unsigned)time(NULL));

    // Generate and derive
    generate_private_key(priv_key);
    derive_address(priv_key, address);

    // Save to file
    if (save_wallet(filename, priv_key, address)) {
        printf("New wallet created successfully.\n");
        printf("Wallet Address: %s\n", address);
    } else {
        fprintf(stderr, "Error: Failed to save wallet to '%s'.\n", filename);
    }
}

// Display the wallet address by loading from disk.
// If no wallet file is found, instruct user to create one.
void show_wallet(const char *filename) {
    char priv_key[PRIV_KEY_LEN + 1];
    char address[ADDRESS_LEN + 1];

    if (load_wallet(filename, priv_key, address)) {
        printf("Wallet Address: %s\n", address);
    } else {
        printf("No wallet found at '%s'.\n", filename);
        printf("Use the 'createwallet' command to generate a new wallet.\n");
    }
}
