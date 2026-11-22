#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace wear_prefs {
using KvMap = std::map<std::string, std::vector<uint8_t>>;
using StoreMap = std::map<std::string, KvMap>;

extern std::mutex gMux;
extern StoreMap gStore;
extern bool gDirty;

void loadLocked();
void flushLocked();
}  // namespace wear_prefs
