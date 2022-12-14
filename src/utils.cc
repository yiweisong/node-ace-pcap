#include "utils.h"

using namespace Napi;

Napi::String GetNetworkInterface(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    char errbuf[PCAP_ERRBUF_SIZE];
    char name4[INET_ADDRSTRLEN];
    char name6[INET6_ADDRSTRLEN];
    // char *ip = nullptr;
    pcap_if_t *alldevs = nullptr, *dev;
    pcap_addr_t *addr;
    bool found = false;

    Napi::String iface;

    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        Napi::TypeError::New(env, errbuf)
            .ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    if (!info[0].IsString())
    {
        Napi::TypeError::New(env, "IP should be a string value")
            .ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    std::string ip = info[0].As<Napi::String>().Utf8Value();

    for (dev = alldevs; dev != nullptr; dev = dev->next)
    {
        if (dev->addresses != nullptr)
        {
            for (addr = dev->addresses; addr != nullptr; addr = addr->next)
            {
                if (addr->addr->sa_family == AF_INET || addr->addr->sa_family == AF_INET6)
                {
                    if (!ip.empty())
                    {
                        if (addr->addr->sa_family == AF_INET)
                        {
                            inet_ntop(AF_INET,
                                      (char *)&(((struct sockaddr_in *)(addr->addr))->sin_addr),
                                      name4, INET_ADDRSTRLEN);
                            if (strcmp(ip.c_str(), name4) != 0)
                                continue;
                        }
                        else if (addr->addr->sa_family == AF_INET6)
                        {
                            inet_ntop(AF_INET6,
                                      (char *)&(
                                          ((struct sockaddr_in6 *)(addr->addr))->sin6_addr),
                                      name6, INET6_ADDRSTRLEN);
                            if (strcmp(ip.c_str(), name6) != 0)
                                continue;
                        }
                    }
                    iface = Napi::String::New(env, dev->name);
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }

    if (alldevs)
        pcap_freealldevs(alldevs);

    // if (ip)
    //     free(ip);

    return iface;
}