#pragma once

#include <string>
#include <queue>

#include "all.h"
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
	virtual bool SNetReceiveMessage(int *sender, char **data, int *size);
	virtual bool SNetSendMessage(int dest, void *data, unsigned int size);
	virtual bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned (&size)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual bool SNetSendTurn(char *data, unsigned int size);
	virtual bool SNetGetProviderCaps(struct _SNETCAPS *caps);
	virtual void SNetLeaveGame(int type);
	virtual bool SNetDropPlayer(int playerid, unsigned flags);
	virtual bool SNetGetOwnerTurnsWaiting(DWORD *turns);
	virtual bool SNetGetTurnsInTransit(DWORD *turns);
	virtual std::string make_default_gamename();
};

} // namespace net
DEVILUTION_END_NAMESPACE
