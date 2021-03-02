#pragma once

#include <string>
#include <queue>

#include "all.h"
#include "dvlnet/abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class loopback : public abstract_net {
private:
	std::queue<buffer_t> message_queue;
	buffer_t message_last;
	int plr_single;

public:
	loopback()
	{
		plr_single = 0;
	};

	virtual int create(std::string addrstr, std::string passwd);
	virtual int join(std::string addrstr, std::string passwd);
	virtual bool SNetReceiveMessage(int *sender, char **data, int *size);
	virtual bool SNetSendMessage(int dest, void *data, unsigned int size);
	virtual bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned int (&size)[MAX_PLRS], DWORD (&status)[MAX_PLRS]);
	virtual bool SNetSendTurn(char *data, unsigned int size);
	virtual bool SNetGetProviderCaps(struct _SNETCAPS *caps);
	virtual bool SNetRegisterEventHandler(event_type evtype,
		SEVTHANDLER func);
	virtual bool SNetUnregisterEventHandler(event_type evtype,
		SEVTHANDLER func);
	virtual bool SNetLeaveGame(int type);
	virtual bool SNetDropPlayer(int playerid, DWORD flags);
	virtual bool SNetGetOwnerTurnsWaiting(DWORD *turns);
	virtual bool SNetGetTurnsInTransit(DWORD *turns);
	virtual void setup_gameinfo(buffer_t info);
};

} // namespace net
DEVILUTION_END_NAMESPACE
