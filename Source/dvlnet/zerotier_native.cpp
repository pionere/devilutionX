#include "zerotier_native.h"

#ifdef ZEROTIER
#include <atomic>
#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#else
#include "utils/sdl2_backports.h"
#endif

#include <ZeroTierSockets.h>
#include <cstdlib>

#include "utils/paths.h"

#include "dvlnet/zerotier_lwip.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

//static constexpr uint64_t zt_earth = 0x8056c2e21c000001;
static constexpr uint64_t ZtNetwork = 0xaf78bf943649eb12;

static std::atomic_bool zt_network_ready(false);
static std::atomic_bool zt_node_online(false);
static std::atomic_bool zt_started(false);
static std::atomic_bool zt_joined(false);

static void Callback(struct zts_callback_msg *msg)
{
	//printf("callback %i\n", msg->eventCode);
	if (msg->eventCode == ZTS_EVENT_NODE_ONLINE) {
		SDL_Log("ZeroTier: ZTS_EVENT_NODE_ONLINE, nodeId=%llx\n", (unsigned long long)msg->node->address);
		zt_node_online = true;
		if (!zt_joined) {
			zts_join(ZtNetwork);
			zt_joined = true;
		}
	} else if (msg->eventCode == ZTS_EVENT_NODE_OFFLINE) {
		SDL_Log("ZeroTier: ZTS_EVENT_NODE_OFFLINE\n");
		zt_node_online = false;
	} else if (msg->eventCode == ZTS_EVENT_NETWORK_READY_IP6) {
		SDL_Log("ZeroTier: ZTS_EVENT_NETWORK_READY_IP6, networkId=%llx\n", (unsigned long long)msg->network->nwid);
		zt_ip6setup();
		zt_network_ready = true;
	} else if (msg->eventCode == ZTS_EVENT_ADDR_ADDED_IP6) {
		print_ip6_addr(&(msg->addr->addr));
	}
}

bool zerotier_network_ready()
{
	return zt_network_ready && zt_node_online;
}

void zerotier_network_stop()
{
	zts_stop();
}

void zerotier_network_start()
{
	if (zt_started)
		return;
	zts_start(GetPrefPath().c_str(), (void (*)(void *))Callback, 0);
	std::atexit(zerotier_network_stop);
}

}
DEVILUTION_END_NAMESPACE
#endif
