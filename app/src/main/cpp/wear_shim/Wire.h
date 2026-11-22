#pragma once

#include <cstddef>
#include <cstdint>

class TwoWire {
 public:
  void begin(int = 0, int = 0) {}
  void setTimeOut(uint16_t) {}
  void setTimeout(unsigned long) {}
  void beginTransmission(uint8_t) {}
  uint8_t endTransmission(bool = true) { return 0; }
  size_t write(uint8_t) { return 1; }
  size_t requestFrom(uint8_t, size_t) { return 0; }
  size_t readBytes(uint8_t *, size_t) { return 0; }
  int available() { return 0; }
  int read() { return 0; }
};

extern TwoWire Wire;
