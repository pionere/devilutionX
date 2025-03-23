#pragma once
#ifdef ZEROTIER

#include "../defs.h"

#include <ZeroTierSockets.h>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

bool zerotier_network_ready();
void zerotier_network_start();
void zerotier_network_stop();
bool zerotier_current_addr(zts_sockaddr_storage* addr);

// NOTE: We have patched our libzt to have the corresponding multicast
// MAC hardcoded, since libzt is still missing the proper handling.
const unsigned char dvl_multicast_addr[16] = {
	0xff, 0x0e, 0xa8, 0xa9, 0xb6, 0x11, 0x61, 0xce,
	0x04, 0x12, 0xfd, 0x73, 0x37, 0x86, 0x6f, 0xb7
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
