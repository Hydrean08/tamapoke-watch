CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -DBOARD_WEAR_OS=1 -DBOARD_AMOLED_175C=0

SHIM := ../app/src/main/cpp/wear_shim
CORE := ../app/src/main/cpp/tamapoke

SRCS := test_wear_shim.cpp \
        $(SHIM)/arduino_wear.cpp \
        $(SHIM)/prefs_store_wear.cpp \
        $(SHIM)/preferences_wear.cpp \
        $(SHIM)/gfx_wear.cpp \
        $(SHIM)/gfx_geom_wear.cpp \
        $(SHIM)/gfx_text_wear.cpp \
        $(SHIM)/touch_wear.cpp \
        $(SHIM)/sdmon_wear.cpp \
        $(SHIM)/sdmon_thumbs_wear.cpp \
        $(SHIM)/rtcbat_wear.cpp \
        $(SHIM)/audio_wear.cpp \
        $(SHIM)/wear_bridge.cpp \
        $(CORE)/pet.cpp \
        $(CORE)/i18n.cpp

all: run

test_wear_shim: $(SRCS)
	$(CXX) $(CXXFLAGS) -I $(SHIM) -I $(CORE) -o $@ $(SRCS)

run: test_wear_shim
	./test_wear_shim

clean:
	rm -f test_wear_shim
