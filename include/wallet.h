#ifndef WALLET_H
#define WALLET_H

/**
 * Buat wallet baru (generate private key + derive address lalu simpan).
 * @param filename  Nama file untuk menyimpan (misal "wallet.dat").
 */
void create_wallet(const char *filename);

/**
 * Tampilkan address wallet yang ada di file.
 * Jika file tidak ditemukan, tampilkan instruksi buat baru.
 * @param filename  Nama file wallet (misal "wallet.dat").
 */
void show_wallet(const char *filename);

#endif // WALLET_H
