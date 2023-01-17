#ifndef ACE_PCAP_LIVE_DEVICE_CAPTURE_H
#define ACE_PCAP_LIVE_DEVICE_CAPTURE_H

#include <uv.h>
#include <napi.h>
#include <string.h>
#include <pcap.h>

struct PacketEventData
{
    const u_char *pkt_data;
    size_t copy_len;
};

class LiveDeviceCapture : public Napi::ObjectWrap<LiveDeviceCapture>
{
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    static void EmitPacket(u_char *user, const struct pcap_pkthdr *pkt_hdr, const u_char *pkt_data);
#ifdef _WIN32
    static void cb_packets(uv_async_t *handle);
    static void cb_close(uv_handle_t *handle);
    static void CALLBACK OnPacket(void *data, BOOLEAN didTimeout);
#else
    static void cb_packets(uv_poll_t *handle, int status, int events);
#endif
    LiveDeviceCapture(const Napi::CallbackInfo &);
    void Start(const Napi::CallbackInfo &);
    void Stop(const Napi::CallbackInfo &);
    void SetFilter(const Napi::CallbackInfo &);
    void SendPacket(const Napi::CallbackInfo &);

private:
    Napi::ThreadSafeFunction tsEmit_;
    std::string iface;
    std::string filter;
    pcap_t *pcap_handle = NULL;
    bool close();
    void cleanup();
    bool closing;
    bool handling_packets;
    struct bpf_program fcode;
#ifdef _WIN32
    HANDLE wait;
    uv_async_t async;
#else
    uv_poll_t poll_handle;
    int fd;
#endif
};

#endif