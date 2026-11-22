#include "prefs_store_wear.h"
#include "Preferences.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace wear_prefs {
constexpr char NVS_MAGIC[4] = {'T', 'P', 'K', 'V'};
std::mutex gMux;
StoreMap gStore;
std::string gDirPath;
bool gLoaded = false;
bool gDirty = false;

bool readExact(FILE *f, void *dst, size_t n) { return n == 0 || std::fread(dst, 1, n, f) == n; }
bool writeExact(FILE *f, const void *src, size_t n) { return n == 0 || std::fwrite(src, 1, n, f) == n; }

void loadLocked() {
  if (gLoaded) return;
  gLoaded = true;
  if (gDirPath.empty()) return;
  FILE *f = std::fopen((gDirPath + "/tamapoke_nvs.bin").c_str(), "rb");
  if (!f) return;

  char magic[4];
  uint32_t nsCount = 0;
  if (!readExact(f, magic, 4) || std::memcmp(magic, NVS_MAGIC, 4) != 0 ||
      !readExact(f, &nsCount, 4) || nsCount > 64) {
    std::fclose(f);
    return;
  }
  StoreMap parsed;
  for (uint32_t i = 0; i < nsCount; i++) {
    uint16_t nsLen = 0;
    uint32_t kvCount = 0;
    if (!readExact(f, &nsLen, 2) || nsLen == 0 || nsLen > 256) { std::fclose(f); return; }
    std::string ns(nsLen, '\0');
    if (!readExact(f, &ns[0], nsLen) || !readExact(f, &kvCount, 4) || kvCount > 4096) {
      std::fclose(f);
      return;
    }
    KvMap &kv = parsed[ns];
    for (uint32_t j = 0; j < kvCount; j++) {
      uint16_t kLen = 0;
      uint32_t vLen = 0;
      if (!readExact(f, &kLen, 2) || kLen == 0 || kLen > 256) { std::fclose(f); return; }
      std::string key(kLen, '\0');
      if (!readExact(f, &key[0], kLen) || !readExact(f, &vLen, 4) || vLen > 65536) {
        std::fclose(f);
        return;
      }
      std::vector<uint8_t> val(vLen);
      if (!readExact(f, val.data(), vLen)) { std::fclose(f); return; }
      kv[key] = std::move(val);
    }
  }
  std::fclose(f);
  gStore = std::move(parsed);
}

void flushLocked() {
  if (!gDirty || gDirPath.empty()) return;
  std::string path = gDirPath + "/tamapoke_nvs.bin";
  std::string tmp = path + ".tmp";
  FILE *f = std::fopen(tmp.c_str(), "wb");
  if (!f) return;

  uint32_t nsCount = static_cast<uint32_t>(gStore.size());
  bool ok = writeExact(f, NVS_MAGIC, 4) && writeExact(f, &nsCount, 4);
  for (const auto &ns : gStore) {
    if (!ok) break;
    uint16_t nsLen = static_cast<uint16_t>(ns.first.size());
    uint32_t kvCount = static_cast<uint32_t>(ns.second.size());
    ok = writeExact(f, &nsLen, 2) && writeExact(f, ns.first.data(), nsLen) &&
         writeExact(f, &kvCount, 4);
    for (const auto &kv : ns.second) {
      if (!ok) break;
      uint16_t kLen = static_cast<uint16_t>(kv.first.size());
      uint32_t vLen = static_cast<uint32_t>(kv.second.size());
      ok = writeExact(f, &kLen, 2) && writeExact(f, kv.first.data(), kLen) &&
           writeExact(f, &vLen, 4) && writeExact(f, kv.second.data(), vLen);
    }
  }
  if (ok && std::fflush(f) == 0) ::fsync(fileno(f));
  std::fclose(f);
  if (ok && std::rename(tmp.c_str(), path.c_str()) == 0) gDirty = false;
  else std::remove(tmp.c_str());
}
}  // namespace wear_prefs

void wearPrefsSetDir(const char *dirPath) {
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::gDirPath = dirPath ? dirPath : "";
  wear_prefs::gLoaded = false;
  wear_prefs::gStore.clear();
  wear_prefs::gDirty = false;
  wear_prefs::loadLocked();
}

void wearPrefsFlush() {
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::flushLocked();
}

void wearPrefsResetMemory() {
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::gStore.clear();
  wear_prefs::gDirPath.clear();
  wear_prefs::gLoaded = true;
  wear_prefs::gDirty = false;
}
