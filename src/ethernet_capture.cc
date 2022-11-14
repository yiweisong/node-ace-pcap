#include "ethernet_capture.h"

using namespace Napi;

void LiveDeviceCapture::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(
        env,
        "LiveDeviceCapture",
        {
            InstanceMethod("start", &MessageParser::Reset),
            InstanceMethod("stop", &MessageParser::Reset),
            InstanceMethod("setFilter", &MessageParser::Reset),
            InstanceMethod("send", &MessageParser::Reset),
        });
    exports.Set("LiveDeviceCapture", func);
}

LiveDeviceCapture::LiveDeviceCapture(const Napi::CallbackInfo &info) : ObjectWrap(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsObject())
    {
        Napi::TypeError::New(env, "Set an options for the parser")
            .ThrowAsJavaScriptException();
        return;
    }

    Napi::Object options = info[0].As<Napi::Object>();
    if (options.Has("network"))
    {
        this->_network = options.Get("network").As<Napi::String>().Utf8Value();
    }
}

void LiveDeviceCapture::Start(const Napi::CallbackInfo &info)
{
    // pcap_open_live
    char errbuf[PCAP_ERRBUF_SIZE];
    this->pcap_handle = pcap_open_live(this->_network.c_str(), // name of the device
                                       65536,                  // portion of the packet to capture.
                                       1,                      // promiscuous mode (nonzero means promiscuous)
                                       1000,                   // read timeout
                                       errbuf                  // error buffer
    );
    if (this->pcap_handle == NULL)
    {
        Napi::TypeError::New(env, errbuf).ThrowAsJavaScriptException();
        return;
    }

    // pcap_dispatch
    // uv_async_init
}

void LiveDeviceCapture::Stop(const Napi::CallbackInfo &info)
{
    pcap_close(this->pcap_handle)
}

void LiveDeviceCapture::SetFilter(const Napi::CallbackInfo &info)
{
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsString())
    {
        Napi::TypeError::New(env, "Filter should be a string value")
            .ThrowAsJavaScriptException();
        return;
    }

    std::string filter = info[0].As<Napi::String>().Utf8Value();
    struct bpf_program fcode;
    bpf_u_int32 NetMask = 0xffffff;

    // compile the filter
    if (pcap_compile(this->pcap_handle, &fcode, filter.c_str(), 1, NetMask) < 0)
    {
        Napi::TypeError::New(env, "Error compiling filter: wrong syntax.")
            .ThrowAsJavaScriptException();
        pcap_freecode(&fcode);
        pcap_close(this->pcap_handle);
        return;
    }

    // set the filter
    if (pcap_setfilter(this->pcap_handle, &fcode) < 0)
    {
        Napi::TypeError::New(env, "Error setting the filter")
            .ThrowAsJavaScriptException();
        pcap_freecode(&fcode);
        pcap_close(this->pcap_handle);
        return;
    }

    pcap_freecode(&fcode);
}

void LiveDeviceCapture::SendPacket(const Napi::CallbackInfo &info)
{
    if (info.Length() != 1) {
        Napi::Error::New(info.Env(), "Expected exactly one argument")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    if (!info[0].IsBuffer()) {
        Napi::Error::New(info.Env(), "Expected an Buffer")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    size_t buffer_size = buf.Length();
    
    // pcap_send_packet
    if (pcap_sendpacket(this->pcap_handle,
                        buf,
                        (int)buffer_size) == -1)
    {
        Napi::Error::New(info.Env(), pcap_geterr(obj->pcap_handle))
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }
}