# 🪙 CAB Coin – Archana Berry Cryptocurrency  
**(mycoin-crypto-cab)**  
> Educational blockchain for Blueberry Lowbush environment (⁠⊃⁠｡⁠•́⁠‿⁠•̀⁠｡⁠)⁠⊃ 🌱🫐

---

## 🍇 Tentang Proyek

**CAB Coin (Archana Berry Coin)** adalah proyek cryptocurrency ringan dan modular, terinspirasi oleh arsitektur Verus Coin namun didesain ulang dengan sistem database fleksibel dan struktur modular yang dapat dikembangkan.

> Selamat datang di repositori resmi CAB Coin — fondasi blockchain edukatif untuk eksperimen lightweight dan embedded-friendly.

---

## 📁 Struktur Proyek

```
crypt_cab/ 
├── include/               # Header file (.h) 
├── source/                # Sumber utama (bukan src/) │   
├── main.c             # Entrypoint 
│   ├── wallet.c           # Logika dompet 
│   └── blockchain.c       # Logika block dan chain 
├── build/                 # Output binary hasil kompilasi 
├── Makefile               # Untuk kompilasi 
├── README.md              # Dokumentasi ini 
├── LICENSE.ABPL           # Lisensi ABPL 
└── image.png              # Dummy visual repositori
```

---

## 🚀 Cara Kompilasi

```bash
$ git clone https://github.com/archanaberry/cabcoin.git
$ cd cabcoin
$ make
$ cd build
$ ./cabcoin help
```


---

🧠 Ringkasan Fitur

✅ Inti blockchain ringan

✅ Dompet CLI & interaksi blockchain

⚠️ DB kustom (masih dummy/test)

🔐 Belum ada file identitas dompet (_identwallet.cabident)

🔧 Lisensi eksklusif: ABPL (Archana Berry Public License)



---

🛠️ Cara Kerja Singkat

1. Saat dijalankan:

Membaca blockchain dari chain.cabdb

Memuat dompet dari wallet.cabwallet

Menampilkan status dompet dan jumlah blok



2. Format file masih sederhana:

Satu file menyimpan semua data dompet & blok

Belum ada sistem identitas unik dompet (akan hadir via _identwallet.cabident)


3. Fitur transaksi, validasi, dan PoW masih berupa placeholder (dummy)




---

📦 Contoh Penggunaan CLI

```
./cabcoin initwallet
# Membuat dompet baru dan menyimpan ke wallet.cabwallet

./cabcoin showwallet
# Menampilkan informasi dompet: address, balance, history, block count

./cabcoin addblock "txdata"
# Menambahkan block dummy dengan data transaksi yang disimulasikan

./cabcoin dumpwallet
# (opsional) Menampilkan isi mentah file dompet untuk debugging

./cabcoin dumpproof
# (opsional) Menampilkan proof dummy dari block terakhir

./cabcoin reindex
# Re-scan dan validasi ulang semua block (fitur dummy saat ini)

./cabcoin genident
# (eksperimental) Membuat file identitas dompet: _identwallet.cabident

./cabcoin sync
# (fitur akan datang) Sinkronisasi blockchain dari sumber jaringan

./cabcoin sendto <address> <amount>
# (fitur akan datang) Mengirim saldo ke address tujuan

./cabcoin exportwallet backup.cab
# (fitur akan datang) Mengekspor dompet ke file backup

./cabcoin importwallet backup.cab
# (fitur akan datang) Mengimpor dompet dari backup

./cabcoin help
# Menampilkan semua perintah CLI yang tersedia
```
---

📄 Lisensi

Proyek ini dilisensikan di bawah [Archana Berry Public License - ABPL](https://github.com/archanaberry/Lisensi).
Lisensi ini mendorong pembelajaran, eksplorasi, dan pengembangan sistem eksperimental berbasis Blueberry Environment.
