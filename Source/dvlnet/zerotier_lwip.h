#pragma once

#ifdef ZEROTIER
#include "dvlnet/abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void print_ip6_addr(void* x);
void zt_ip6setup();

}
DEVILUTION_END_NAMESPACE
#endif