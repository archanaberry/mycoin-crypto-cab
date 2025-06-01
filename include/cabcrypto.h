#ifndef CABCRYPTO_H
#define CABCRYPTO_H

/**
 * Menghasilkan hash SHA-256 dari input string.
 *
 * @param input   String input (null-terminated)
 * @param output  Buffer untuk hasil hash dalam bentuk hex (65 bytes: 64 + '\0')
 */
void sha256(char *input, char output[65]);

#endif // CABCRYPTO_H
