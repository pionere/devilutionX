#include "zerotier_native.h"
#ifdef ZEROTIER
#include <SDL.h>
#include <atomic>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#else
#include "utils/sdl2_backports.h"
#endif

#include <ZeroTierSockets.h>
#include <cstdlib>

#include "utils/log.h"
#include "utils/paths.h"

#include "dvlnet/zerotier_lwip.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

//static constexpr uint64_t zt_earth = 0x8056c2e21c000001;
static constexpr uint64_t ZtNetwork = 0xaf78bf943649eb12;

static std::atomic_bool zt_network_ready(false);
static std::atomic_bool zt_node_online(false);
static std::atomic_bool zt_joined(false);

static void zerotier_event_handler(void* ptr)
{
	zts_event_msg_t* msg = reinterpret_cast<zts_event_msg_t*>(ptr);
	switch (msg->event_code) {
	case ZTS_EVENT_NODE_ONLINE:
		DoLog("ZeroTier: ZTS_EVENT_NODE_ONLINE, nodeId=%llx", (unsigned long long)msg->node->node_id);
		zt_node_online = true;
		if (!zt_joined) {
			zts_net_join(ZtNetwork);
			zt_joined = true;
		}
		break;
	case ZTS_EVENT_NODE_OFFLINE:
		DoLog("ZeroTier: ZTS_EVENT_NODE_OFFLINE");
		zt_node_online = false;
		break;
	case ZTS_EVENT_NETWORK_READY_IP6:
		DoLog("ZeroTier: ZTS_EVENT_NETWORK_READY_IP6, networkId=%llx", (unsigned long long)msg->network->net_id);
		multicast_join();
		zt_network_ready = true;
	case ZTS_EVENT_ADDR_ADDED_IP6:
		print_ip6_addr(&(msg->addr->addr));
		break;
	}
}

bool zerotier_network_ready()
{
	return zt_network_ready && zt_node_online;
}

void zerotier_network_start()
{
	std::string ztpath = GetPrefPath();
	ztpath += "zerotier";
	zts_init_from_storage(ztpath.c_str());
	zts_init_set_event_handler(&zerotier_event_handler);
	zts_node_start();
}

void zerotier_network_stop()
{
	zts_node_stop();
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
