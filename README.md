# ⌚ ESP32 Smartwatch UI with TFT 240x240 IPS & Chronos

Proyek ini adalah implementasi antarmuka smartwatch menggunakan **ESP32** dan layar **TFT IPS 240x240 (ST7789)**. Smartwatch ini mendukung notifikasi, status baterai, informasi cuaca, dan navigasi berbasis BLE dari aplikasi **Chronos (Android)**.

## 🧠 Fitur Utama

- 🌐 **Koneksi BLE** ke aplikasi Chronos
- 🔋 **Ikon status baterai** dengan indikator charging
- 📲 **Tampilan notifikasi** terakhir
- ⏰ **Jam dan tanggal** dengan format 12 jam
- ☁️ **Informasi cuaca** saat tersedia
- 🧭 **Navigasi dengan estimasi waktu dan arah**
- 🎨 Antarmuka gaya cyber / digital-hacker, dengan warna neon dan bitmap tajam
- 📡 Tampilan ikon koneksi BLE, status cuaca, dan arah navigasi

## 🖥️ Spesifikasi Hardware

- **Board:** ESP32 Dev Module
- **Display:** TFT IPS 240x240 RGB (ST7789, SPI)
- **Resolusi:** 240×240 pixel

## 🔌 Wiring Diagram

| TFT Display Pin | ESP32 GPIO | Deskripsi   |
|------------------|------------|-------------|
| VCC              | 3.3V       | Power       |
| GND              | GND        | Ground      |
| SCL / CLK        | GPIO 18    | SPI Clock   |
| SDA / MOSI       | GPIO 23    | SPI Data    |
| DC               | GPIO 2     | Data/Command|
| RST              | GPIO 4     | Reset       |
| CS               | -1 (None)  | Tidak digunakan |

> **Catatan:** Proyek ini tidak menggunakan pin CS dan Touch.

## ⚙️ Setup dan Library

Pastikan Anda menginstal library berikut:

- [`TFT_eSPI`](https://github.com/Bodmer/TFT_eSPI)
- [`ChronosESP32`](https://github.com/AmanullahAnsari/ChronosESP32) (atau versi forked yang digunakan)
- `Adafruit GFX` (jika dibutuhkan oleh bitmap)

### `User_Setup.h` (untuk TFT_eSPI)

```cpp
#define ST7789_DRIVER
#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define TFT_RST 4
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS -1
#define TOUCH_CS -1

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SPI_FREQUENCY 27000000
