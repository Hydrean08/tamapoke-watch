#include "wear_bridge.h"

#include "Arduino.h"
#include "Arduino_GFX_Library.h"
#include "Preferences.h"
#include "TouchDrvCSTXXX.hpp"
#include "audio.h"
#include "rtcbat.h"
#include "sdmon.h"
#include "tamapoke_prototypes.h"

#include "TamaPoke.ino"

namespace {
bool gInitialized = false;
bool gFirstResumePending = true;
}  // namespace

void wearCoreInit(const char *saveDir, const char *assetsDir) {
  if (gInitialized) return;
  wearClockReset();
  wearTouchReset();
  wearPrefsSetDir(saveDir);
  if (assetsDir) {
    extern void wearSetAssetsDir(const char *);
    wearSetAssetsDir(assetsDir);
  }
  setup();
  lastRender = millis() - 200u;
  gInitialized = true;
  gFirstResumePending = true;
}

bool wearCoreStep() {
  if (!gInitialized) return false;
  loop();
  wearPrefsFlush();
  return gfx->takeFrameDirty();
}

void wearCoreTouch(int action, int16_t x, int16_t y) {
  if (!gInitialized) return;
  wearTouchPush(action, x, y);
}

bool wearCoreBackPressed() {
  if (!gInitialized) return false;
  bool handled = true;
  if (kbOpen) kbOpen = false;
  else if (clockOpen) clockOpen = false;
  else if (choiceKind) choiceKind = 0;
  else if (confirmUntil) confirmUntil = 0;
  else if (feedMenuUntil) feedMenuUntil = 0;
  else if (galleryDetail) { galleryDetail = 0; galleryPmd.unload(); galleryDirty = true; }
  else if (galleryOpen) { galleryOpen = false; galleryPmd.unload(); }
  else if (cardOpen) cardOpen = false;
  else if (gameOpen) gameOpen = false;
  else if (sackOpen) sackOpen = false;
  else handled = false;
  if (handled) {
    screenKeyValid = false;
    lastRender = millis() - 200u;
    return true;
  }
  pet.setClock(rtcEpoch());
  wearPrefsFlush();
  return false;
}

void wearCoreRotary(int delta) {
  if (!gInitialized || delta == 0) return;
  int dir = delta > 0 ? -1 : 1;
  if (galleryOpen && !galleryDetail) onSwipe(dir);
  else if (cardOpen) onSwipe(dir);
  screenKeyValid = false;
  lastRender = millis() - 200u;
}

void wearCorePause() {
  if (!gInitialized) return;
  wearClockSetPaused(true);
  pet.setClock(rtcEpoch());
  wearPrefsFlush();
}

void wearCoreResume() {
  if (!gInitialized) return;
  wearClockSetPaused(false);
  if (gFirstResumePending) {
    gFirstResumePending = false;
  } else {
    pet.syncClock(rtcEpoch());
  }
  lastInteract = millis();
  dimStage = 0;
  screenOff = false;
  screenKeyValid = false;
  galleryDirty = true;
}

const uint16_t *wearCoreFramebuffer() { return gfx->frontBuffer(); }
int wearCoreWidth() { return gfx->width(); }
int wearCoreHeight() { return gfx->height(); }
