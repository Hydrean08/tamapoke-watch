#include "Arduino_GFX_Library.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

Arduino_Canvas::Arduino_Canvas(int16_t w, int16_t h, Arduino_CO5300 *) : w_(w), h_(h) {}

Arduino_Canvas::~Arduino_Canvas() {
  std::free(back_);
  std::free(front_);
}

bool Arduino_Canvas::begin(int32_t) {
  if (w_ <= 0 || h_ <= 0) return false;
  size_t bytes = static_cast<size_t>(w_) * static_cast<size_t>(h_) * sizeof(uint16_t);
  if (!back_) back_ = static_cast<uint16_t *>(std::calloc(1, bytes));
  if (!front_) front_ = static_cast<uint16_t *>(std::calloc(1, bytes));
  return back_ != nullptr && front_ != nullptr;
}

void Arduino_Canvas::putPixel(int16_t x, int16_t y, uint16_t color) {
  if (!back_ || x < 0 || y < 0 || x >= w_ || y >= h_) return;
  back_[static_cast<size_t>(y) * w_ + x] = color;
}

void Arduino_Canvas::drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  if (!back_ || y < 0 || y >= h_ || w <= 0) return;
  int32_t x0 = x < 0 ? 0 : x;
  int32_t x1 = static_cast<int32_t>(x) + w;
  if (x1 > w_) x1 = w_;
  if (x0 >= x1) return;
  uint16_t *row = back_ + static_cast<size_t>(y) * w_ + x0;
  std::fill(row, row + (x1 - x0), color);
}

void Arduino_Canvas::drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  if (!back_ || x < 0 || x >= w_ || h <= 0) return;
  int32_t y0 = y < 0 ? 0 : y;
  int32_t y1 = static_cast<int32_t>(y) + h;
  if (y1 > h_) y1 = h_;
  for (int32_t yy = y0; yy < y1; yy++) {
    back_[static_cast<size_t>(yy) * w_ + x] = color;
  }
}

void Arduino_Canvas::fillScreen(uint16_t color) {
  if (!back_) return;
  std::fill(back_, back_ + static_cast<size_t>(w_) * h_, color);
}

void Arduino_Canvas::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!back_ || w <= 0 || h <= 0) return;
  int32_t x0 = x < 0 ? 0 : x;
  int32_t y0 = y < 0 ? 0 : y;
  int32_t x1 = static_cast<int32_t>(x) + w;
  int32_t y1 = static_cast<int32_t>(y) + h;
  if (x1 > w_) x1 = w_;
  if (y1 > h_) y1 = h_;
  if (x0 >= x1 || y0 >= y1) return;
  int32_t span = x1 - x0;
  for (int32_t yy = y0; yy < y1; yy++) {
    uint16_t *row = back_ + static_cast<size_t>(yy) * w_ + x0;
    std::fill(row, row + span, color);
  }
}

void Arduino_Canvas::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (w <= 0 || h <= 0) return;
  drawHLine(x, y, w, color);
  drawHLine(x, y + h - 1, w, color);
  drawVLine(x, y, h, color);
  drawVLine(x + w - 1, y, h, color);
}

void Arduino_Canvas::flush(bool) {
  if (!back_ || !front_) return;
  std::memcpy(front_, back_, static_cast<size_t>(w_) * static_cast<size_t>(h_) * sizeof(uint16_t));
  dirty_ = true;
}

bool Arduino_Canvas::takeFrameDirty() {
  bool was = dirty_;
  dirty_ = false;
  return was;
}
