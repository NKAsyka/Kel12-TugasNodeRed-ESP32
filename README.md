# KelX-TugasNodeRED-ESP32
tes tes 123

## Video
- Video presentasi (maks. 7 menit): **[isi link di sini]**
- Video demo alat (maks. 5 menit): **[isi link di sini]**

## Deskripsi
Sistem monitoring IoT ini menggunakan empat ESP32 dan sensor DHT11 untuk mengirim suhu serta kelembapan setiap ruangan melalui MQTT. Node-RED menerima data, menampilkan grafik dan gauge pada dashboard, serta mengirim perintah untuk mengendalikan dua lampu LED di masing-masing ruangan.

## Anggota Kelompok
| Anggota | Nama | NRP | Tanggung jawab |
|---|---|---|---|
| 1 | [M. Akmalul Ihsan]     | [2124600093] | Ruang 1; integrasi ESP32 dan MQTT   |
| 2 | [M. Jefri Al Bukhori]b | [2124600098] | Ruang 2; dashboard Node-RED         |
| 3 | [Ahmad Syahrial Karim] | [2124600100] | Ruang 3; dokumentasi dan pengujian  |
| 4 | [Lailiah Ratu Nova]    | [2124600104] | Ruang 4; integrasi GitHub dan video |

> Ganti semua placeholder `[ ... ]`, `KelX`, dan `klpX` dengan data kelompok yang benar.

## Arsitektur
```text
ESP32 Ruang 1 ─┐
ESP32 Ruang 2 ─┼──> MQTT broker ──> Node-RED ──> Dashboard /ui
ESP32 Ruang 3 ─┤       ▲                │
ESP32 Ruang 4 ─┘       └── perintah lampu┘
```

## Perangkat keras per ruangan
| Komponen | Pin ESP32 | Catatan |
|---|---:|---|
| DHT11 VCC | 3.3 V | Catu daya sensor |
| DHT11 GND | GND | Ground bersama |
| DHT11 DATA | GPIO16 | Sensor suhu dan kelembapan |
| LED lampu 1 + resistor 220 Ω | GPIO25 | Anoda ke pin melalui resistor, katoda ke GND |
| LED lampu 2 + resistor 220 Ω | GPIO26 | Anoda ke pin melalui resistor, katoda ke GND |

## Broker dan topik MQTT
- Broker: `broker.emqx.io`
- Port: `1883`
- Prefix wajib: `pens/klp12/` 

|
Fungsi | Topic | Arah | Payload |
|---|---|---|---|
| Suhu | `pens/klp12/ruang1/temperature` | ESP32 → Node-RED | angka °C, 1 desimal |
| Kelembapan | `pens/klp12/ruang1/humidity` | ESP32 → Node-RED | angka %, 1 desimal |
| Lampu 1 | `pens/klp12/ruang1/lamp1` | Node-RED → ESP32 | string `1` / `0` |
| Lampu 2 | `pens/klp12/ruang1/lamp2` | Node-RED → ESP32 | string `1` / `0` |
| Status | `pens/klpX/ruang1/status` | ESP32/LWT → Node-RED | `online` / `offline`, retained |


## Isi dashboard
- Tab: **Gedung**
- Group **Suhu**: satu line chart dengan empat garis bernama `ruang1`–`ruang4`.
- Group tiap ruangan: satu gauge kelembapan 0–100%, dua saklar lampu, dan indikator status koneksi.
- Alamat lokal: `http://localhost:1883/ui`
- Akses dari HP di jaringan yang sama: `http://IP_LAPTOP:1880/ui`

## Menjalankan sisteml
1. Install Node.js dan Node-RED, lalu jalankan Node-RED.
2. Dari **Menu → Manage palette → Install**, install `node-red-dashboard`.
3. Import `node-red/flows.json` melalui **Menu → Import → select a file**, lalu lakukan Deploy.
4. Edit konfigurasi broker pada flow bila broker berubah. Pastikan prefix `klp12` pada topic dashboard cocok dengan prefix pada kode ESP32.
6. Di Arduino IDE, install `EspMQTTClient`, `DHT sensor library`, dan `Adafruit Unified Sensor`.
7. Pada file `.ino`, ubah `ROOM`, `WIFI_SSID`, `WIFI_PASS`, dan `KELOMPOK` sesuai konfigurasi kelompok/ruangan. Gunakan Client ID unik tiap board.
8. Upload kode ke masing-masing ESP32. Buka Serial Monitor pada 115200 baud.
9. Buka dashboard di `http://localhost:1880/ui` dan uji sensor, saklar, serta status.

## Bukti dan pengujian
Simpan gambar di `docs/images/` dengan nama yang jelas, misalnya:
- `rangkaian.jpg`
- `flow.png`
- `dashboard.png`
- `uji_lampu.png`
- `uji_status.png`

| Uji | Hasil | Keterangan |
|---|---|---|
| T1 Koneksi MQTT | [✓/✗] | [Catatan hasil] |
| T2 Data 4 ruang | [✓/✗] | [Catatan hasil] |
| T3 Chart suhu | [✓/✗] | [Catatan hasil] |
| T4 Sensor berubah | [✓/✗] | [Catatan hasil] |
| T5 Lampu tepat sasaran | [✓/✗] | [Catatan hasil] |
| T6 Matikan lampu | [✓/✗] | [Catatan hasil] |
| T7 Payload salah | [✓/✗] | [Catatan hasil] |
| T8 Status online | [✓/✗] | [Catatan hasil] |
| T9 LWT offline | [✓/✗] | [Catatan hasil] |
| T10 Retained status | [✓/✗] | [Catatan hasil] |
| T11 Reconnect | [✓/✗] | [Catatan hasil] |
| T12 Akses HP | [✓/✗] | [Catatan hasil] |

Ruangan simulasi (jika ada): **[tulis ruang dan metode simulasi; minimal 2 ESP32 sungguhan sesuai modul]**

## Kendala dan solusi
| Kendala | Solusi yang dilakukan |
|---|---|
| Dashboard kosong | Cocokkan broker, prefix topic, lalu Deploy ulang |
| Chart hanya satu garis | Pastikan function mengubah `msg.topic` menjadi nama ruang |
| Data DHT11 `nan` | Periksa pin DATA, VCC, GND, dan interval pembacaan |
| Lampu tidak merespons | Periksa topic, payload string `1`/`0`, pin, dan wiring |
| Status offline terlambat | LWT dipicu setelah broker mendeteksi koneksi putus; waktu dapat sekitar 1,5 × keep-alive |

## Pembagian tugas
Sesuaikan tabel anggota dan tanggung jawab di atas dengan pembagian aktual. Setiap anggota minimal membuat dua commit bermakna atas kontribusinya sendiri. Commit harus dilakukan oleh akun GitHub masing-masing; jangan membuat commit palsu atas nama anggota lain.

## Struktur repository
```text
KelX-TugasNodeRED-ESP32/
├── README.md
├── esp32/esp32_ruang/esp32_ruang.ino
├── node-red/flows.json
├── docs/laporan.pdf
├── docs/images/
└── video/LINK.md
```
