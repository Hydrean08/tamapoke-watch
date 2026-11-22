#pragma once

#include <cstdint>
#include "Wire.h"

class TouchDrvCST92xx {
 public:
  void setPins(int, int) {}
  bool begin(TwoWire &, uint8_t, int, int) { return true; }
  void reset() {}
  void setMaxCoordinates(int16_t w, int16_t h) {
    maxW_ = w;
    maxH_ = h;
  }
  void setMirrorXY(bool, bool) {}
  uint8_t getPoint(int16_t *x, int16_t *y, uint8_t count);

 private:
  int16_t maxW_ = 466;
  int16_t maxH_ = 466;
};

// Feed Android MotionEvent actions (0=DOWN, 1=UP, 2=MOVE, 3=CANCEL) in 0..465 space.
void wearTouchPush(int action, int16_t x, int16_t y);
void wearTouchReset();
bool wearTouchHasPending();
