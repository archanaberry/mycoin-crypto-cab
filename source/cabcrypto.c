// File: src/cabcrypto.c
//
// Simple wrapper around OpenSSL's SHA-256 for Coin Archana Berry (CAB).
// Implements the sha256() function declared in cabcrypto.h.
//
// Dependencies:
//   - <openssl/sha.h>
//   - <string.h>
//   - "cabcrypto.h" (which should declare: void sha256(char *input, char output[65]));

#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include "cabcrypto.h"

/**
 * Computes the SHA-256 hash of the given input string.
 * The result is hex-encoded into a 64-character string plus a null terminator.
 *
 * @param input   The null-terminated input string to hash.
 * @param output  A buffer of at least 65 bytes where the hex string will be stored.
 *                After execution, output will contain exactly 64 hex characters,
 *                followed by '\0'.
 */
void sha256(char *input, char output[65]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    // Compute raw SHA-256 digest
    SHA256((unsigned char*)input, strlen(input), hash);
    // Convert each byte to two hex characters
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    // Null-terminate the string
    output[64] = '\0';
}
