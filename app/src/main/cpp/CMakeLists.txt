cmake_minimum_required(VERSION 3.22.1)
project(tamapoke_wear LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tamapoke_wear SHARED
    wear_shim/arduino_wear.cpp
    wear_shim/prefs_store_wear.cpp
    wear_shim/preferences_wear.cpp
    wear_shim/gfx_wear.cpp
    wear_shim/gfx_geom_wear.cpp
    wear_shim/gfx_text_wear.cpp
    wear_shim/touch_wear.cpp
    wear_shim/sdmon_wear.cpp
    wear_shim/sdmon_thumbs_wear.cpp
    wear_shim/rtcbat_wear.cpp
    wear_shim/audio_wear.cpp
    wear_shim/wear_bridge.cpp
    wear_shim/jni_bridge.cpp
    tamapoke/pet.cpp
    tamapoke/i18n.cpp
)

target_include_directories(tamapoke_wear PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/wear_shim
    ${CMAKE_CURRENT_SOURCE_DIR}/tamapoke
)

target_compile_definitions(tamapoke_wear PRIVATE
    BOARD_WEAR_OS=1
    BOARD_AMOLED_175C=0
)

target_link_libraries(tamapoke_wear
    android
    jnigraphics
    log
)
