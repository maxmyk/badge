#include "flag.h"
#include "photo.h"
#include "qrcodegen.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>

// FOR WEMOS LOLIN32-S2 mini & prototype board
#define TFT_CS 14
#define TFT_RST 18
#define TFT_DC 16
#define TFT_MOSI 33  // SDA
#define TFT_SCLK 35  // SCL

SPIClass displaySPI(FSPI);

Adafruit_ST7789 tft = Adafruit_ST7789(&displaySPI, TFT_CS, TFT_DC, TFT_RST);
static const int16_t W = 240;
static const int16_t H = 320;

static const uint8_t SCREEN_SECONDS = 15;

static const uint32_t TFT_SPI_SPEED = 8000000;  // 16000000

uint8_t qrTemp[qrcodegen_BUFFER_LEN_MAX];
uint8_t qrData[qrcodegen_BUFFER_LEN_MAX];

enum ScreenId : uint8_t {
  SCR_INTRO = 0,
  SCR_ABOUT = 1,
  SCR_QR = 2,
  SCREEN_COUNT
};

ScreenId screen = SCR_INTRO;

const char *NAME = "MAKSYM";
const char *SURNAME = "MYKHASYUTA";
const char *TITLE = "Software Engineer";
const char *QR_URL = "https://maxmyk.ca/contacts";

static void setupPowerSaving();
static void waitBetweenScreens();

static void initDisplayBus();
static void hardResetDisplay();
static void reinitDisplay();
static void beginScreen();

static void switchScreen(ScreenId next);

static void drawIntroScreen();
static void drawAboutScreen();
static void drawQRScreen();

static void drawFlag();
static void drawCenteredText(const char *text, int16_t y, uint8_t size,
                             uint16_t color);
static void drawRGB565_P_scaled(int16_t x, int16_t y, const uint16_t *src,
                                int16_t sw, int16_t sh, int16_t dw, int16_t dh);
static void drawRGB565_P(int16_t x, int16_t y, const uint16_t *src, int16_t sw,
                         int16_t sh);
static void drawQR(const char *text, int16_t x, int16_t y, int16_t sizePx);

static void setupPowerSaving() {
  WiFi.mode(WIFI_OFF);
  WiFi.disconnect(true);

#ifdef TFT_BL  // for some displays, untested
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
#endif
}

static void waitBetweenScreens() {
  delay((uint32_t)SCREEN_SECONDS * 1000UL);
}

static void initDisplayBus() {
  displaySPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
}

static void hardResetDisplay() {
  pinMode(TFT_RST, OUTPUT);

  digitalWrite(TFT_RST, HIGH);
  delay(20);

  digitalWrite(TFT_RST, LOW);
  delay(50);

  digitalWrite(TFT_RST, HIGH);
  delay(150);
}

static void reinitDisplay() {
  hardResetDisplay();

  tft.init(W, H);
  tft.setSPISpeed(TFT_SPI_SPEED);
  tft.setRotation(2);
  tft.setTextWrap(true);

  delay(10);
}

static void beginScreen() {
  tft.fillScreen(ST77XX_BLACK);
  delay(30);

  reinitDisplay();

  tft.fillScreen(ST77XX_BLACK);
}

static void drawCenteredText(const char *text, int16_t y, uint8_t size,
                             uint16_t color) {
  tft.setTextSize(size);
  tft.setTextColor(color);

  int16_t x1;
  int16_t y1;
  uint16_t tw;
  uint16_t th;

  tft.getTextBounds(text, 0, y, &x1, &y1, &tw, &th);

  int16_t x = (W - tw) / 2;
  if (x < 0) {
    x = 0;
  }

  tft.setCursor(x, y);
  tft.print(text);
}

static void drawRGB565_P(int16_t x, int16_t y, const uint16_t *src, int16_t sw,
                         int16_t sh) {
  if (!src) {
    return;
  }

  tft.startWrite();
  tft.setAddrWindow(x, y, sw, sh);

  for (int16_t j = 0; j < sh; j++) {
    for (int16_t i = 0; i < sw; i++) {
      uint16_t c = pgm_read_word(src + j * sw + i);
      tft.writeColor(c, 1);
    }

    yield();
  }

  tft.endWrite();
}

static void drawRGB565_P_scaled(int16_t x, int16_t y, const uint16_t *src,
                                int16_t sw, int16_t sh, int16_t dw,
                                int16_t dh) {
  if (!src) {
    return;
  }

  tft.startWrite();
  tft.setAddrWindow(x, y, dw, dh);

  for (int16_t j = 0; j < dh; j++) {
    int16_t sy = (int32_t)j * sh / dh;

    for (int16_t i = 0; i < dw; i++) {
      int16_t sx = (int32_t)i * sw / dw;
      uint16_t c = pgm_read_word(src + sy * sw + sx);
      tft.writeColor(c, 1);
    }

    yield();
  }

  tft.endWrite();
}

static void drawFlag() {
  const int16_t x = W - flag_width;
  const int16_t y = 0;
  drawRGB565_P(x, y, flag_img, flag_width, flag_height);
}

static void drawQR(const char *text, int16_t x, int16_t y, int16_t sizePx) {
  bool ok = qrcodegen_encodeText(text, qrTemp, qrData, qrcodegen_Ecc_LOW,
                                 qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
                                 qrcodegen_Mask_AUTO, true);

  if (!ok) {
    return;
  }

  int qrSize = qrcodegen_getSize(qrData);
  int scale = sizePx / qrSize;

  if (scale <= 0) {
    return;
  }

  int drawnSize = qrSize * scale;
  int xCenter = (W - drawnSize) / 2;

  for (int yy = 0; yy < qrSize; yy++) {
    for (int xx = 0; xx < qrSize; xx++) {
      bool isBlack = qrcodegen_getModule(qrData, xx, yy);

      if (!isBlack) {
        tft.fillRect(x + xCenter + xx * scale, y + yy * scale, scale, scale,
                     ST77XX_WHITE);
      }
    }

    yield();
  }
}

static void drawIntroScreen() {
  beginScreen();

  drawRGB565_P_scaled(0, 0, photo, photo_width, photo_height, 240, 228);

  drawCenteredText(NAME, 236, 3, ST77XX_WHITE);
  drawCenteredText(SURNAME, 264, 3, ST77XX_WHITE);
  drawCenteredText(TITLE, 294, 2, ST77XX_GREEN);

  drawFlag();
}

static void drawAboutScreen() {
  beginScreen();

  drawCenteredText("About me ", 10, 3, ST77XX_WHITE);
  drawCenteredText(TITLE, 46, 2, ST77XX_GREEN);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 70);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("--------------------");

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(0, 92);

  tft.print("C++ / Embedded / R&D\n\n");
  tft.print("I build real-world\n");
  tft.print("systems from proto-\n");
  tft.print("types to stable 24/7\n");
  tft.print("operation.\n\n");
  tft.print("Also software, hard-\n");
  tft.print("ware, and fun side\n");
  tft.print("projects like this.\n");

  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(0, 268);
  tft.print("--------------------");

  drawCenteredText("Say hi! =)", 292, 2, ST77XX_MAGENTA);

  drawFlag();
}

static void drawQRScreen() {
  beginScreen();

  drawCenteredText("Scan me", 10, 3, ST77XX_WHITE);
  drawCenteredText("Portfolio / contact", 46, 2, ST77XX_YELLOW);

  drawQR(QR_URL, 0, 65, 240);

  drawCenteredText("maxmyk.ca/max436.com", 292, 2, ST77XX_YELLOW);

  drawFlag();
}

static void switchScreen(ScreenId next) {
  screen = next;

  switch (screen) {
    case SCR_INTRO:
      drawIntroScreen();
      break;

    case SCR_ABOUT:
      drawAboutScreen();
      break;

    case SCR_QR:
      drawQRScreen();
      break;

    default:
      screen = SCR_INTRO;
      drawIntroScreen();
      break;
  }
}

void setup() {
  setupPowerSaving();

  initDisplayBus();

  reinitDisplay();

  switchScreen(SCR_INTRO);
}

void loop() {
  waitBetweenScreens();

  ScreenId next = (ScreenId)((screen + 1) % SCREEN_COUNT);
  switchScreen(next);
}