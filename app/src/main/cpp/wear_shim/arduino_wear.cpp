#include "Arduino.h"
#include "Wire.h"

#include <chrono>
#include <cstdarg>
#include <mutex>
#include <thread>

#ifdef __ANDROID__
#include <android/log.h>
#endif

TwoWire Wire;
WearSerial Serial;
WearEsp ESP;

namespace {
std::mutex gClockMux;
bool gClockInited = false;
bool gClockPaused = false;
int64_t gPausedAccumMs = 0;
std::chrono::steady_clock::time_point gResumePoint;

int64_t nowSteadyMs(std::chrono::steady_clock::time_point tp) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

uint32_t gSeed = 12345u;
std::mutex gRandMux;

uint32_t nextRand() {
  std::lock_guard<std::mutex> lock(gRandMux);
  gSeed = gSeed * 1664525u + 1013904223u;
  return gSeed >> 1;
}
}  // namespace

uint32_t millis() {
  std::lock_guard<std::mutex> lock(gClockMux);
  auto now = std::chrono::steady_clock::now();
  if (!gClockInited) {
    gClockInited = true;
    gClockPaused = false;
    gPausedAccumMs = 0;
    gResumePoint = now;
    return 0;
  }
  int64_t total = gPausedAccumMs;
  if (!gClockPaused) {
    total += nowSteadyMs(now) - nowSteadyMs(gResumePoint);
  }
  return total < 0 ? 0u : static_cast<uint32_t>(total);
}

void wearClockSetPaused(bool paused) {
  std::lock_guard<std::mutex> lock(gClockMux);
  auto now = std::chrono::steady_clock::now();
  if (!gClockInited) {
    gClockInited = true;
    gPausedAccumMs = 0;
    gResumePoint = now;
    gClockPaused = paused;
    return;
  }
  if (paused && !gClockPaused) {
    gPausedAccumMs += nowSteadyMs(now) - nowSteadyMs(gResumePoint);
    gClockPaused = true;
  } else if (!paused && gClockPaused) {
    gResumePoint = now;
    gClockPaused = false;
  }
}

void wearClockReset() {
  std::lock_guard<std::mutex> lock(gClockMux);
  gClockInited = true;
  gClockPaused = false;
  gPausedAccumMs = 0;
  gResumePoint = std::chrono::steady_clock::now();
}

void delay(uint32_t ms) {
  if (ms <= 2) return;
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void randomSeed(unsigned long seed) {
  std::lock_guard<std::mutex> lock(gRandMux);
  gSeed = seed ? static_cast<uint32_t>(seed) : 1u;
}

long random(long howbig) {
  if (howbig <= 0) return 0;
  return static_cast<long>(nextRand() % static_cast<uint32_t>(howbig));
}

long random(long howsmall, long howbig) {
  if (howsmall >= howbig) return howsmall;
  return howsmall + random(howbig - howsmall);
}

int WearSerial::printf(const char *fmt, ...) {
  char buf[512];
  va_list ap;
  va_start(ap, fmt);
  int n = std::vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_INFO, "TamaPoke", "%s", buf);
#else
  std::fputs(buf, stdout);
#endif
  return n;
}

void WearSerial::print(const char *s) {
  if (!s) return;
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_INFO, "TamaPoke", "%s", s);
#else
  std::fputs(s, stdout);
#endif
}

void WearSerial::println(const char *s) {
  if (!s) {
    println();
    return;
  }
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_INFO, "TamaPoke", "%s", s);
#else
  std::printf("%s\n", s);
#endif
}

void WearSerial::println() {
#ifndef __ANDROID__
  std::putchar('\n');
#endif
}

uint32_t WearEsp::getFreeHeap() const { return 1024u * 1024u; }
uint32_t WearEsp::getMinFreeHeap() const { return 1024u * 1024u; }
