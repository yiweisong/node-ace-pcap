#include <napi.h>
#include "live_device_capture.h"
#include "utils.h"

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);
  LiveDeviceCapture::Init(env, exports);
  exports.Set("GetNetworkInterface", Napi::Function::New(env, GetNetworkInterface));
  exports.Set("IsDependencyInstalled", Napi::Function::New(env, IsDependencyInstalled));
  return exports;
}

NODE_API_MODULE(pcap, Init);