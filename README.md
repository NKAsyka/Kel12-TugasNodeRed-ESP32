# Kel12-TugasNodeRED-ESP32

## Video
- Video presentasi (maks. 7 menit): **[https://drive.google.com/file/d/1XIcBbFof-WR9SOHgMw_B3jEM0WmLTeT6/view?usp=drivesdk]** 
- Video demo alat (maks. 5 menit): **[https://youtu.be/EFjaDAxAD6o?si=UwSpxen0CkvJ5SAb]**

## Deskripsi
Sistem monitoring IoT ini menggunakan empat ESP32 dan sensor DHT11 untuk mengirim suhu serta kelembapan setiap ruangan melalui MQTT. Node-RED menerima data, menampilkan grafik dan gauge pada dashboard, serta mengirim perintah untuk mengendalikan dua lampu LED di masing-masing ruangan.

## Anggota Kelompok
| Anggota | Nama | NRP | Tanggung jawab | Ruangan yang di pegang |
|---|---|---|---|
| 1 | Moh Akmalul Ihsan      | (2124600093) | integrasi ESP32 dan MQTT   | Ruang1 |
| 2 | Moh Jefri Al Bukhori   | (2124600098) | dokumentasi dan pengujian  | Ruang2 |
| 3 | Ahmad Syahrial Karim   | (2124600100) | dashboard Node-RED         | Ruang3 |
| 4 | Lailiah Ratu Nova      | (2124600104) | integrasi GitHub dan video | Ruang4 |


## Arsitektur
Alur data: empat ESP32 mem-publish temperature, humidity, dan status ke broker, lalu Node-RED melakukan subscribe dengan wildcard dan menampilkannya di dashboard (http://localhost:1883/ui). Arah sebaliknya, saklar dashboard mem-publish lamp1/lamp2 ke broker dan diterima ESP32 ruangan yang dituju.

```text
ESP32 Ruang 1 ─┐
ESP32 Ruang 2 ─┼──> MQTT broker ──> Node-RED ──> Dashboard /ui
ESP32 Ruang 3 ─┤       ▲                 │
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
- Akses dari HP di jaringan yang sama: `192.168.10.44:1883/ui`

## Menjalankan sistem
1. Install Node.js dan Node-RED, lalu jalankan Node-RED.
2. Dari **Menu → Manage palette → Install**, install `node-red-dashboard`.
3. Import `node-red/flows.json` melalui **Menu → Import → select a file**, lalu lakukan Deploy.
4. Edit konfigurasi broker pada flow bila broker berubah. Pastikan prefix `klp12` pada topic dashboard cocok dengan prefix pada kode ESP32.
6. Di Arduino IDE, install `EspMQTTClient`, `DHT sensor library`, dan `Adafruit Unified Sensor`.
7. Pada file `.ino`, ubah `ROOM`, `WIFI_SSID`, `WIFI_PASS`, dan `KELOMPOK` sesuai konfigurasi kelompok/ruangan. Gunakan Client ID unik tiap board.
8. Upload kode ke masing-masing ESP32. Buka Serial Monitor pada 115200 baud.
9. Buka dashboard di `http://localhost:1880/ui` dan uji sensor, saklar, serta status.

## Bukti dan pengujian
- `rangkaian.jpg`
- `flow.png`
- `dashboard.png`
- `uji_lampu.png`
- `uji_status.png`

| Uji | Hasil | Keterangan |
|---|---|---|
| T1 Koneksi MQTT | [✓] | Koneksi ke broker berhasil; pesan konfirmasi muncul di Serial Monitor. |
| T2 Data 4 ruang | [✓] | Data tiap ruang diterima Node-RED dan muncul di tab Debug |
| T3 Chart suhu | [✓] | Chart menampilkan garis ruang1 sampai ruang4 sesuai nama topic |
| T4 Sensor berubah | [✓] | Hanya data ruang2 yang berubah; ruangan lain tidak terpengaruh |
| T5 Lampu tepat sasaran | [✓] | Hanya lampu 2 ruang3 yang menyala lewat saklar dashboard |
| T6 Matikan lampu | [✓] | Lampu 2 ruang3 mati saat saklar dimatikan |
| T7 Payload salah | [✓] | Payload ON diabaikan; lampu tidak berubah dan ESP32 tetap berjalan |
| T8 Status online | [✓] | Indikator status keempat ruang menampilkan online |
| T9 LWT offline | [✓] | Status ruang4 berubah menjadi offline setelah daya dicabut (LWT) |
| T10 Retained status | [✓] | Status langsung tampil setelah Node-RED di-restart (retained message) |
| T11 Reconnect | [✓] | ESP32 tersambung kembali otomatis dan status kembali online |
| T12 Akses HP | [✓] | Dashboard dan saklar berfungsi dari HP di jaringan yang sama |

Ruangan simulasi (jika ada): tidak ada

## Kendala dan solusi
| Kendala | Solusi yang dilakukan |
|---|---|
| Dua ESP32 saling putus-sambung |	clientId unik berbasis MAC • topik per ruang • cek stabilitas Wi-Fi • reconnect non-blocking |
Chart hanya satu garis | Pisahkan topik temperature dan humidity, atau konfigurasi multi-series pada chart Node-RED |
Status offline lama muncul | Retained LWT yang benar • publish online retained • status per ruang • tangani retained subscriber |
