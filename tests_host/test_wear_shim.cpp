#include "wear_bridge.h"
#include "Arduino.h"
#include "Preferences.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

extern void wearSetAssetsDir(const char *);

namespace {
bool hasNonBlackCenter(const uint16_t *fb, int w, int h) {
  int count = 0;
  for (int y = h / 4; y < (h * 3) / 4; y++) {
    for (int x = w / 4; x < (w * 3) / 4; x++) {
      if (fb[y * w + x] != 0) count++;
    }
  }
  return count > 1000;
}

void writePpm(const char *path, const uint16_t *fb, int w, int h) {
  FILE *f = std::fopen(path, "wb");
  if (!f) return;
  std::fprintf(f, "P6\n%d %d\n255\n", w, h);
  for (int i = 0; i < w * h; i++) {
    uint16_t c = fb[i];
    uint8_t rgb[3] = {
        static_cast<uint8_t>(((c >> 11) & 31) * 255 / 31),
        static_cast<uint8_t>(((c >> 5) & 63) * 255 / 63),
        static_cast<uint8_t>((c & 31) * 255 / 31),
    };
    std::fwrite(rgb, 1, 3, f);
  }
  std::fclose(f);
}

void stepMs(int totalMs, int step = 25) {
  for (int t = 0; t < totalMs; t += step) {
    delay(static_cast<uint32_t>(step));
    wearCoreStep();
  }
}
}  // namespace

int main() {
  char tmpTemplate[] = "./tamapoke_wear_test_XXXXXX";
  char *tmpDir = ::mkdtemp(tmpTemplate);
  assert(tmpDir != nullptr);

  wearCoreInit(tmpDir, "../app/src/main/assets");
  assert(wearCoreWidth() == 466 && wearCoreHeight() == 466);
  assert(wearCoreStep());
  const uint16_t *fb = wearCoreFramebuffer();
  assert(fb != nullptr && hasNonBlackCenter(fb, 466, 466));
  writePpm((std::string(tmpDir) + "/starter.ppm").c_str(), fb, 466, 466);

  // Tap Charmander (second starter row: y around 110 + 78 + 35 = 223, x = 233)
  wearCoreTouch(0, 233, 223);
  stepMs(30);
  wearCoreTouch(1, 233, 223);
  stepMs(150);

  // Tap egg 3 times to hatch Charmander
  for (int i = 0; i < 3; i++) {
    wearCoreTouch(0, 233, 202);
    stepMs(30);
    wearCoreTouch(1, 233, 202);
    stepMs(150);
  }
  writePpm((std::string(tmpDir) + "/hatched.ppm").c_str(), fb, 466, 466);

  // Fast swipe right to open Gallery, then Back button closes Gallery
  wearCoreTouch(0, 80, 233);
  wearCoreTouch(2, 220, 233);
  wearCoreTouch(1, 260, 233);
  stepMs(150);
  assert(wearCoreBackPressed() == true);   // closed Gallery
  assert(wearCoreBackPressed() == false);  // back on main screen -> save & allow exit

  // Verify Preferences persisted Charmander speciesId=4 to disk
  wearPrefsResetMemory();
  wearPrefsSetDir(tmpDir);
  Preferences p;
  assert(p.begin("tamapoke", true));
  assert(p.getShort("dexn", -1) == 4);
  p.end();

  std::remove((std::string(tmpDir) + "/starter.ppm").c_str());
  std::remove((std::string(tmpDir) + "/hatched.ppm").c_str());
  std::remove((std::string(tmpDir) + "/tamapoke_nvs.bin").c_str());
  ::rmdir(tmpDir);
  std::printf("wear_shim host tests PASSED\n");
  return 0;
}
