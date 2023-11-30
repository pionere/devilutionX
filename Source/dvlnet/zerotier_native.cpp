#include "zerotier_native.h"
#ifdef ZEROTIER
#include <SDL.h>

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

typedef enum zerotier_status : uint8_t {
	ZT_READY,
	ZT_CONNECT,
	ZT_DOWN,
} zerotier_status;
static zerotier_status zt_status = ZT_DOWN;

static void zerotier_event_handler(void* ptr)
{
	zts_event_msg_t* msg = reinterpret_cast<zts_event_msg_t*>(ptr);
	switch (msg->event_code) {
	case ZTS_EVENT_NODE_ONLINE:
		// DoLog("ZeroTier: ZTS_EVENT_NODE_ONLINE");
		// zt_status = ZT_READY; -- don't change, zt-nodes are coming and going...
		break;
	case ZTS_EVENT_NODE_OFFLINE:
		// DoLog("ZeroTier: ZTS_EVENT_NODE_OFFLINE");
		// zt_status = ZT_DOWN; -- don't change, they are just joking...
		break;
	case ZTS_EVENT_NODE_DOWN:
		// DoLog("ZeroTier: ZTS_EVENT_NODE_DOWN");
		zt_status = ZT_DOWN;
		break;
	case ZTS_EVENT_NETWORK_READY_IP6:
		// DoLog("ZeroTier: ZTS_EVENT_NETWORK_READY_IP6, networkId=%llx", (unsigned long long)msg->network->net_id);
		// print_ip6_addr(&msg->network->assigned_addrs[0]);
		multicast_join();
		zt_status = ZT_READY;
		break;
	case ZTS_EVENT_NETWORK_ACCESS_DENIED:
		// DoLog("ZeroTier: ZTS_EVENT_NETWORK_ACCESS_DENIED"); -- TODO: what now?
		break;
	case ZTS_EVENT_NETWORK_CLIENT_TOO_OLD:
		// DoLog("ZeroTier: ZTS_EVENT_NETWORK_CLIENT_TOO_OLD"); -- TODO: what now?
		break;
	case ZTS_EVENT_NETWORK_DOWN:
		// DoLog("ZeroTier: ZTS_EVENT_NETWORK_DOWN");
		zt_status = ZT_DOWN;
		break;
	case ZTS_EVENT_NETWORK_NOT_FOUND:
		// DoLog("ZeroTier: ZTS_EVENT_NETWORK_NOT_FOUND"); -- TODO: what now?
		break;
	case ZTS_EVENT_NODE_UP:
		// DoLog("ZeroTier: ZTS_EVENT_NODE_UP");
		zts_net_join(ZtNetwork);
		break;
	}
}

bool zerotier_network_ready()
{
	// return zts_addr_is_assigned(ZtNetwork, ZTS_AF_INET);
	return zt_status == ZT_READY;
}

void zerotier_network_start()
{
	if (zt_status == ZT_DOWN) {
		zt_status = ZT_CONNECT;
		std::string ztpath = GetPrefPath();
		ztpath += "zerotier";
		zts_init_from_storage(ztpath.c_str());
		zts_init_set_event_handler(&zerotier_event_handler);
		zts_node_start();
	}
}

void zerotier_network_stop()
{
	zts_node_stop();
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
