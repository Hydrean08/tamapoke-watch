#pragma once
#include <cstdint>
struct ScreenLock {
  bool locked() const { return false; }
  bool blank(uint32_t) const { return false; }
  bool toggle() { return false; }
  void frameShown(uint32_t) {}
};
