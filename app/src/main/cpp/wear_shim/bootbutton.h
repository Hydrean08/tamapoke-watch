#pragma once
#include <cstdint>
struct BootButton {
  enum Action : uint8_t { NONE, SINGLE, DOUBLE, LONG };
  void begin(bool, uint32_t) {}
  Action update(bool, uint32_t) { return NONE; }
  bool active() const { return false; }
};
struct PwrButton {
  void begin(bool, uint32_t) {}
  bool update(bool, uint32_t) { return false; }
  bool active() const { return false; }
};
