#include <napi.h>
#include "live_device_capture.h"
#include "utils.h"

#ifdef _WIN32
#include <stdio.h>
#include <tchar.h>
BOOL LoadNpcapDlls()
{
	_TCHAR npcap_dir[512];
	UINT len;
	len = GetSystemDirectory(npcap_dir, 480);
	if (!len) {
		fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
		return FALSE;
	}
	_tcscat_s(npcap_dir, 512, _T("\\Npcap"));
	if (SetDllDirectory(npcap_dir) == 0) {
		fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
		return FALSE;
	}
	return TRUE;
}
#endif

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
#ifdef _WIN32
	/* Load Npcap and its functions. */
	if (!LoadNpcapDlls())
	{
		fprintf(stderr, "Couldn't load Npcap\n");
		exit(1);
	}
#endif

  Napi::HandleScope scope(env);
  LiveDeviceCapture::Init(env, exports);
  exports.Set("GetNetworkInterface", Napi::Function::New(env, GetNetworkInterface));

  return exports;
}

NODE_API_MODULE(pcap, Init);