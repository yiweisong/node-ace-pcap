#ifndef ETHERNET_CAPTURE_H
#define ETHERNET_CAPTURE_H

#include <uv.h>
#include <napi.h>
#include <string.h>

class LiveDeviceCapture : public Napi::ObjectWrap<LiveDeviceCapture>{
    public:
        static void Init(Napi::Env env, Napi::Object exports);
        LiveDeviceCapture(const Napi::CallbackInfo&);

        void Start(const Napi::CallbackInfo&);
        void Stop(const Napi::CallbackInfo&);
        void SetFilter(const Napi::CallbackInfo&);
        void SendPacket(const Napi::CallbackInfo&);
    private:
        std::string _network;
        uv_async_t async;
        pcap_t *pcap_handle;
};

#endif