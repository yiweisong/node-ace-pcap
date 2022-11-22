#include "live_device_capture.h"

using namespace Napi;

Napi::Object LiveDeviceCapture::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(
        env,
        "LiveDeviceCapture",
        {
            InstanceMethod("start", &LiveDeviceCapture::Start),
            InstanceMethod("stop", &LiveDeviceCapture::Stop),
            InstanceMethod("setFilter", &LiveDeviceCapture::SetFilter),
            InstanceMethod("send", &LiveDeviceCapture::SendPacket),
        });
    exports.Set("LiveDeviceCapture", func);
    return exports;
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
    if (options.Has("iface"))
    {
        this->iface = options.Get("iface").As<Napi::String>().Utf8Value();
    }

    if (options.Has("filter"))
    {
        this->filter = options.Get("filter").As<Napi::String>().Utf8Value();
    }
    else
    {
        this->filter = "";
    }
#ifdef _WIN32
    this->wait = nullptr;
#endif
    this->handling_packets = false;
    this->closing = false;
}

#ifdef _WIN32
void LiveDeviceCapture::cb_packets(uv_async_t *handle)
{
    LiveDeviceCapture *obj = (LiveDeviceCapture *)handle->data;
    int packet_count;

    if (obj->closing)
        return obj->cleanup();

    obj->handling_packets = true;

    do
    {
        packet_count = pcap_dispatch(obj->pcap_handle,
                                     1,
                                     LiveDeviceCapture::EmitPacket,
                                     (u_char *)obj);
    } while (packet_count > 0 && !obj->closing);

    obj->handling_packets = false;
    if (obj->closing)
        obj->cleanup();
}
void CALLBACK LiveDeviceCapture::OnPacket(void *data, BOOLEAN didTimeout)
{
    // assert(!didTimeout);
    uv_async_t *async = (uv_async_t *)data;
    int r = uv_async_send(async);
    // assert(r == 0);
}
void LiveDeviceCapture::cb_close(uv_handle_t *handle)
{
}
#else
void LiveDeviceCapture::cb_packets(uv_poll_t *handle, int status, int events)
{
    // assert(status == 0);
    LiveDeviceCapture *obj = (LiveDeviceCapture *)handle->data;

    int packet_count;

    if (obj->closing)
        return obj->cleanup();

    if (events & UV_READABLE)
    {
        obj->handling_packets = true;

        do
        {
            packet_count = pcap_dispatch(obj->pcap_handle,
                                         1,
                                         LiveDeviceCapture::EmitPacket,
                                         (u_char *)obj);
        } while (packet_count > 0 && !obj->closing);

        obj->handling_packets = false;
        if (obj->closing)
            obj->cleanup();
    }
}
#endif
void LiveDeviceCapture::EmitPacket(u_char *user,
                                   const struct pcap_pkthdr *pkt_hdr,
                                   const u_char *pkt_data)
{
    LiveDeviceCapture *obj = (LiveDeviceCapture *)user;

    PacketEventData *eventData = new PacketEventData;
    eventData->pkt_data = pkt_data;
    eventData->copy_len = pkt_hdr->caplen;

    auto callback = [](Napi::Env env, Napi::Function jsCallback, PacketEventData *data)
    {
        // printf("%d \n", data->copy_len);
        jsCallback.Call({Napi::String::New(env, "data"),
                         Napi::Buffer<uint8_t>::Copy(env, data->pkt_data, data->copy_len)});
    };

    obj->tsEmit_.BlockingCall(eventData, callback);
}

void LiveDeviceCapture::Start(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    Napi::Function emit = info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
    Napi::Function bound = emit.Get("bind").As<Function>().Call(emit, {info.This()}).As<Function>();

    this->tsEmit_ = Napi::ThreadSafeFunction::New(
        bound.Env(),
        bound,          // JavaScript function called asynchronously
        "packet",       // Name
        0,              // Unlimited queue
        1,              // Only one thread will use this initially
        [](Napi::Env) { // Finalizer used to clean threads up
            // nativeThread.join();
        });
    ;

    char errbuf[PCAP_ERRBUF_SIZE] = "";
    // printf("%s", errbuf);
    this->pcap_handle = pcap_open_live(this->iface.c_str(), // name of the device
                                       65535,               // portion of the packet to capture.
                                       1,                   // promiscuous mode (nonzero means promiscuous)
                                       1000,                // read timeout
                                       errbuf               // error buffer
    );

    // if (pcap_set_buffer_size(this->pcap_handle, 10485760) != 0)
    // {
    //     Napi::TypeError::New(env, "Unable to set buffer size").ThrowAsJavaScriptException();
    //     return;
    // }
    if (strlen(errbuf) != 0)
    {
        Napi::TypeError::New(env, errbuf).ThrowAsJavaScriptException();
        return;
    }

    if (pcap_setnonblock(this->pcap_handle, 1, errbuf) == -1)
    {
        Napi::TypeError::New(env, errbuf).ThrowAsJavaScriptException();
        return;
    }

    if (this->pcap_handle == NULL)
    {
        Napi::TypeError::New(env, errbuf).ThrowAsJavaScriptException();
        return;
    }

    int r;

#ifdef _WIN32
    // uv_async_init
    r = uv_async_init(uv_default_loop(),
                      &this->async,
                      (uv_async_cb)LiveDeviceCapture::cb_packets);
    // assert(r == 0);
    this->async.data = this;
    r = RegisterWaitForSingleObject(
        &this->wait,
        pcap_getevent(this->pcap_handle),
        LiveDeviceCapture::OnPacket,
        &this->async,
        INFINITE,
        WT_EXECUTEINWAITTHREAD);
    if (!r)
    {
        char *errmsg = nullptr;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr,
                      GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&errmsg,
                      0,
                      nullptr);
        Napi::TypeError::New(env, errmsg).ThrowAsJavaScriptException();
        return;
    }

#else
    this->fd = pcap_get_selectable_fd(this->pcap_handle);
    r = uv_poll_init(uv_default_loop(), &this->poll_handle, this->fd);
    assert(r == 0);
    r = uv_poll_start(&this->poll_handle, UV_READABLE, LiveDeviceCapture::cb_packets);
    this->poll_handle.data = this;
#endif

    if (!this->filter.empty())
    {
        bpf_u_int32 NetMask = 0xffffff;

        // compile the filter
        if (pcap_compile(this->pcap_handle, &this->fcode, this->filter.c_str(), 1, NetMask) < 0)
        {
            Napi::TypeError::New(env, "Error compiling filter: wrong syntax.")
                .ThrowAsJavaScriptException();
            pcap_freecode(&this->fcode);
            pcap_close(this->pcap_handle);
            return;
        }

        // set the filter
        if (pcap_setfilter(this->pcap_handle, &this->fcode) < 0)
        {
            Napi::TypeError::New(env, "Error setting the filter")
                .ThrowAsJavaScriptException();
            pcap_freecode(&this->fcode);
            pcap_close(this->pcap_handle);
            return;
        }
    }
}

void LiveDeviceCapture::Stop(const Napi::CallbackInfo &info)
{
#ifdef _WIN32
    if (this->wait)
    {
        UnregisterWait(this->wait);
        wait = nullptr;
    }
    uv_close((uv_handle_t *)&this->async, cb_close);
#else
    uv_poll_stop(&this->poll_handle);
#endif
    pcap_close(this->pcap_handle);
}

void LiveDeviceCapture::SetFilter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
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

    this->filter = filter;

    if (this->pcap_handle == NULL)
    {    
        return;
    }
    
#ifdef _WIN32

#else
    uv_poll_stop(&this->poll_handle);
    uv_poll_start(&this->poll_handle, UV_READABLE, LiveDeviceCapture::cb_packets);
#endif
    bpf_u_int32 NetMask = 0xffffff;

    // compile the filter
    if (pcap_compile(this->pcap_handle, &this->fcode, filter.c_str(), 1, NetMask) < 0)
    {
        Napi::TypeError::New(env, "Error compiling filter: wrong syntax.")
            .ThrowAsJavaScriptException();
        pcap_freecode(&this->fcode);
        pcap_close(this->pcap_handle);
        return;
    }

    // set the filter
    if (pcap_setfilter(this->pcap_handle, &this->fcode) < 0)
    {
        Napi::TypeError::New(env, "Error setting the filter")
            .ThrowAsJavaScriptException();
        pcap_freecode(&this->fcode);
        pcap_close(this->pcap_handle);
        return;
    }

    pcap_freecode(&this->fcode);
}

void LiveDeviceCapture::SendPacket(const Napi::CallbackInfo &info)
{
    if (info.Length() != 1)
    {
        Napi::Error::New(info.Env(), "Expected exactly one argument")
            .ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsBuffer())
    {
        Napi::Error::New(info.Env(), "Expected an Buffer")
            .ThrowAsJavaScriptException();
        return;
    }

    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    size_t buffer_size = buf.Length();

    // pcap_send_packet
    if (pcap_sendpacket(this->pcap_handle,
                        (const u_char *)buf.Data(),
                        (int)buffer_size) == -1)
    {
        Napi::Error::New(info.Env(), pcap_geterr(this->pcap_handle))
            .ThrowAsJavaScriptException();
        return;
    }
}

bool LiveDeviceCapture::close()
{
    if (this->pcap_handle && !this->closing)
    {
#ifdef _WIN32
        if (this->wait)
        {
            UnregisterWait(this->wait);
            this->wait = nullptr;
        }
        uv_close((uv_handle_t *)&this->async, LiveDeviceCapture::cb_close);
#else
        uv_poll_stop(&this->poll_handle);
#endif
        this->closing = true;
        this->cleanup();
        return true;
    }
    return false;
}

void LiveDeviceCapture::cleanup()
{
#ifdef _WIN32
    this->wait = nullptr;
#endif
    this->handling_packets = false;
    this->closing = true;
}