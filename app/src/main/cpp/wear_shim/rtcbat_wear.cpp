#include "rtcbat.h"

#include <atomic>
#include <ctime>

namespace {
std::atomic<int> gBatPercent{100};
std::atomic<bool> gBatCharging{false};
std::atomic<int64_t> gManualOffsetSec{0};

uint32_t currentLocalSystemEpoch() {
  std::time_t now = std::time(nullptr);
  if (now <= 0) return 0;
  std::tm localTm{};
  ::tzset();
  if (::localtime_r(&now, &localTm) == nullptr) {
    return static_cast<uint32_t>(now);
  }
  int64_t localSec = static_cast<int64_t>(now) + static_cast<int64_t>(localTm.tm_gmtoff);
  return localSec > 0 ? static_cast<uint32_t>(localSec) : 0u;
}
}  // namespace

void wearSetBatteryState(int percent, bool charging) {
  if (percent > 100) percent = 100;
  gBatPercent.store(percent, std::memory_order_relaxed);
  gBatCharging.store(charging, std::memory_order_relaxed);
}

bool rtcBegin() { return true; }

uint32_t rtcEpoch() {
  uint32_t base = currentLocalSystemEpoch();
  if (base == 0) return 0;
  int64_t shifted = static_cast<int64_t>(base) + gManualOffsetSec.load(std::memory_order_relaxed);
  return shifted > 0 ? static_cast<uint32_t>(shifted) : base;
}

void rtcSetEpoch(uint32_t e) {
  uint32_t base = currentLocalSystemEpoch();
  if (base == 0 || e == 0) return;
  gManualOffsetSec.store(static_cast<int64_t>(e) - static_cast<int64_t>(base),
                         std::memory_order_relaxed);
}

bool batBegin() { return true; }
void pmuEnablePanel() {}
void batSetPollMs(uint32_t) {}
int batPercent() { return gBatPercent.load(std::memory_order_relaxed); }
bool batCharging() { return gBatCharging.load(std::memory_order_relaxed); }
int batMillivolts() { return 4000; }
bool batShouldShutdown() { return false; }
void batPowerOff() {}
bool usbPresent() { return gBatCharging.load(std::memory_order_relaxed); }
bool usbPresentNow() { return gBatCharging.load(std::memory_order_relaxed); }
void pwrSetup() {}
bool pwrShortPressed() { return false; }
