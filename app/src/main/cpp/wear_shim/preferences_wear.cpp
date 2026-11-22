#include "Preferences.h"
#include "prefs_store_wear.h"

#include <cstring>

namespace {
using namespace wear_prefs;

size_t rawPut(const std::string &ns, bool open, bool ro, const char *k, const void *v, size_t len) {
  if (!open || ro || !k) return 0;
  std::lock_guard<std::mutex> lock(gMux);
  loadLocked();
  const auto *p = static_cast<const uint8_t *>(v);
  gStore[ns][k].assign(p, p + len);
  gDirty = true;
  return len;
}

template <class T>
size_t putVal(const std::string &ns, bool open, bool ro, const char *k, T v) {
  return rawPut(ns, open, ro, k, &v, sizeof(T));
}

template <class T>
T getVal(const std::string &ns, bool open, const char *k, T def) {
  if (!open || !k) return def;
  std::lock_guard<std::mutex> lock(gMux);
  loadLocked();
  auto nsIt = gStore.find(ns);
  if (nsIt == gStore.end()) return def;
  auto kvIt = nsIt->second.find(k);
  if (kvIt == nsIt->second.end() || kvIt->second.size() != sizeof(T)) return def;
  T out;
  std::memcpy(&out, kvIt->second.data(), sizeof(T));
  return out;
}
}  // namespace

bool Preferences::begin(const char *name, bool readOnly) {
  if (!name) return false;
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::loadLocked();
  ns_ = name;
  ro_ = readOnly;
  open_ = true;
  return true;
}

void Preferences::end() {
  if (open_ && !ro_) wearPrefsFlush();
  open_ = false;
}

bool Preferences::clear() {
  if (!open_ || ro_) return false;
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::gStore[ns_].clear();
  wear_prefs::gDirty = true;
  wear_prefs::flushLocked();
  return true;
}

bool Preferences::isKey(const char *key) {
  if (!open_ || !key) return false;
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::loadLocked();
  auto nsIt = wear_prefs::gStore.find(ns_);
  return nsIt != wear_prefs::gStore.end() && nsIt->second.find(key) != nsIt->second.end();
}

bool Preferences::remove(const char *key) {
  if (!open_ || ro_ || !key) return false;
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  auto nsIt = wear_prefs::gStore.find(ns_);
  if (nsIt == wear_prefs::gStore.end()) return false;
  bool erased = nsIt->second.erase(key) > 0;
  if (erased) wear_prefs::gDirty = true;
  return erased;
}

size_t Preferences::putBool(const char *k, bool v) { return putVal<uint8_t>(ns_, open_, ro_, k, v ? 1 : 0); }
size_t Preferences::putUChar(const char *k, uint8_t v) { return putVal(ns_, open_, ro_, k, v); }
size_t Preferences::putChar(const char *k, int8_t v) { return putVal(ns_, open_, ro_, k, v); }
size_t Preferences::putShort(const char *k, int16_t v) { return putVal(ns_, open_, ro_, k, v); }
size_t Preferences::putUShort(const char *k, uint16_t v) { return putVal(ns_, open_, ro_, k, v); }
size_t Preferences::putInt(const char *k, int32_t v) { return putVal(ns_, open_, ro_, k, v); }
size_t Preferences::putUInt(const char *k, uint32_t v) { return putVal(ns_, open_, ro_, k, v); }
size_t Preferences::putBytes(const char *k, const void *v, size_t len) { return rawPut(ns_, open_, ro_, k, v, len); }
size_t Preferences::putString(const char *k, const char *v) {
  const char *s = v ? v : "";
  return rawPut(ns_, open_, ro_, k, s, std::strlen(s) + 1);
}

bool Preferences::getBool(const char *k, bool def) { return getVal<uint8_t>(ns_, open_, k, def ? 1 : 0) != 0; }
uint8_t Preferences::getUChar(const char *k, uint8_t def) { return getVal(ns_, open_, k, def); }
int8_t Preferences::getChar(const char *k, int8_t def) { return getVal(ns_, open_, k, def); }
int16_t Preferences::getShort(const char *k, int16_t def) { return getVal(ns_, open_, k, def); }
uint16_t Preferences::getUShort(const char *k, uint16_t def) { return getVal(ns_, open_, k, def); }
int32_t Preferences::getInt(const char *k, int32_t def) { return getVal(ns_, open_, k, def); }
uint32_t Preferences::getUInt(const char *k, uint32_t def) { return getVal(ns_, open_, k, def); }

size_t Preferences::getBytes(const char *k, void *buf, size_t maxLen) {
  if (!open_ || !k || !buf || maxLen == 0) return 0;
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::loadLocked();
  auto nsIt = wear_prefs::gStore.find(ns_);
  if (nsIt == wear_prefs::gStore.end()) return 0;
  auto kvIt = nsIt->second.find(k);
  if (kvIt == nsIt->second.end()) return 0;
  size_t n = kvIt->second.size() < maxLen ? kvIt->second.size() : maxLen;
  std::memcpy(buf, kvIt->second.data(), n);
  return n;
}

size_t Preferences::getString(const char *k, char *buf, size_t maxLen) {
  if (!open_ || !k || !buf || maxLen == 0) return 0;
  std::lock_guard<std::mutex> lock(wear_prefs::gMux);
  wear_prefs::loadLocked();
  auto nsIt = wear_prefs::gStore.find(ns_);
  if (nsIt == wear_prefs::gStore.end()) { buf[0] = 0; return 0; }
  auto kvIt = nsIt->second.find(k);
  if (kvIt == nsIt->second.end()) { buf[0] = 0; return 0; }
  size_t n = kvIt->second.size() < maxLen ? kvIt->second.size() : maxLen - 1;
  std::memcpy(buf, kvIt->second.data(), n);
  buf[n < maxLen ? n : maxLen - 1] = 0;
  return std::strlen(buf);
}
