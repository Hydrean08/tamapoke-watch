#include "TouchDrvCSTXXX.hpp"

#include <cstdlib>
#include <deque>
#include <mutex>

extern volatile bool gTouchIrq;

namespace {
constexpr int ACTION_DOWN = 0;
constexpr int ACTION_UP = 1;
constexpr int ACTION_MOVE = 2;
constexpr int ACTION_CANCEL = 3;
constexpr int ACTION_POINTER_DOWN = 5;
constexpr int ACTION_POINTER_UP = 6;
constexpr int SWIPE_INTERMEDIATE_THRESHOLD = 10;

struct TouchSample {
  bool pressed;
  int16_t x;
  int16_t y;
};

std::mutex gTouchMux;
std::deque<TouchSample> gQueue;
TouchSample gCurrent = {false, 0, 0};
bool gActiveFinger = false;
int16_t gDownX = 0;
int16_t gDownY = 0;
}  // namespace

void wearTouchReset() {
  std::lock_guard<std::mutex> lock(gTouchMux);
  gQueue.clear();
  gCurrent = {false, 0, 0};
  gActiveFinger = false;
}

void wearTouchPush(int action, int16_t x, int16_t y) {
  std::lock_guard<std::mutex> lock(gTouchMux);
  if (action == ACTION_DOWN || action == ACTION_POINTER_DOWN) {
    gActiveFinger = true;
    gDownX = x;
    gDownY = y;
    // Collapse redundant idle release if a quick second tap starts immediately.
    if (!gQueue.empty() && !gQueue.back().pressed) {
      gQueue.push_back({true, x, y});
    } else {
      gQueue.push_back({true, x, y});
    }
    gTouchIrq = true;
  } else if (action == ACTION_MOVE) {
    if (!gActiveFinger) return;
    // Preserve the initial DOWN sample in front; replace only a trailing MOVE sample.
    if (gQueue.size() >= 2 && gQueue.back().pressed) {
      gQueue.back() = {true, x, y};
    } else {
      gQueue.push_back({true, x, y});
    }
    gTouchIrq = true;
  } else if (action == ACTION_UP || action == ACTION_CANCEL || action == ACTION_POINTER_UP) {
    if (!gActiveFinger) return;
    gActiveFinger = false;
    // Ensure a fast flick delivers (true, end) before (false, end) so tXl/tYl records dx/dy.
    int dx = std::abs(x - gDownX);
    int dy = std::abs(y - gDownY);
    if ((dx > SWIPE_INTERMEDIATE_THRESHOLD || dy > SWIPE_INTERMEDIATE_THRESHOLD) &&
        (gQueue.empty() || !gQueue.back().pressed || gQueue.back().x != x || gQueue.back().y != y)) {
      gQueue.push_back({true, x, y});
    }
    gQueue.push_back({false, x, y});
    gTouchIrq = true;
  }
}

bool wearTouchHasPending() {
  std::lock_guard<std::mutex> lock(gTouchMux);
  return !gQueue.empty() || gCurrent.pressed;
}

uint8_t TouchDrvCST92xx::getPoint(int16_t *x, int16_t *y, uint8_t count) {
  if (!x || !y || count == 0) return 0;
  std::lock_guard<std::mutex> lock(gTouchMux);
  if (!gQueue.empty()) {
    gCurrent = gQueue.front();
    gQueue.pop_front();
    if (!gQueue.empty()) gTouchIrq = true;
  }
  if (!gCurrent.pressed) return 0;
  int16_t cx = gCurrent.x < 0 ? 0 : (gCurrent.x >= maxW_ ? maxW_ - 1 : gCurrent.x);
  int16_t cy = gCurrent.y < 0 ? 0 : (gCurrent.y >= maxH_ ? maxH_ - 1 : gCurrent.y);
  x[0] = cx;
  y[0] = cy;
  return 1;
}
