#include "abstract_net.h"

#include "dvlnet/base_protocol.h"
#include "dvlnet/cdwrap.h"
#include "dvlnet/protocol_zt.h"
#include "dvlnet/tcp_client.h"
#include "dvlnet/loopback.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

std::unique_ptr<abstract_net> abstract_net::make_net(unsigned provider)
{
	switch (provider) {
#ifdef TCPIP
	case SELCONN_TCP:
		return std::make_unique<tcp_client>();
#endif
#ifdef ZEROTIER
	case SELCONN_ZT:
		return std::make_unique<cdwrap<base_protocol<protocol_zt>>>();
#endif
	case SELCONN_LOOPBACK:
		return std::make_unique<loopback>();
	default:
		ASSUME_UNREACHABLE
		return NULL;
	}
}

} // namespace net
DEVILUTION_END_NAMESPACE
