#pragma once

#include <cstdint>

uint8_t *wearReadAssetBlob(const char *relPath, uint32_t minBytes, uint32_t maxBytes, uint32_t *outSize);
