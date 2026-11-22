#pragma once

#include <cstdint>
#include <cstddef>

#define GFX_NOT_DEFINED (-1)
#define RGB565_BLACK 0x0000
#define RGB565_WHITE 0xFFFF

class Arduino_DataBus {
 public:
  virtual ~Arduino_DataBus() = default;
  virtual void writePixels(uint16_t *, uint32_t) {}
};

class Arduino_ESP32QSPI : public Arduino_DataBus {
 public:
  Arduino_ESP32QSPI(int8_t, int8_t, int8_t, int8_t, int8_t, int8_t) {}
};

class Arduino_CO5300 {
 public:
  Arduino_CO5300(Arduino_DataBus *, int8_t, uint8_t, int16_t, int16_t, uint8_t, uint8_t, uint8_t, uint8_t) {}
  void setBrightness(uint8_t) {}
  void displayOn() {}
  void displayOff() {}
  void startWrite() {}
  void writeAddrWindow(int16_t, int16_t, uint16_t, uint16_t) {}
  void endWrite() {}
};

class Arduino_Canvas {
 public:
  Arduino_Canvas(int16_t w, int16_t h, Arduino_CO5300 * = nullptr);
  ~Arduino_Canvas();

  bool begin(int32_t speed = 0);
  void fillScreen(uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

  void setFont(const void *f = nullptr);
  void setUTF8Print(bool utf8);
  void setTextColor(uint16_t color);
  void setTextSize(uint8_t s);
  void setCursor(int16_t x, int16_t y);
  int16_t getCursorX() const { return curX_; }
  int16_t getCursorY() const { return curY_; }
  void getTextBounds(const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
  size_t print(char c);
  size_t print(const char *s);

  void flush(bool force_flush = false);

  const uint16_t *frontBuffer() const { return front_; }
  bool takeFrameDirty();
  int16_t width() const { return w_; }
  int16_t height() const { return h_; }

 protected:
  int16_t w_;
  int16_t h_;
  uint16_t *back_ = nullptr;
  uint16_t *front_ = nullptr;
  bool dirty_ = false;
  bool utf8_ = false;
  const void *font_ = nullptr;
  uint16_t textColor_ = RGB565_WHITE;
  uint8_t textSize_ = 1;
  int16_t curX_ = 0;
  int16_t curY_ = 0;

  void putPixel(int16_t x, int16_t y, uint16_t color);
  void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color);
  void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
  void drawChar5x7(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size);
};

// Optional UTF-8 CJK callback hooks (used when Japanese/Korean language is active).
struct WearUtf8TextHooks {
  void (*measure)(const char *utf8, uint8_t size, int16_t *y1, uint16_t *w, uint16_t *h) = nullptr;
  int16_t (*draw)(const char *utf8, int16_t x, int16_t y, uint8_t size, uint16_t color,
                  uint16_t *buf, int16_t bufW, int16_t bufH) = nullptr;
};
void wearSetUtf8TextHooks(const WearUtf8TextHooks &hooks);
