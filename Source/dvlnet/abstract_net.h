#pragma once

#include <memory>
#ifdef ZEROTIER
#include <string>
#endif
#include <vector>

#include "all.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

typedef std::vector<BYTE> buffer_t;

struct SNetTurn {
	turn_t turn_id;
	buffer_t payload;
	SNetTurn(turn_t t, buffer_t p)
	    : turn_id(t)
	    , payload(p)
	{
	}
};

struct SNetMessage {
	int sender;
	buffer_t payload;
	SNetMessage(int s, buffer_t p)
	    : sender(s)
	    , payload(p)
	{
	}
};

static constexpr uint8_t PLR_MASTER = SNPLAYER_MASTER;
static constexpr uint8_t PLR_BROADCAST = SNPLAYER_ALL;

class abstract_net {
public:
	virtual bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) = 0;
	virtual SNetMsgPkt* SNetReceiveMessage() = 0;
	virtual void SNetSendMessage(int receiver, const BYTE* data, unsigned size) = 0;
	virtual SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) = 0;
	virtual void SNetSendTurn(turn_t turn, const BYTE* data, unsigned size) = 0;
	virtual turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) = 0;
	virtual unsigned SNetGetTurnsInTransit() = 0;
	virtual void SNetRegisterEventHandler(int evtype, SEVTHANDLER func) = 0;
	virtual void SNetUnregisterEventHandler(int evtype) = 0;
	virtual void SNetLeaveGame() = 0;
	virtual void SNetDropPlayer(int playerid) = 0;
	virtual void SNetDisconnect() = 0;
	virtual ~abstract_net() = default;

	virtual void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) = 0;
	static abstract_net* make_net(unsigned provider);
};

} // namespace net
DEVILUTION_END_NAMESPACE
