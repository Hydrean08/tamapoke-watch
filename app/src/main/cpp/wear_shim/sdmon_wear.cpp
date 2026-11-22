#include "sdmon.h"
#include "sdmon_asset_wear.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

bool sdReady = false;
bool sdDirty = false;
SdThumbs thumbs;

namespace {
#ifdef __ANDROID__
AAssetManager *gAssetMgr = nullptr;
#endif
std::string gAssetsDir;
}  // namespace

uint8_t *wearReadAssetBlob(const char *relPath, uint32_t minBytes, uint32_t maxBytes, uint32_t *outSize) {
  *outSize = 0;
#ifdef __ANDROID__
  if (gAssetMgr) {
    AAsset *asset = AAssetManager_open(gAssetMgr, relPath, AASSET_MODE_BUFFER);
    if (!asset) return nullptr;
    off_t len = AAsset_getLength(asset);
    if (len < static_cast<off_t>(minBytes) || len > static_cast<off_t>(maxBytes)) {
      AAsset_close(asset);
      return nullptr;
    }
    auto *buf = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(len)));
    if (!buf || AAsset_read(asset, buf, static_cast<size_t>(len)) != len) {
      std::free(buf);
      AAsset_close(asset);
      return nullptr;
    }
    AAsset_close(asset);
    *outSize = static_cast<uint32_t>(len);
    return buf;
  }
#endif
  if (gAssetsDir.empty()) return nullptr;
  FILE *f = std::fopen((gAssetsDir + "/" + relPath).c_str(), "rb");
  if (!f) return nullptr;
  if (std::fseek(f, 0, SEEK_END) != 0) { std::fclose(f); return nullptr; }
  long len = std::ftell(f);
  std::rewind(f);
  if (len < static_cast<long>(minBytes) || len > static_cast<long>(maxBytes)) {
    std::fclose(f);
    return nullptr;
  }
  auto *buf = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(len)));
  if (!buf || std::fread(buf, 1, static_cast<size_t>(len), f) != static_cast<size_t>(len)) {
    std::free(buf);
    std::fclose(f);
    return nullptr;
  }
  std::fclose(f);
  *outSize = static_cast<uint32_t>(len);
  return buf;
}

#ifdef __ANDROID__
void wearSetAssetManager(AAssetManager *mgr) { gAssetMgr = mgr; }
#endif
void wearSetAssetsDir(const char *dirPath) { gAssetsDir = dirPath ? dirPath : ""; }

bool sdBegin() {
#ifdef __ANDROID__
  sdReady = (gAssetMgr != nullptr) || !gAssetsDir.empty();
#else
  sdReady = !gAssetsDir.empty();
#endif
  return sdReady;
}

bool sdSerialCommand(const String &) { return false; }

bool PmdMon::load(uint8_t dexNum, bool shiny) {
  unload();
  if (!sdReady) return false;
  char path[32];
  uint32_t size = 0;
  std::snprintf(path, sizeof(path), "mons/p%s%03u.bin", shiny ? "s" : "", dexNum);
  blob = wearReadAssetBlob(path, 7, 3u * 1024u * 1024u, &size);
  if (!blob && shiny) {
    std::snprintf(path, sizeof(path), "mons/p%03u.bin", dexNum);
    blob = wearReadAssetBlob(path, 7, 3u * 1024u * 1024u, &size);
  }
  if (!blob || std::memcmp(blob, "TPK2", 4) != 0) { unload(); return false; }

  uint8_t nActs = blob[4];
  std::memcpy(&palCount, blob + 5, 2);
  if (palCount > 256 || 7u + palCount * 2u > size) { unload(); return false; }
  std::memcpy(pal, blob + 7, palCount * 2);

  const uint8_t *p = blob + 7 + palCount * 2;
  const uint8_t *end = blob + size;
  for (uint8_t i = 0; i < nActs && p + 4 <= end; i++) {
    uint8_t id = p[0], aw = p[1], ah = p[2], nf = p[3];
    p += 4;
    if (id >= PMD_NACTS || nf > 24) { unload(); return false; }
    uint32_t bytes = static_cast<uint32_t>(nf) * 2u + static_cast<uint32_t>(aw) * ah * nf;
    if (aw == 0 || ah == 0 || nf == 0 || p + bytes > end) { unload(); return false; }
    PmdAct &a = acts[id];
    a.w = aw; a.h = ah; a.frames = nf;
    for (uint8_t k = 0; k < nf; k++) {
      a.ms[k] = static_cast<uint16_t>(p[0] | (p[1] << 8));
      if (a.ms[k] == 0) a.ms[k] = 100;
      p += 2;
    }
    a.data = p;
    p += static_cast<uint32_t>(aw) * ah * nf;
    uint8_t base = 1;
    for (uint8_t f = 0; f < nf; f++) {
      const uint8_t *fr = a.data + static_cast<uint32_t>(f) * aw * ah;
      for (int r = ah - 1; r >= 0; r--) {
        bool any = false;
        for (int c = 0; c < aw && !any; c++) {
          if (fr[r * aw + c] != 0xFF) any = true;
        }
        if (any) { if (r + 1 > base) base = static_cast<uint8_t>(r + 1); break; }
      }
    }
    a.base = base;
  }
  loaded = true;
  return true;
}

void PmdMon::unload() {
  if (blob) { std::free(blob); blob = nullptr; }
  for (auto &a : acts) { a.w = a.h = a.frames = a.base = 0; a.data = nullptr; }
  loaded = false;
}
