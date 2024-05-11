#include "sockets.hpp"
#ifndef NONET
#include <malloc.h>
#include <cstdlib>
#include <3ds.h>
#include "utils/log.h"

constexpr auto SOC_ALIGN = 0x1000;
constexpr auto SOC_BUFFERSIZE = 0x100000;
static u32 *socBuffer;
static bool initialized;

static bool waitForWifi()
{
	// 100 ms
	constexpr s64 sleepNano = 100 * 1000 * 1000;

	// 5 sec
	constexpr int loopCount = 5 * 1000 / 100;

	uint32_t wifi = 0;
	for (int i = 0; i < loopCount; ++i) {
		if (R_SUCCEEDED(ACU_GetWifiStatus(&wifi)) && wifi)
			return true;

		svcSleepThread(sleepNano);
	}

	return false;
}

void n3ds_socExit()
{
	if (socBuffer == NULL)
		return;

	socExit();
	free(socBuffer);
	socBuffer = NULL;
}

void n3ds_socInit()
{
	if (!waitForWifi()) {
		DoLog("n3ds_socInit: Wifi off");
		return;
	}

	socBuffer = (u32 *)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	if (socBuffer == NULL) {
		DoLog("n3ds_socInit: memalign() failed");
		return;
	}

	Result result = socInit(socBuffer, SOC_BUFFERSIZE);
	if (!R_SUCCEEDED(result)) {
		DoLog("n3ds_socInit: socInit() failed");
		free(socBuffer);
		return;
	}

	if (!initialized)
		atexit([]() { n3ds_socExit(); });
	initialized = true;
}
#else
void n3ds_socExit()
{
}
void n3ds_socInit()
{
}
#endif // !NONET