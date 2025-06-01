# mycoin-crypto-cab
For educational blockchain with Blueberry Lowbush enviroment (⁠⊃⁠｡⁠•́⁠‿⁠•̀⁠｡⁠)⁠⊃ 🌱🫐

🍇 Archana Berry Coin (CAB) - Cryptocurrency Project

Welcome to the official repository of CAB Coin (Archana Berry Coin), a lightweight and customizable cryptocurrency based on Verus Coin principles, re-engineered with a flexible DB and modular architecture.
Selamat datang di repositori resmi CAB Coin (Archana Berry Coin), sebuah proyek kripto ringan dan dapat dikembangkan ulang berdasarkan struktur Verus Coin, namun dengan sistem database dan arsitektur modular yang lebih fleksibel.


---

📁 Project Structure / Struktur Proyek

crypt_cab/
├── include/               # Header files
├── source/                # Main source code (bukan src/)
│   ├── main.c             # Entrypoint utama
│   ├── wallet.c           # Wallet management logic
│   └── blockchain.c       # Block, chain, consensus logic
├── build/                 # Output binary hasil kompilasi
├── Makefile               # Untuk kompilasi
├── README.md              # Dokumentasi ini
├── LICENSE.ABPL           # Lisensi (lihat bagian Lisensi)
└── image.png              # Dummy gambar repositori


---

🚀 Build Instructions / Cara Kompilasi

$ git clone https://github.com/archanaberry/cabcoin.git
$ cd cabcoin
$ make
$ cd build
$ ./cabcoin help


---

🧠 Overview / Ringkasan Fitur

✅ Lightweight blockchain core

✅ CLI wallet & blockchain interaction

⚠️ Custom DB (sementara dummy, belum lengkap)

❌ Belum ada file identitas dompet

🔧 Lisensi menggunakan ABPL (Archana Berry Public License)



---

🛠️ How It Works / Cara Kerja Sederhana

1. Saat dijalankan, program akan:

Membaca blockchain dari file (chain.cabdb)

Memuat wallet dari file (wallet.cabwallet)

Menampilkan status wallet dan jumlah block


2. Format dompet dan chain masih minimal:

Satu file menyimpan semua data

Tanpa identitas file (_identwallet.cabident belum dibuat)


3. Transaksi, validasi, dan proof-of-work masih dummy



📦 Sample CLI Usage / Contoh Pemakaian

./cabcoin initwallet                  # Buat dompet baru
./cabcoin showwallet                 # Tampilkan informasi dompet
./cabcoin addblock "txdata"         # Tambah block dummy


---

📄 License / Lisensi

This project is licensed under [Archana Berry Public License - ABPL](https://github.com/archanaberry/Lisensi).
