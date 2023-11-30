#include "zerotier_lwip.h"
#ifdef ZEROTIER
#include <lwip/mld6.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>

#include "utils/log.h"

#include "dvlnet/zerotier_native.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void print_ip6_addr(void* x)
{
	char ipstr[INET6_ADDRSTRLEN];
	auto* in = static_cast<sockaddr_in6*>(x);
	lwip_inet_ntop(AF_INET6, &(in->sin6_addr), ipstr, INET6_ADDRSTRLEN);
	DoLog("ZeroTier: addr=%s", ipstr);
}

void multicast_join()
{
	ip6_addr_t mcaddr;
	memcpy(mcaddr.addr, dvl_multicast_addr, 16);
	mcaddr.zone = IP6_NO_ZONE;
	LOCK_TCPIP_CORE();
	mld6_joingroup(IP6_ADDR_ANY6, &mcaddr);
	UNLOCK_TCPIP_CORE();
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
