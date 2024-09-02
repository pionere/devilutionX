#include "platform/switch/network.h"

#ifndef NONET
#include <cstdlib>
#include <unistd.h>
#include <switch.h>

static int nxlink_sock = -1; // for stdio on Switch

void switch_enable_network()
{
	// enable network and stdio on Switch
	socketInitializeDefault();
	// enable error messages via nxlink on Switch
	nxlink_sock = nxlinkStdio();
	atexit(switch_disable_network);
}

void switch_disable_network()
{
	// disable network and stdio on Switch
	if (nxlink_sock != -1)
		close(nxlink_sock);
	socketExit();
}
#else
void switch_enable_network()
{
}
void switch_disable_network()
{
}
#endif