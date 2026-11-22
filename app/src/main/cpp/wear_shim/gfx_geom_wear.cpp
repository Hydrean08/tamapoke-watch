#include "Arduino_GFX_Library.h"

#include <algorithm>

void Arduino_Canvas::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
  int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
  while (x < y) {
    if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) { putPixel(x0 + x, y0 + y, color); putPixel(x0 + y, y0 + x, color); }
    if (cornername & 0x2) { putPixel(x0 + x, y0 - y, color); putPixel(x0 + y, y0 - x, color); }
    if (cornername & 0x8) { putPixel(x0 - y, y0 + x, color); putPixel(x0 - x, y0 + y, color); }
    if (cornername & 0x1) { putPixel(x0 - y, y0 - x, color); putPixel(x0 - x, y0 - y, color); }
  }
}

void Arduino_Canvas::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, uint16_t color) {
  int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r, px = x, py = y;
  delta++;
  while (x < y) {
    if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (x < (y + 1)) {
      if (corners & 1) drawVLine(x0 + x, y0 - y, 2 * y + delta, color);
      if (corners & 2) drawVLine(x0 - x, y0 - y, 2 * y + delta, color);
    }
    if (y != py) {
      if (corners & 1) drawVLine(x0 + py, y0 - px, 2 * px + delta, color);
      if (corners & 2) drawVLine(x0 - py, y0 - px, 2 * px + delta, color);
      py = y;
    }
    px = x;
  }
}

void Arduino_Canvas::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  if (r < 0) return;
  int16_t f = 1 - r, ddF_x = 1, ddF_y = -2 * r, x = 0, y = r;
  putPixel(x0, y0 + r, color);
  putPixel(x0, y0 - r, color);
  putPixel(x0 + r, y0, color);
  putPixel(x0 - r, y0, color);
  while (x < y) {
    if (f >= 0) { y--; ddF_y += 2; f += ddF_y; }
    x++;
    ddF_x += 2;
    f += ddF_x;
    putPixel(x0 + x, y0 + y, color);
    putPixel(x0 - x, y0 + y, color);
    putPixel(x0 + x, y0 - y, color);
    putPixel(x0 - x, y0 - y, color);
    putPixel(x0 + y, y0 + x, color);
    putPixel(x0 - y, y0 + x, color);
    putPixel(x0 + y, y0 - x, color);
    putPixel(x0 - y, y0 - x, color);
  }
}

void Arduino_Canvas::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  if (r < 0) return;
  drawVLine(x0, y0 - r, 2 * r + 1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void Arduino_Canvas::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
  if (w <= 0 || h <= 0) return;
  int16_t maxR = ((w < h) ? w : h) / 2;
  if (r > maxR) r = maxR;
  if (r < 0) r = 0;
  drawHLine(x + r, y, w - 2 * r, color);
  drawHLine(x + r, y + h - 1, w - 2 * r, color);
  drawVLine(x, y + r, h - 2 * r, color);
  drawVLine(x + w - 1, y + r, h - 2 * r, color);
  drawCircleHelper(x + r, y + r, r, 1, color);
  drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
  drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

void Arduino_Canvas::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
  if (w <= 0 || h <= 0) return;
  int16_t maxR = ((w < h) ? w : h) / 2;
  if (r > maxR) r = maxR;
  if (r < 0) r = 0;
  fillRect(x + r, y, w - 2 * r, h, color);
  fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
  fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void Arduino_Canvas::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
  if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }
  int16_t dx = x1 - x0, dy = std::abs(y1 - y0), err = dx / 2, ystep = (y0 < y1) ? 1 : -1;
  for (; x0 <= x1; x0++) {
    if (steep) putPixel(y0, x0, color);
    else putPixel(x0, y0, color);
    err -= dy;
    if (err < 0) { y0 += ystep; err += dx; }
  }
}

void Arduino_Canvas::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
  if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
  if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
  if (y0 == y2) {
    int16_t a = std::min(x0, std::min(x1, x2));
    int16_t b = std::max(x0, std::max(x1, x2));
    drawHLine(a, y0, b - a + 1, color);
    return;
  }
  int32_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;
  int16_t last = (y1 == y2) ? y1 : y1 - 1;
  for (int16_t y = y0; y <= last; y++) {
    int32_t a = x0 + sa / dy01, b = x0 + sb / dy02;
    sa += dx01; sb += dx02;
    if (a > b) std::swap(a, b);
    drawHLine(static_cast<int16_t>(a), y, static_cast<int16_t>(b - a + 1), color);
  }
  sa = dx12 * (last + 1 - y1);
  sb = dx02 * (last + 1 - y0);
  for (int16_t y = last + 1; y <= y2; y++) {
    int32_t a = x1 + sa / dy12, b = x0 + sb / dy02;
    sa += dx12; sb += dx02;
    if (a > b) std::swap(a, b);
    drawHLine(static_cast<int16_t>(a), y, static_cast<int16_t>(b - a + 1), color);
  }
}
