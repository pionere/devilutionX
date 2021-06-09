#pragma once

#include <queue>
#include <string>

#include "dvlnet/base.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class loopback : public base {
private:
	std::queue<buffer_t> message_queue;
	buffer_t message_last;
	plr_t plr_single;

public:
	loopback()
	{
		plr_single = 0;
	};

	virtual bool create(const std::string &addrstr, unsigned port, const std::string &passwd);
	virtual bool join(const std::string &addrstr, unsigned port, const std::string &passwd);
	virtual void poll();
	virtual void send(packet &pkt);
	virtual bool SNetReceiveMessage(int *sender, char **data, unsigned *size);
	virtual void SNetSendMessage(int dest, const void *data, unsigned size);
	virtual bool SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn);
	virtual void SNetLeaveGame(int reason);
	virtual void SNetDropPlayer(int playerid);
	virtual uint32_t SNetGetOwnerTurnsWaiting();
	virtual uint32_t SNetGetTurnsInTransit();
	virtual std::string make_default_gamename();
};

} // namespace net
DEVILUTION_END_NAMESPACE
