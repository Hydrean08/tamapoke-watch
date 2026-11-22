#pragma once

#include <cstdint>

void wearCoreInit(const char *saveDir, const char *assetsDir);
bool wearCoreStep();
void wearCoreTouch(int action, int16_t x, int16_t y);
bool wearCoreBackPressed();
void wearCoreRotary(int delta);
void wearCorePause();
void wearCoreResume();
const uint16_t *wearCoreFramebuffer();
int wearCoreWidth();
int wearCoreHeight();
