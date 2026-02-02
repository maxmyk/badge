#include "photo.h"
#include "qrcodegen.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// FOR WEMOS LOLIN32-S2 mini & prototype board
#define TFT_CS 14
#define TFT_RST 18
#define TFT_DC 16
#define TFT_MOSI 33 // SDA
#define TFT_SCLK 35 // SCL

Adafruit_ST7789 tft =
    Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

static const int16_t W = 240;
static const int16_t H = 320;
static const uint8_t SLEEP_SECONDS = 15;

uint8_t qrTemp[qrcodegen_BUFFER_LEN_MAX];
uint8_t qrData[qrcodegen_BUFFER_LEN_MAX];

enum ScreenId : uint8_t { SCR_INTRO = 0, SCR_ABOUT = 1, SCR_QR = 2, LENGTH };
ScreenId screen = SCR_INTRO;

static void setPortraitRotation();
static void drawFrame(uint8_t glow);
static void drawBadgeArea();
static void drawIntroScreen();
static void drawAboutScreen();
static void drawQRScreen();
static void switchScreen(ScreenId next);
static void drawRGB565_P_scaled(int16_t x, int16_t y, const uint16_t *src,
                                int16_t sw, int16_t sh, int16_t dw, int16_t dh);

const int flag_w = 45;
const int flag_h = 30;

// Modify as you wish
const char *NAME = "MAKSYM";
const char *SURNAME = "MYKHASYUTA";
const char *TITLE = "Software Engineer";
// TODO: redesign this bit
const char *BLURB =
    "C++ / Embedded\n\n2+ years of R&D\nexperience\n\nI specialize in\nmaking "
    "systems from prototype to stable 24/7 operation\n\nAlso, I build "
    "fun\nstuff in my spare \ntime.";

static void setPortraitRotation() {
  tft.setRotation(2); // portrait, use 0 if upside down
}

static void drawFrame(uint8_t glow) {
  uint16_t c = tft.color565(glow, glow, glow);
  tft.drawRect(0, 0, W, H, c);
  tft.drawRect(1, 1, W - 2, H - 2, c);
}

static void drawBadgeArea() {
  tft.fillRect(W - flag_w - 2, 0, flag_w + 2, flag_h + 2, ST77XX_BLACK);
  tft.fillRect(W - flag_w - 1, 1, flag_w, flag_h / 2, ST77XX_BLUE);
  tft.fillRect(W - flag_w - 1, flag_h / 2 + 1, flag_w, flag_h / 2,
               ST77XX_YELLOW);
}

static void drawRGB565_P_scaled(int16_t x, int16_t y, const uint16_t *src,
                                int16_t sw, int16_t sh, int16_t dw,
                                int16_t dh) {
  if (!src)
    return;

  tft.startWrite();
  tft.setAddrWindow(x, y, dw, dh);

  for (int16_t j = 0; j < dh; j++) {
    int16_t sy = (int32_t)j * sh / dh;
    for (int16_t i = 0; i < dw; i++) {
      int16_t sx = (int32_t)i * sw / dw;
      uint16_t c = pgm_read_word(src + sy * sw + sx);
      tft.writeColor(c, 1);
    }
  }

  tft.endWrite();
}

static void drawIntroScreen() {
  tft.fillScreen(ST77XX_BLACK);

  drawRGB565_P_scaled(0, 0, photo, photo_width, photo_height, 240, 240);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 245);
  tft.print(NAME);
  tft.setCursor(0, 270);
  tft.print(SURNAME);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 295);
  tft.print(TITLE);

  drawBadgeArea();
}

static void drawAboutScreen() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 5);
  tft.print(NAME);
  tft.setCursor(0, 30);
  tft.print(SURNAME);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 55);
  tft.print(TITLE);

  tft.setCursor(0, 88);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.print(BLURB);

  tft.setCursor(W - W / 2, H - 20);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setTextSize(2);
  tft.print("Say HI! =)");

  drawBadgeArea();
}
void drawQR(const char *text, int x, int y, int sizePx) {
  bool ok = qrcodegen_encodeText(text, qrTemp, qrData, qrcodegen_Ecc_LOW,
                                 qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
                                 qrcodegen_Mask_AUTO, true);

  if (!ok)
    return;

  int qrSize = qrcodegen_getSize(qrData);
  int scale = sizePx / qrSize;
  int x_center = (W - qrSize * scale) / 2;

  for (int yy = 0; yy < qrSize; yy++) {
    for (int xx = 0; xx < qrSize; xx++) {
      uint16_t color =
          qrcodegen_getModule(qrData, xx, yy) ? ST77XX_BLACK : ST77XX_WHITE;
      if (color == ST77XX_WHITE) { // minimal optimization
        tft.fillRect(x + x_center + xx * scale, y + yy * scale, scale, scale,
                     color);
      }
    }
  }
}

static void drawQRScreen() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 10);
  tft.print("  Scan me!");

  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(8, 44);
  tft.print("Website / portfolio");
  drawQR("https://max436.com", 0, 65, 240);

  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(0, 300);
  tft.print(" https://max436.com");

  drawBadgeArea();
}

static void switchScreen(ScreenId next) {
  screen = next;
  if (screen == SCR_INTRO)
    drawIntroScreen();
  else if (screen == SCR_ABOUT)
    drawAboutScreen();
  else
    drawQRScreen();
}

void setup() {
  tft.init(W, H);
  setPortraitRotation();
  tft.setTextWrap(true);

  switchScreen(SCR_INTRO);
  sleep(SLEEP_SECONDS);
}

void loop() {
  ScreenId next = (ScreenId)((screen + 1) % LENGTH);
  switchScreen(next);
  sleep(SLEEP_SECONDS);
}
