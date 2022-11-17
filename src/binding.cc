#include <napi.h>
#include "live_device_capture.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  LiveDeviceCapture::Init(env, exports);
  return exports;
}

NODE_API_MODULE(pcap, Init);