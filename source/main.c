// File: src/main.c
//
// Entry point untuk cabcoin. Hanya memanggil fungsi‐fungsi dari modul lain,
// tanpa menyalin atau mendefinisikan ulang implementasi apapun selain main().

#include <stdio.h>
#include <string.h>

// Include header publik, tidak pernah include file .c langsung.
#include "blockchain.h"
#include "wallet.h"
#include "miner.h"

int main(int argc, char *argv[]) {
    // Inisialisasi blockchain (load atau buat Genesis Block)
    init_blockchain();

    if (argc < 2) {
        printf("Usage: cabcoin <command> [args]\n");
        printf("Commands:\n");
        printf("  createwallet                   Generate a new wallet\n");
        printf("  showwallet                     Display existing wallet address\n");
        printf("  mine <data>                    Solo mining (tanpa pool)\n");
        printf("  minepool <name> <host> <port>  Pool mining\n");
        printf("  showchain                      Display the entire blockchain\n");
        printf("  validate                       Validate blockchain integrity\n");
        return 0;
    }

    if (strcmp(argv[1], "createwallet") == 0) {
        create_wallet("wallet.dat");
    }
    else if (strcmp(argv[1], "showwallet") == 0) {
        show_wallet("wallet.dat");
    }
    else if (strcmp(argv[1], "mine") == 0) {
        if (argc != 3) {
            printf("Usage: cabcoin mine \"<data>\"\n");
        } else {
            mine_local(argv[2]);
        }
    }
    else if (strcmp(argv[1], "minepool") == 0) {
        if (argc != 5) {
            printf("Usage: cabcoin minepool <miner_name> <host> <port>\n");
        } else {
            mine_pool(argv[2], argv[3], argv[4]);
        }
    }
    else if (strcmp(argv[1], "showchain") == 0) {
        print_chain();
    }
    else if (strcmp(argv[1], "validate") == 0) {
        if (is_chain_valid()) {
            printf("Blockchain valid.\n");
        } else {
            printf("Blockchain TIDAK valid!\n");
        }
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
    }

    return 0;
}
