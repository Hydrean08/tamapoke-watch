#pragma once
#include <cstdint>
struct TrainingTouch {
  enum Action : uint8_t { NONE, HIT, EXIT };
  static constexpr int EXIT_X = 153, EXIT_Y = 384, EXIT_W = 160, EXIT_H = 48;
};
struct TrainingReport {
  enum Kind : uint8_t { ERROR, CONTACT, RELEASE, EMPTY, COVER };
  bool valid = false, pressed = false;
  int16_t x = 0, y = 0;
  Kind kind = ERROR;
  bool attempted = false, irq = false;
  uint8_t bytes = 0, raw[12] = {};
};
struct TrainingTrace { TrainingReport report; uint32_t at = 0; uint8_t action = 0; };
struct TrainingEvent { TrainingTouch::Action action = TrainingTouch::NONE; uint32_t at = 0; };
struct TrainingCapture {
  struct Frame { Frame(TrainingCapture &, bool) {} };
  bool begin(TrainingReport (*)()) { return false; }
  void reset(bool = false) {}
  void poll() {}
  bool down() const { return false; }
  bool waitingRelease() const { return false; }
  uint32_t exitHeldMs(uint32_t) const { return 0; }
  bool next(TrainingEvent &) { return false; }
  bool nextTrace(TrainingTrace &) { return false; }
  void setTracing(bool) {}
  bool tracing() const { return false; }
  uint32_t dropped() const { return 0; }
  uint32_t traceDropped() const { return 0; }
};
