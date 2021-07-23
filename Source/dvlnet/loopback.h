#pragma once

#include "base.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class loopback : public base {
public:

	virtual bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info);
	virtual bool join_game(const char* addrstr, unsigned port, const char* passwd);
	virtual void SNetSendMessage(int receiver, const BYTE* data, unsigned size);
	virtual bool SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn);
	virtual void SNetLeaveGame(int reason);
	virtual void SNetDropPlayer(int playerid);
	virtual uint32_t SNetGetOwnerTurnsWaiting();
	virtual uint32_t SNetGetTurnsInTransit();
	virtual void make_default_gamename(char (&gamename)[128]);

protected:
	virtual void poll();
	virtual void send_packet(packet &pkt);
};

} // namespace net
DEVILUTION_END_NAMESPACE
