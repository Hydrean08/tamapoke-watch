#include "audio.h"
#include "Preferences.h"

#include <atomic>
#include <vector>

namespace {
constexpr int SAMPLE_RATE = 16000;
constexpr int RAMP_ATTACK = 64;
constexpr int RAMP_DECAY = 96;

struct Note {
  uint16_t f;
  uint16_t ms;
};

constexpr Note N_TAP[] = {{880, 35}};
constexpr Note N_EAT[] = {{660, 45}, {0, 12}, {660, 45}};
constexpr Note N_PLAY[] = {{784, 45}, {988, 60}};
constexpr Note N_HEART[] = {{1047, 55}, {1319, 90}};
constexpr Note N_HATCH[] = {{523, 80}, {659, 80}, {784, 110}, {1047, 170}};
constexpr Note N_EVOLVE[] = {{523, 80}, {659, 80}, {784, 80}, {1047, 90}, {1319, 230}};
constexpr Note N_MEDAL[] = {{784, 70}, {0, 25}, {784, 70}, {0, 25}, {1047, 200}};
constexpr Note N_DENY[] = {{300, 110}, {200, 170}};
constexpr Note N_BYE[] = {{784, 150}, {659, 150}, {523, 280}};
constexpr Note N_LEVEL[] = {{784, 70}, {1047, 130}};

struct SfxDef {
  const Note *n;
  uint8_t len;
};

constexpr SfxDef SFX[SFX_COUNT] = {
    {N_TAP, 1}, {N_EAT, 3}, {N_PLAY, 2}, {N_HEART, 2}, {N_HATCH, 4},
    {N_EVOLVE, 5}, {N_MEDAL, 5}, {N_DENY, 2}, {N_BYE, 3}, {N_LEVEL, 2},
};

std::atomic<bool> gReady{false};
std::atomic<bool> gOn{true};
std::atomic<bool> gSleeping{false};
std::atomic<uint8_t> gVol{0};
void (*gAudioOutFn)(const int16_t *samples, int count) = nullptr;

void appendTone(std::vector<int16_t> &pcm, uint16_t f, uint16_t ms, int16_t amp) {
  int total = SAMPLE_RATE * ms / 1000;
  int half = f ? (SAMPLE_RATE / (2 * f)) : 0;
  int phase = 0;
  bool high = true;
  for (int i = 0; i < total; i++) {
    int16_t s = 0;
    if (f) {
      s = high ? amp : static_cast<int16_t>(-amp);
      if (i < RAMP_ATTACK) s = static_cast<int16_t>(s * i / RAMP_ATTACK);
      else if (i > total - RAMP_DECAY) s = static_cast<int16_t>(s * (total - i) / RAMP_DECAY);
      if (++phase >= half) {
        phase = 0;
        high = !high;
      }
    }
    pcm.push_back(s);
  }
}
}  // namespace

void wearSetAudioCallback(void (*fn)(const int16_t *samples, int count)) { gAudioOutFn = fn; }

void audioBegin() {
  Preferences p;
  p.begin("tamapoke", true);
  gOn.store(p.getBool("snd", true), std::memory_order_relaxed);
  gVol.store(p.getUChar("vol", 0) % VOL_LEVELS, std::memory_order_relaxed);
  p.end();
  gReady.store(true, std::memory_order_relaxed);
  sfxPlay(SFX_HATCH);
}

void sfxPlay(uint8_t id) {
  if (!gReady.load(std::memory_order_relaxed) || !gOn.load(std::memory_order_relaxed) ||
      gSleeping.load(std::memory_order_relaxed) || id >= SFX_COUNT || !gAudioOutFn) {
    return;
  }
  const SfxDef &d = SFX[id];
  int16_t amp = sfxAmp(gVol.load(std::memory_order_relaxed));
  std::vector<int16_t> pcm;
  pcm.reserve(SAMPLE_RATE / 2);
  for (uint8_t i = 0; i < d.len; i++) {
    appendTone(pcm, d.n[i].f, d.n[i].ms, amp);
  }
  if (!pcm.empty()) {
    gAudioOutFn(pcm.data(), static_cast<int>(pcm.size()));
  }
}

void audioSetEnabled(bool on) {
  gOn.store(on, std::memory_order_relaxed);
  Preferences p;
  p.begin("tamapoke", false);
  p.putBool("snd", on);
  p.end();
}

bool audioEnabled() { return gOn.load(std::memory_order_relaxed); }

void audioSetVolume(uint8_t level) {
  uint8_t clamped = level % VOL_LEVELS;
  gVol.store(clamped, std::memory_order_relaxed);
  Preferences p;
  p.begin("tamapoke", false);
  p.putUChar("vol", clamped);
  p.end();
}

uint8_t audioVolume() { return gVol.load(std::memory_order_relaxed); }
void audioSetSleeping(bool sleeping) { gSleeping.store(sleeping, std::memory_order_relaxed); }
bool audioBusy() { return false; }
void audioSetIdle(bool) {}
