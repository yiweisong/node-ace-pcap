#include <napi.h>
#include "ethernet_capture.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  LiveDeviceCapture::Init(env, exports);
  return exports;
}

NODE_API_MODULE(pcap, Init);