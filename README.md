# TamaPoke for Wear OS

Standalone Wear OS port of **[socquique/TamaPoke](https://github.com/socquique/TamaPoke)** — raise, evolve, and train all 151 Gen-1 Pokémon (plus shinies) on a round Wear OS smartwatch.

Runs the original C++17 game engine, `466×466` pixel-art UI, all 151 + shiny animated sprites from [PMD SpriteCollab](https://github.com/PMDCollab/SpriteCollab), 9 languages, and 16 kHz square-wave sound synth natively via Android NDK.

- **Tested on:** Samsung Galaxy Watch 7 (44mm, Wear OS 6 / Android 16)
- **Compatibility:** Wear OS 3.0+ (`minSdk 30`), round displays, `armeabi-v7a` / `arm64-v8a` / `x86_64`

---

## Install on Your Watch (No Coding Required)

Download the latest `.apk` from the **[Releases](../../releases)** page, then install it using either your Android phone or a PC.

### Option A: From Your Android Phone (Easiest)

1. Download the APK (`TamaPoke-WearOS-*.apk`) onto your phone.
2. On your watch, enable **Developer options**:
   - **Settings → About watch → Software information** → tap **Software version** 7 times until Developer mode turns on.
3. On your watch, enable **ADB debugging** and **Wireless debugging**:
   - **Settings → Developer options → ADB debugging** (ON)
   - **Settings → Developer options → Wireless debugging** (ON, make sure watch is on the same Wi-Fi as your phone)
4. Use a Wear OS sideloading app on your phone (such as **Wear Installer 2** or **Bugjaeger**) to pair with your watch's Wireless debugging IP/port/code and send the APK.
5. Turn off **Wireless debugging** afterward to save battery.

### Option B: From a PC / Mac / Linux via ADB

1. Install [Android Platform Tools (`adb`)](https://developer.android.com/tools/releases/platform-tools).
2. Enable **Developer options → ADB debugging → Wireless debugging** on your watch (on the same Wi-Fi network as your computer).
3. Tap **Pair new device** on the watch, then run:
   ```bash
   adb pair <watch-ip>:<pairing-port> <6-digit-code>
   adb connect <watch-ip>:<wireless-debugging-port>
   adb install -r TamaPoke-WearOS-*.apk
   ```
   *(Use the port shown on the main **Wireless debugging** screen for `adb connect`, which differs from the pairing port.)*
4. Turn off **Wireless debugging** on the watch afterward to save battery.

---

## Controls

| Input | Action |
|---|---|
| **Tap** | Hatch egg, pet creature, press bottom icons (Feed / Play / Sleep Light / Bath), navigate menus |
| **Swipe left / right** | Open Pokédex gallery; flip pages in gallery & stat card |
| **Swipe up** | Open Stat Card (Profile / Battle / Medals / Progress; tap name on Profile to rename) |
| **Swipe down** | Open Settings (Clock, Sound volume/mute, Brightness, 9 UI languages) |
| **Long press (3s)** on creature | Release dialog |
| **Watch Back button** | Close active overlay/minigame/gallery, or save & exit from main screen |
| **Touch / rotary bezel** | Flip pages in Pokédex gallery and Stat Card |

> **Battery & offline progress:** Closing the app or letting the watch screen sleep pauses active decay and saves your timestamp. When you reopen TamaPoke, `syncClock()` catches up age, energy recovery during sleep, and gentle offline decay (up to 2 weeks, floored so your Pokémon won't run away while the app is closed).

---

## Build from Source

Requires JDK 17 and Android SDK (`platforms;android-35`, `ndk;27.1.12297006`, `cmake;3.22.1`):

```bash
./gradlew assembleDebug
# APK at app/build/outputs/apk/debug/app-debug.apk
```

Host shim tests (runs on Linux/macOS with `g++`):

```bash
make -C tests_host
```

---

## Credits & License

- **Original project:** [socquique/TamaPoke](https://github.com/socquique/TamaPoke)
- **Sprites:** [PMDCollab/SpriteCollab](https://github.com/PMDCollab/SpriteCollab) (CC BY-NC 4.0)
- **Stats & localized names:** [PokéAPI](https://pokeapi.co)
- **Font table:** Classic 5×7 CP437 font from [moononournation/Arduino_GFX](https://github.com/moononournation/Arduino_GFX)

Source code is licensed under **[CC BY-NC-SA 4.0](LICENSE)** (personal, non-commercial use only; forks must keep the same license). Pokémon names and sprites are © Nintendo / Game Freak / The Pokémon Company. This is an unofficial non-commercial fan project.
