#include "Arduino_GFX_Library.h"
#include "wear_bridge.h"

#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <cstring>
#include <jni.h>

extern void wearSetAssetManager(AAssetManager *mgr);
extern void wearSetAudioCallback(void (*fn)(const int16_t *samples, int count));
extern void wearSetBatteryState(int percent, bool charging);

namespace {
JavaVM *gVm = nullptr;
jobject gActivity = nullptr;
jmethodID gPlayAudioMid = nullptr;
jmethodID gRasterTextMid = nullptr;

JNIEnv *getEnv() {
  if (!gVm) return nullptr;
  JNIEnv *env = nullptr;
  if (gVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) return nullptr;
  return env;
}

void onAudioPcm(const int16_t *samples, int count) {
  JNIEnv *env = getEnv();
  if (!env || !gActivity || !gPlayAudioMid || count <= 0) return;
  jshortArray arr = env->NewShortArray(count);
  if (!arr) return;
  env->SetShortArrayRegion(arr, 0, count, samples);
  env->CallVoidMethod(gActivity, gPlayAudioMid, arr);
  env->DeleteLocalRef(arr);
}

void onUtf8Measure(const char *utf8, uint8_t size, int16_t *y1, uint16_t *w, uint16_t *h) {
  JNIEnv *env = getEnv();
  if (!env || !gActivity || !gRasterTextMid || !utf8) return;
  jstring js = env->NewStringUTF(utf8);
  if (!js) return;
  auto arr = static_cast<jintArray>(env->CallObjectMethod(gActivity, gRasterTextMid, js, static_cast<jint>(size), JNI_FALSE));
  env->DeleteLocalRef(js);
  if (!arr) return;
  if (env->GetArrayLength(arr) >= 3) {
    jint hdr[3] = {0, 0, 0};
    env->GetIntArrayRegion(arr, 0, 3, hdr);
    *w = static_cast<uint16_t>(hdr[0] > 0 ? hdr[0] : 0);
    *h = static_cast<uint16_t>(hdr[1] > 0 ? hdr[1] : 0);
    *y1 = static_cast<int16_t>(hdr[2]);
  }
  env->DeleteLocalRef(arr);
}

int16_t onUtf8Draw(const char *utf8, int16_t x, int16_t y, uint8_t size, uint16_t color,
                   uint16_t *buf, int16_t bufW, int16_t bufH) {
  JNIEnv *env = getEnv();
  if (!env || !gActivity || !gRasterTextMid || !utf8) return x;
  jstring js = env->NewStringUTF(utf8);
  if (!js) return x;
  auto arr = static_cast<jintArray>(env->CallObjectMethod(gActivity, gRasterTextMid, js, static_cast<jint>(size), JNI_TRUE));
  env->DeleteLocalRef(js);
  if (!arr) return x;
  jsize len = env->GetArrayLength(arr);
  if (len < 3) { env->DeleteLocalRef(arr); return x; }
  jint *elems = env->GetIntArrayElements(arr, nullptr);
  int tw = elems[0], th = elems[1], y1 = elems[2];
  if (tw > 0 && th > 0 && len >= 3 + tw * th) {
    int yTop = y + y1;
    for (int row = 0; row < th; row++) {
      int py = yTop + row;
      if (py < 0 || py >= bufH) continue;
      for (int col = 0; col < tw; col++) {
        int px = x + col;
        if (px < 0 || px >= bufW) continue;
        if (elems[3 + row * tw + col] >= 96) {
          buf[static_cast<size_t>(py) * bufW + px] = color;
        }
      }
    }
  }
  env->ReleaseIntArrayElements(arr, elems, JNI_ABORT);
  env->DeleteLocalRef(arr);
  return static_cast<int16_t>(x + tw);
}
}  // namespace

extern "C" {

JNIEXPORT void JNICALL
Java_com_tamapoke_watch_MainActivity_nativeInit(JNIEnv *env, jobject thiz, jobject assetMgr, jstring filesDir) {
  env->GetJavaVM(&gVm);
  if (gActivity) env->DeleteGlobalRef(gActivity);
  gActivity = env->NewGlobalRef(thiz);
  jclass cls = env->GetObjectClass(thiz);
  gPlayAudioMid = env->GetMethodID(cls, "playPcmAudio", "([S)V");
  gRasterTextMid = env->GetMethodID(cls, "rasterizeUtf8Text", "(Ljava/lang/String;IZ)[I");
  env->DeleteLocalRef(cls);

  wearSetAssetManager(AAssetManager_fromJava(env, assetMgr));
  wearSetAudioCallback(onAudioPcm);
  WearUtf8TextHooks textHooks{onUtf8Measure, onUtf8Draw};
  wearSetUtf8TextHooks(textHooks);

  const char *dirStr = env->GetStringUTFChars(filesDir, nullptr);
  wearCoreInit(dirStr, nullptr);
  env->ReleaseStringUTFChars(filesDir, dirStr);
}

JNIEXPORT jboolean JNICALL
Java_com_tamapoke_watch_MainActivity_nativeStep(JNIEnv *env, jobject, jobject bitmap) {
  if (!wearCoreStep()) return JNI_FALSE;
  const uint16_t *fb = wearCoreFramebuffer();
  if (!fb) return JNI_FALSE;
  AndroidBitmapInfo info{};
  if (AndroidBitmap_getInfo(env, bitmap, &info) != ANDROID_BITMAP_RESULT_SUCCESS) return JNI_FALSE;
  void *pixels = nullptr;
  if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESULT_SUCCESS || !pixels) return JNI_FALSE;
  const int w = wearCoreWidth(), h = wearCoreHeight();
  const int copyW = static_cast<int>(info.width) < w ? static_cast<int>(info.width) : w;
  const int copyH = static_cast<int>(info.height) < h ? static_cast<int>(info.height) : h;
  auto *dstBytes = static_cast<uint8_t *>(pixels);
  for (int row = 0; row < copyH; row++) {
    std::memcpy(dstBytes + static_cast<size_t>(row) * info.stride,
                fb + static_cast<size_t>(row) * w,
                static_cast<size_t>(copyW) * sizeof(uint16_t));
  }
  AndroidBitmap_unlockPixels(env, bitmap);
  return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_tamapoke_watch_MainActivity_nativeTouch(JNIEnv *, jobject, jint action, jint x, jint y) {
  wearCoreTouch(action, static_cast<int16_t>(x), static_cast<int16_t>(y));
}

JNIEXPORT jboolean JNICALL
Java_com_tamapoke_watch_MainActivity_nativeBackPressed(JNIEnv *, jobject) {
  return wearCoreBackPressed() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_tamapoke_watch_MainActivity_nativeRotary(JNIEnv *, jobject, jint delta) {
  wearCoreRotary(delta);
}

JNIEXPORT void JNICALL
Java_com_tamapoke_watch_MainActivity_nativeOnPause(JNIEnv *, jobject) {
  wearCorePause();
}

JNIEXPORT void JNICALL
Java_com_tamapoke_watch_MainActivity_nativeOnResume(JNIEnv *, jobject) {
  wearCoreResume();
}

JNIEXPORT void JNICALL
Java_com_tamapoke_watch_MainActivity_nativeSetBattery(JNIEnv *, jobject, jint percent, jboolean charging) {
  wearSetBatteryState(percent, charging == JNI_TRUE);
}

}  // extern "C"
