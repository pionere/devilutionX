#include "loopback.h"

#include "utils/stubs.h"
#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

static constexpr plr_t PLR_SINGLE = 0;

bool loopback::create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info)
{
	return true;
}

bool loopback::join_game(const char* addrstr, unsigned port, const char* passwd)
{
#ifdef _DEVMODE
	ABORT();
#endif
	return false;
}

bool loopback::SNetReceiveMessage(int* sender, BYTE** data, unsigned* size)
{
	if (message_queue.empty())
		return false;
	message_last = message_queue.front();
	message_queue.pop_front();
	*sender = PLR_SINGLE;
	*size = message_last.size();
	*data = message_last.data();
	return true;
}

void loopback::SNetSendMessage(int receiver, const BYTE* data, unsigned size)
{
#ifdef _DEVMODE
	if (receiver != SNPLAYER_ALL && receiver != PLR_SINGLE)
		ABORT();
#endif
	message_queue.emplace_back(data, data + size);
}

bool loopback::SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
{
	// todo: check that this is safe
	//for (auto i = 0; i < MAX_PLRS; ++i) {
	//	turns[i] = NULL;
	//	status[i] = 0;
	//}
	return true;
}

void loopback::SNetSendTurn(uint32_t turn)
{
}

void loopback::SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
}

void loopback::SNetUnregisterEventHandler(int evtype)
{
}

void loopback::SNetLeaveGame(int reason)
{
	message_queue.clear();
}

void loopback::SNetDropPlayer(int playerid)
{
}

uint32_t loopback::SNetGetOwnerTurnsWaiting()
{
	return 0;
}

uint32_t loopback::SNetGetTurnsInTransit()
{
	return 0;
}

void loopback::make_default_gamename(char (&gamename)[128])
{
	copy_cstr(gamename, "loopback");
}

} // namespace net
DEVILUTION_END_NAMESPACE
