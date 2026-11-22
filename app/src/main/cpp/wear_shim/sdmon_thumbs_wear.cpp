#include "sdmon.h"
#include "sdmon_asset_wear.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

void SdThumbs::unload() {
  if (data) { std::free(data); data = nullptr; }
  loaded = false;
  count = 0;
  size = 0;
}

bool SdThumbs::load() {
  unload();
  if (!sdReady) return false;
  uint32_t sz = 0;
  data = wearReadAssetBlob("mons/thumbs.bin", 10, 2u * 1024u * 1024u, &sz);
  if (!data || std::memcmp(data, "TPTH", 4) != 0) { unload(); return false; }
  std::memcpy(&count, data + 4, 2);
  if (6u + 4u * count > sz) { unload(); return false; }
  size = sz;
  loaded = true;
  return true;
}

const uint8_t *SdThumbs::get(int16_t dex) const {
  if (!loaded || dex < 1 || dex > count) return nullptr;
  uint32_t off = 0;
  std::memcpy(&off, data + 6 + 4 * (dex - 1), 4);
  if (off > size - 3) return nullptr;
  uint32_t need = 3u + static_cast<uint32_t>(data[off + 2]) * 2u +
                  static_cast<uint32_t>(data[off]) * data[off + 1];
  if (need > size || off > size - need) return nullptr;
  return data + off;
}

bool SdMon::load(uint8_t dexNum, bool shiny) {
  unload();
  if (!sdReady) return false;
  char path[32];
  uint32_t sz = 0;
  std::snprintf(path, sizeof(path), "mons/%s%03u.bin", shiny ? "s" : "", dexNum);
  uint8_t *raw = wearReadAssetBlob(path, 14, 1024u * 1024u, &sz);
  if (!raw && shiny) {
    std::snprintf(path, sizeof(path), "mons/%03u.bin", dexNum);
    raw = wearReadAssetBlob(path, 14, 1024u * 1024u, &sz);
  }
  if (!raw || std::memcmp(raw, "TPK1", 4) != 0) { std::free(raw); return false; }
  uint16_t header[4];
  std::memcpy(header, raw + 4, 8);
  w = header[0];
  h = header[1];
  frames = header[2];
  frameMs = header[3];
  std::memcpy(&palCount, raw + 12, 2);
  uint32_t pixSize = static_cast<uint32_t>(w) * h * frames;
  uint32_t palBytes = static_cast<uint32_t>(palCount) * 2u;
  if (palCount > 256 || w == 0 || w > 256 || h == 0 || h > 256 ||
      frames == 0 || frames > 64 || 14u + palBytes + pixSize > sz) {
    std::free(raw);
    return false;
  }
  std::memcpy(pal, raw + 14, palBytes);
  data = static_cast<uint8_t *>(std::malloc(pixSize));
  if (!data) { std::free(raw); return false; }
  std::memcpy(data, raw + 14 + palBytes, pixSize);
  std::free(raw);
  scale = static_cast<uint8_t>(200 / h);
  if (scale < 2) scale = 2;
  if (scale > 5) scale = 5;
  loaded = true;
  return true;
}

void SdMon::unload() {
  if (data) { std::free(data); data = nullptr; }
  loaded = false;
}
