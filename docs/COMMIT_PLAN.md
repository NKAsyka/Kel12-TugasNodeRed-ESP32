# Rencana Commit Kelompok (4 Anggota)

Aturan modul: setiap anggota minimal 2 commit sendiri. Gunakan akun GitHub masing-masing, commit hanya pekerjaan yang benar-benar dibuat/diperiksa, dan lakukan commit terakhir sebelum tenggat.

| Anggota | Fokus kontribusi | Commit 1 | Commit 2 |
|---|---|---|---|
| Anggota 1 — Ruang 1 | Kode ESP32 dan uji MQTT | `feat(esp32): implement DHT11 publish for ruang1` | `test(esp32): verify lamp control and LWT for ruang1` |
| Anggota 2 — Ruang 2 | Flow Node-RED dan chart | `feat(nodered): add MQTT temperature chart for four rooms` | `feat(nodered): add humidity gauges and debug nodes` |
| Anggota 3 — Ruang 3 | Dashboard kontrol/status dan pengujian | `feat(dashboard): add eight lamp switches and room status` | `test(dashboard): document T1-T12 verification results` |
| Anggota 4 — Ruang 4 | README, laporan, bukti dan integrasi | `docs(readme): document setup, topics, and architecture` | `docs(report): add screenshots, test evidence, and video links` |

## Cara kerja yang disarankan
1. Setiap anggota clone repository yang sama.
2. Anggota membuat perubahan di file sesuai tanggung jawab, lalu commit dan push dengan akun sendiri.
3. Anggota integrator memeriksa hasil, menggabungkan perubahan, dan memastikan flow bisa di-import.
4. Jangan sekadar membuat commit kosong untuk memenuhi jumlah. Commit harus mencerminkan perubahan nyata.
5. Jangan memasukkan password Wi-Fi asli ke repository publik.
