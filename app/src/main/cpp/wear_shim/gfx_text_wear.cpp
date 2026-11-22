#include "Arduino_GFX_Library.h"
#include "glcdfont_cp437.h"

#include <cstring>

namespace {
WearUtf8TextHooks gUtf8Hooks;
}

void wearSetUtf8TextHooks(const WearUtf8TextHooks &hooks) { gUtf8Hooks = hooks; }

void Arduino_Canvas::setFont(const void *f) { font_ = f; }
void Arduino_Canvas::setUTF8Print(bool utf8) { utf8_ = utf8; }
void Arduino_Canvas::setTextColor(uint16_t color) { textColor_ = color; }
void Arduino_Canvas::setTextSize(uint8_t s) { textSize_ = s == 0 ? 1 : s; }
void Arduino_Canvas::setCursor(int16_t x, int16_t y) { curX_ = x; curY_ = y; }

void Arduino_Canvas::drawChar5x7(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size) {
  const unsigned char *glyph = font + static_cast<size_t>(c) * 5u;
  for (int8_t col = 0; col < 5; col++) {
    uint8_t line = glyph[col];
    for (int8_t row = 0; row < 8; row++, line >>= 1) {
      if (line & 1u) {
        if (size == 1) putPixel(x + col, y + row, color);
        else fillRect(x + col * size, y + row * size, size, size, color);
      }
    }
  }
}

void Arduino_Canvas::getTextBounds(const char *str, int16_t x, int16_t y,
                                   int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
  if (!str || !*str) {
    if (x1) *x1 = x;
    if (y1) *y1 = y;
    if (w) *w = 0;
    if (h) *h = 0;
    return;
  }
  if (font_ && utf8_ && gUtf8Hooks.measure) {
    int16_t oy1 = 0;
    uint16_t ow = 0, oh = 0;
    gUtf8Hooks.measure(str, textSize_, &oy1, &ow, &oh);
    if (x1) *x1 = x;
    if (y1) *y1 = static_cast<int16_t>(y + oy1);
    if (w) *w = ow;
    if (h) *h = oh;
    return;
  }
  if (x1) *x1 = x;
  if (y1) *y1 = y;
  if (w) *w = static_cast<uint16_t>(std::strlen(str) * 6u * textSize_);
  if (h) *h = static_cast<uint16_t>(8u * textSize_);
}

size_t Arduino_Canvas::print(char c) {
  char s[2] = {c, '\0'};
  return print(s);
}

size_t Arduino_Canvas::print(const char *s) {
  if (!s || !*s) return 0;
  if (font_ && utf8_ && gUtf8Hooks.draw && back_) {
    curX_ = gUtf8Hooks.draw(s, curX_, curY_, textSize_, textColor_, back_, w_, h_);
    return std::strlen(s);
  }
  size_t len = std::strlen(s);
  for (size_t i = 0; i < len; i++) {
    unsigned char ch = static_cast<unsigned char>(s[i]);
    if (ch == '\n') {
      curX_ = 0;
      curY_ = static_cast<int16_t>(curY_ + 8 * textSize_);
    } else if (ch != '\r') {
      drawChar5x7(curX_, curY_, ch, textColor_, textSize_);
      curX_ = static_cast<int16_t>(curX_ + 6 * textSize_);
    }
  }
  return len;
}
