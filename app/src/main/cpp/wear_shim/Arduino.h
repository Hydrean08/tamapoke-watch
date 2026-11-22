#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define FALLING 2

#ifndef PI
#define PI 3.14159265358979323846
#endif

using std::abs;

template <class T>
static inline T min(T a, T b) {
  return a < b ? a : b;
}

template <class T>
static inline T max(T a, T b) {
  return a > b ? a : b;
}

template <class T, class A, class B>
static inline T constrain(T x, A low, B high) {
  T lo = static_cast<T>(low);
  T hi = static_cast<T>(high);
  return x < lo ? lo : (x > hi ? hi : x);
}

static inline void *ps_malloc(size_t size) { return std::malloc(size); }

class String {
 public:
  String() = default;
  String(const char *s) : s_(s ? s : "") {}
  String(const std::string &s) : s_(s) {}

  size_t length() const { return s_.size(); }
  const char *c_str() const { return s_.c_str(); }

  void trim() {
    size_t start = 0;
    while (start < s_.size() && (unsigned char)s_[start] <= ' ') start++;
    size_t end = s_.size();
    while (end > start && (unsigned char)s_[end - 1] <= ' ') end--;
    s_ = s_.substr(start, end - start);
  }

  bool startsWith(const char *prefix) const {
    if (!prefix) return false;
    size_t n = std::strlen(prefix);
    return s_.size() >= n && s_.compare(0, n, prefix) == 0;
  }

  String substring(int start) const {
    if (start <= 0) return *this;
    if ((size_t)start >= s_.size()) return String("");
    return String(s_.substr((size_t)start));
  }

  String substring(int start, int end) const {
    if (start < 0) start = 0;
    if (end <= start || (size_t)start >= s_.size()) return String("");
    size_t len = (size_t)end > s_.size() ? s_.size() - (size_t)start : (size_t)(end - start);
    return String(s_.substr((size_t)start, len));
  }

  long toInt() const { return std::strtol(s_.c_str(), nullptr, 10); }

  int indexOf(const char *needle) const {
    if (!needle) return -1;
    size_t pos = s_.find(needle);
    return pos == std::string::npos ? -1 : (int)pos;
  }

  int lastIndexOf(char ch) const {
    size_t pos = s_.rfind(ch);
    return pos == std::string::npos ? -1 : (int)pos;
  }

  bool operator==(const char *other) const { return s_ == (other ? other : ""); }
  bool operator!=(const char *other) const { return !(*this == other); }
  bool operator==(const String &other) const { return s_ == other.s_; }
  bool operator!=(const String &other) const { return s_ != other.s_; }

  friend String operator+(const char *lhs, const String &rhs) {
    return String(std::string(lhs ? lhs : "") + rhs.s_);
  }

 private:
  std::string s_;
};

uint32_t millis();
void delay(uint32_t ms);
void wearClockSetPaused(bool paused);
void wearClockReset();

long random(long howbig);
long random(long howsmall, long howbig);
void randomSeed(unsigned long seed);

static inline void pinMode(uint8_t, uint8_t) {}
static inline void digitalWrite(uint8_t, uint8_t) {}
static inline int digitalRead(uint8_t) { return HIGH; }
static inline int digitalPinToInterrupt(uint8_t pin) { return pin; }
static inline void attachInterrupt(int, void (*)(), int) {}
static inline void detachInterrupt(int) {}

struct WearSerial {
  void begin(unsigned long) {}
  void setRxBufferSize(size_t) {}
  void setTxTimeoutMs(uint32_t) {}
  void setTimeout(uint32_t) {}
  int available() { return 0; }
  String readStringUntil(char) { return String(""); }
  size_t readBytes(uint8_t *, size_t) { return 0; }
  int printf(const char *fmt, ...);
  void print(const char *s);
  void println(const char *s);
  void println();
};
extern WearSerial Serial;

struct WearEsp {
  uint32_t getFreeHeap() const;
  uint32_t getMinFreeHeap() const;
  void restart() {}
};
extern WearEsp ESP;
