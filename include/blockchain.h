#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

/**
 * Inisialisasi blockchain. 
 * - Jika belum ada, buat Genesis Block
 * - Jika sudah ada, load chain dari disk
 */
void init_blockchain();

/**
 * Tambahkan data baru sebagai blok (solo mining).
 * Fungsi ini melakukan Proof‐of‐Work dan menyimpan hasil ke disk.
 */
void add_new_block(const char *data);

/**
 * Cetak seluruh isi blockchain ke stdout.
 */
void print_chain();

/**
 * Validasi seluruh chain: 
 * - Cek prev_hash tiap blok
 * - Cek hash benar dengan nonce/data
 * - Cek difficulty terpenuhi
 * 
 * @return 1 jika valid, 0 jika tidak valid
 */
int is_chain_valid();

#endif // BLOCKCHAIN_H
