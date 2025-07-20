#include <ChronosESP32.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Booting.h"

ChronosESP32 watch;
TFT_eSPI tft = TFT_eSPI();

// =============================================
// DEFINISI IKON BITMAP (dari referensi Lopaka App)
// =============================================
static const unsigned char PROGMEM image_battery_empty_bits[] = {
    0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFE, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x70, 0x00, 0x01,
    0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80, 0x00, 0x01,
    0x70, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x0F, 0xFF, 0xFE, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00};

static const unsigned char PROGMEM image_download_1_bits[] = {
    0x11, 0x88, 0x63, 0xC6, 0x44, 0x22, 0x8C, 0x11, 0x88, 0x11, 0x10, 0x08, 0x10, 0x08, 0x10,
    0x08, 0x10, 0x08, 0x20, 0x04, 0x20, 0x04, 0x40, 0x02, 0xFF, 0xFF, 0x06, 0x60, 0x03, 0xC0,
    0x00, 0x00};

static const unsigned char PROGMEM image_download_bits[] = {
    0x01, 0x00, 0x02, 0x80, 0x02, 0x40, 0x22, 0x20, 0x12, 0x20, 0x0A, 0x40, 0x06, 0x80, 0x03,
    0x00, 0x06, 0x80, 0x0A, 0x40, 0x12, 0x20, 0x22, 0x20, 0x02, 0x40, 0x02, 0x80, 0x01, 0x00,
    0x00, 0x00};

// =============================================
// DEFINISI WARNA
// =============================================
#define COLOR_TEXT 0x8FE0       // Hijau muda
#define COLOR_ACCENT 0xFC00     // Oranye
#define COLOR_BOX 0x9936        // Coklat
#define COLOR_NOTIF_TEXT 0xFFFF // Putih
#define COLOR_ICON_DL 0x3A96    // Biru kehijauan
#define COLOR_ICON_BAT 0x9936   // Coklat
#define COLOR_ICON_NOTIF 0xE8EC // Ungu muda
#define COLOR_BG TFT_BLACK      // Latar hitam
#define COLOR_WARN 0xF800       // Merah untuk peringatan
#define COLOR_GRAY 0x7BEF       // Abu-abu

// =============================================
// VARIABEL STATE
// =============================================
bool isConnected = false;
String currentTime = "";
String currentDate = "";
int batteryLevel = 0;
bool isCharging = false;
int notificationCount = 0;
String lastNotification = "";
Weather currentWeather;
bool hasWeather = false;
Navigation navData;
bool navActive = false;
bool navChanged = false;
bool isNavScreen = false;
uint32_t navIconCRC = 0xFFFFFFFF;

// =============================================
// FUNGSI TAMPILAN UI
// =============================================

// =============================================
// HELPER FUNCTIONS
// =============================================

// Fungsi untuk reset semua pengaturan TFT ke default
void resetTFTSettings()
{
  tft.setFreeFont(NULL);                  // Reset ke font default sistem
  tft.setTextSize(1);                     // Size default
  tft.setTextDatum(TL_DATUM);             // Top-Left datum
  tft.setTextColor(COLOR_TEXT, COLOR_BG); // Warna default
}

// =============================================
// DRAWING FUNCTIONS
// =============================================

void drawBatteryIcon(int x, int y, int level)
{
  // Pastikan menggunakan pengaturan default
  resetTFTSettings();

  // Gambar ikon baterai dasar
  tft.drawBitmap(x, y, image_battery_empty_bits, 24, 16, COLOR_ICON_BAT);

  // Gambar level baterai
  int fillWidth = (level * 20) / 100;
  if (fillWidth > 0)
  {
    tft.fillRect(x + 2, y + 2, fillWidth, 12, level > 20 ? COLOR_TEXT : COLOR_WARN);
  }

  // Tanda charging
  if (isCharging)
  {
    tft.setTextColor(COLOR_TEXT, COLOR_BG);
    tft.setTextSize(1);
    tft.setCursor(x + 26, y + 4);
    tft.print("*");
  }
}

void drawHeader()
{
  // Clear header area first
  tft.fillRect(0, 0, 240, 40, COLOR_BG);

  // Reset pengaturan TFT
  resetTFTSettings();

  // Ikon koneksi
  tft.drawBitmap(47, 19, image_download_bits, 14, 16,
                 isConnected ? COLOR_ICON_DL : COLOR_GRAY);

  // Ikon baterai
  drawBatteryIcon(19, 18, batteryLevel);

  // Nama device
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(152, 23);
  tft.print("HALO GUNTUR");
}

void drawTime()
{
  // Clear time area first
  tft.fillRect(0, 40, 240, 90, COLOR_BG);

  // Reset ke pengaturan default terlebih dahulu
  resetTFTSettings();

  // Waktu utama
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(6);
  tft.setTextDatum(TC_DATUM); // Posisi tengah untuk waktu
  tft.drawString(currentTime, tft.width() / 2, 65);

  // Tanggal
  tft.setTextColor(COLOR_ACCENT, COLOR_BG);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM); // Tetap center untuk tanggal
  tft.drawString(currentDate, tft.width() / 2, 123);

  // Reset kembali ke default setelah selesai
  resetTFTSettings();
}

void drawNotifications()
{
  // Clear notification area first
  tft.fillRect(0, 160, 240, 80, COLOR_BG);

  // Pastikan menggunakan pengaturan default
  resetTFTSettings();

  // Ikon notifikasi
  tft.drawBitmap(17, 166, image_download_1_bits, 16, 16, COLOR_ICON_NOTIF);

  // Label notifikasi
  tft.setTextColor(COLOR_NOTIF_TEXT, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(42, 169);
  tft.print("Notifikasi (");
  tft.print(notificationCount);
  tft.print(")");

  // Kotak notifikasi
  tft.drawRect(20, 186, 200, 47, COLOR_BOX); // Reduced width for 240px screen

  // Isi notifikasi
  tft.setCursor(32, 191);
  if (notificationCount > 0)
  {
    String displayNotif = lastNotification.length() > 25 ? lastNotification.substring(0, 25) + "..." : lastNotification;
    tft.print(displayNotif);
  }
  else
  {
    tft.print("Tidak ada notifikasi");
  }
}

void drawWeather()
{
  if (!hasWeather)
    return;

  // Clear weather area first
  tft.fillRect(0, 140, 240, 20, COLOR_BG);

  // Reset pengaturan
  resetTFTSettings();

  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextSize(1);
  tft.setCursor(20, 150);
  tft.printf("Cuaca: %.1fC (H:%.0f L:%.0f)",
             currentWeather.temp,
             currentWeather.high,
             currentWeather.low);
}

void drawMainScreen()
{
  isNavScreen = false;

  // PENTING: Reset semua pengaturan TFT ke default
  tft.fillScreen(COLOR_BG);
  resetTFTSettings();

  drawHeader();
  drawTime();
  drawWeather();
  drawNotifications();
}

/**************  helper untuk zoom bitmap 1‑bit **************/
void drawScaledBitmap(int x, int y,
                      const uint8_t *bitmap,
                      int w, int h,
                      uint16_t color,
                      int scale) // contoh: scale=2 ➜ 2×
{
  for (int j = 0; j < h; j++)
  {
    for (int i = 0; i < w; i++)
    {
      int byteIndex = (j * w + i) >> 3; // /8
      int bitIndex = 7 - (i & 7);       // %8
      if (bitmap[byteIndex] & (1 << bitIndex))
      {
        tft.fillRect(x + i * scale,
                     y + j * scale,
                     scale, scale,
                     color);
      }
    }
  }
}

/**************  navigation screen **************/
void drawNavigationScreen()
{
  isNavScreen = true;
  tft.fillScreen(COLOR_BG);

  // konstanta layar
  const int screenW = 240;
  const int screenH = 240;
  const int margin = 10;
  const int bmpW = 48;             // bitmap asli 48×48
  const int scale = 2;             // perbesar 2×
  const int arrowW = bmpW * scale; // 96 px
  const int lineH = 20;
  const int maxPer = 28;

  /**** header ETA ****/
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextColor(COLOR_ACCENT, COLOR_BG);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("Estimasi sampai:", screenW / 2, margin);

  String timeStr = (navData.eta.length() >= 4)
                       ? navData.eta.substring(0, 2) + ":" + navData.eta.substring(2)
                       : navData.eta;

  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.drawString(timeStr, screenW / 2, margin + 22);

  /**** jarak ****/
  tft.setFreeFont(&FreeSansBold9pt7b);
  String distStr = navData.distance;
  if (navData.distance.toFloat() < 1.0)
  {
    distStr = String(int(navData.distance.toFloat() * 1000)) + " m";
  }
  tft.drawString(distStr, screenW / 2, margin + 60);

  /**** panah (icon 48×48 dari Chronos, di‑scale) ****/
  int arrowX = (screenW - arrowW) / 2;
  int arrowY = margin + 80;

  drawScaledBitmap(arrowX, arrowY,
                   navData.icon, 48, 48,
                   COLOR_ACCENT, scale);

  /**** teks arah ****/
  int destY = arrowY + arrowW + 10;
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setTextDatum(TC_DATUM);

  String txt = "Arah: " + navData.directions;
  while (txt.length() && destY < screenH - margin)
  {
    int breakPos = -1;
    for (int i = 0; i < txt.length() && i < maxPer; i++)
      if (txt[i] == ' ')
        breakPos = i;

    String line;
    if (txt.length() <= maxPer)
    {
      line = txt;
      txt = "";
    }
    else if (breakPos > 0)
    {
      line = txt.substring(0, breakPos);
      txt = txt.substring(breakPos + 1);
    }
    else
    {
      line = txt.substring(0, maxPer - 3) + "...";
      txt = "";
    }
    tft.drawString(line, screenW / 2, destY);
    destY += lineH;
  }
}

// =============================================
// CALLBACK FUNCTIONS
// =============================================
void connectionCallback(bool state)
{
  isConnected = state;
  if (!isNavScreen)
  {
    // Reset settings sebelum update
    resetTFTSettings();
    drawHeader();
  }
}

void notificationCallback(Notification notif)
{
  notificationCount = watch.getNotificationCount();
  lastNotification = notif.title + ": " + notif.message;
  if (!isNavScreen)
  {
    // Reset settings sebelum update
    resetTFTSettings();
    drawNotifications();
  }
}

void configCallback(Config config, uint32_t a, uint32_t b)
{
  switch (config)
  {
  case CF_PBAT:
    batteryLevel = b;
    isCharging = (a == 1);
    if (!isNavScreen)
    {
      resetTFTSettings();
      drawHeader();
    }
    break;

  case CF_WEATHER:
    if (a == 2 && watch.getWeatherCount())
    {
      currentWeather = watch.getWeatherAt(0);
      hasWeather = true;
      if (!isNavScreen)
      {
        resetTFTSettings();
        drawWeather();
      }
    }
    break;

  case CF_NAV_DATA:
    navActive = a;
    navData = watch.getNavigation();
    navChanged = true;
    break;

  case CF_NAV_ICON:
    if (a == 2)
    {
      Navigation nd = watch.getNavigation();
      if (navIconCRC != nd.iconCRC)
      {
        navIconCRC = nd.iconCRC;
        memcpy(navData.icon, nd.icon, sizeof(navData.icon));
        navChanged = true;
      }
    }
    break;
  }

  // Handle navigation state changes properly
  if (!navActive && isNavScreen)
  {
    // Navigation closed, return to main screen
    delay(100);         // Small delay to ensure clean transition
    resetTFTSettings(); // PENTING: Reset settings sebelum draw
    drawMainScreen();
    navChanged = false;
  }
  else if (navActive && navChanged)
  {
    // Navigation active, show navigation screen
    drawNavigationScreen();
    navChanged = false;
  }
}

// =============================================
// SETUP & LOOP
// =============================================
void setup()
{
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_BG);

  // Splash screen
  drawBootSplash(tft);
  delay(3000);

  // Inisialisasi pengaturan TFT default
  resetTFTSettings();

  // Inisialisasi tampilan
  currentTime = watch.getHourC() + watch.getTime(":%M");
  currentDate = watch.getDate();
  drawMainScreen();

  // Setup Chronos
  watch.setConnectionCallback(connectionCallback);
  watch.setNotificationCallback(notificationCallback);
  watch.setConfigurationCallback(configCallback);
  watch.begin();
  watch.set24Hour(false); // Format 12 jam
}

void loop()
{
  watch.loop();

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000)
  {
    lastUpdate = millis();

    // Update waktu
    String newTime = watch.getHourC() + watch.getTime(":%M");
    if (newTime != currentTime)
    {
      currentTime = newTime;
      currentDate = watch.getDate();
      if (!isNavScreen)
      {
        // Reset settings sebelum update waktu
        resetTFTSettings();
        drawTime();
      }
    }

    // Update notifikasi
    int newCount = watch.getNotificationCount();
    if (newCount != notificationCount)
    {
      notificationCount = newCount;
      if (!isNavScreen)
      {
        // Reset settings sebelum update notifikasi
        resetTFTSettings();
        drawNotifications();
      }
    }

    // Remove the problematic pulse effect from loop
    // Navigation updates are now handled in configCallback only
  }
}