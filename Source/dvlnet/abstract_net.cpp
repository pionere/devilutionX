#include "abstract_net.h"

#include "dvlnet/loopback.h"
#include "dvlnet/tcp_client.h"
#include "dvlnet/tcpd_client.h"
#include "dvlnet/tcp_host.h"
#include "dvlnet/zt_client.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

abstract_net* abstract_net::make_net(unsigned provider)
{
	switch (provider) {
#ifdef TCPIP
#ifndef HOSTONLY
	case SELCONN_TCP:
		return new tcp_client();
	case SELCONN_TCPD:
		return new tcpd_client();
#endif // HOSTONLY
#ifndef NOHOSTING
	case SELCONN_TCPS:
		return new tcp_host_client(SRV_BASIC);
	case SELCONN_TCPDS:
		return new tcp_host_client(SRV_DIRECT);
#endif // NOHOSTING
#endif // TCPIP
#ifndef HOSTONLY
#ifdef ZEROTIER
	case SELCONN_ZT:
		return new zt_client();
#endif
	case SELCONN_LOOPBACK:
		return new loopback();
#endif
	default:
		ASSUME_UNREACHABLE
		return NULL;
	}
}

} // namespace net
DEVILUTION_END_NAMESPACE
