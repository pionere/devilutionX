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

SNetTurnPkt* loopback::SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	SNetTurnPkt* pkt;
	SNetTurn* pt;
	BYTE* data;
	unsigned dwLen;

#ifdef _DEVMODE
	if (turn_queue.empty())
		ABORT();
#endif
	pt = &turn_queue.front();
	//      pnum           size
	dwLen = sizeof(BYTE) + sizeof(unsigned);
	if (pt->turn_id != 0)
		dwLen += pt->payload.size();

	pkt = (SNetTurnPkt*)DiabloAllocPtr(dwLen + sizeof(SNetTurnPkt) - sizeof(pkt->data));
	pkt->nmpTurn = pt->turn_id;
	pkt->nmpLen = dwLen;
	data = pkt->data;
	*data = PLR_SINGLE;
	data++;
	if (pt->turn_id != 0) {
		//       pnum           size
		dwLen -= sizeof(BYTE) + sizeof(unsigned);
		*(unsigned*)data = dwLen;
		data += sizeof(unsigned);
		memcpy(data, pt->payload.data(), dwLen);
		//data += dwLen;
	} else {
		*(unsigned*)data = 0;
		//data += sizeof(unsigned);
	}
	turn_queue.pop_front();
	return pkt;
}

turn_status loopback::SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
#ifdef _DEVMODE
	if (turn_queue.empty())
		ABORT();
#endif
	status[PLR_SINGLE] = PCS_CONNECTED | PCS_ACTIVE | PCS_TURN_ARRIVED;
	return TS_ACTIVE; // or TS_LIVE
}

uint32_t loopback::SNetLastTurn(unsigned (&status)[MAX_PLRS])
{
#ifdef _DEVMODE
	ABORT();
#endif
	return 0;
}

void loopback::SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size)
{
	turn_queue.emplace_back(turn, buffer_t(data, data + size));
}

void loopback::SNetLeaveGame(int reason)
{
	message_queue.clear();
	turn_queue.clear();
}

void loopback::SNetDropPlayer(int playerid)
{
#ifdef _DEVMODE
	ABORT();
#endif
}

void loopback::make_default_gamename(char (&gamename)[128])
{
	copy_cstr(gamename, "loopback");
}

} // namespace net
DEVILUTION_END_NAMESPACE
