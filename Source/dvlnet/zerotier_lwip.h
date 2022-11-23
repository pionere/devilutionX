#pragma once
#ifdef ZEROTIER
DEVILUTION_BEGIN_NAMESPACE
namespace net {

void print_ip6_addr(void* x);
void zt_ip6setup();

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
