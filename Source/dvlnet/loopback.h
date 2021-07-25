#pragma once

#include <deque>

#include "abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class loopback : public abstract_net {
public:
	bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info);
	bool join_game(const char* addrstr, unsigned port, const char* passwd);
	bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* size);
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size);
	bool SNetReceiveTurns(uint32_t *(&data)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	void SNetSendTurn(uint32_t turn);
	void SNetRegisterEventHandler(int evtype, SEVTHANDLER func);
	void SNetUnregisterEventHandler(int evtype);
	void SNetLeaveGame(int reason);
	void SNetDropPlayer(int playerid);
	uint32_t SNetGetOwnerTurnsWaiting();
	uint32_t SNetGetTurnsInTransit();

	loopback() = default;
	~loopback() = default;

	void make_default_gamename(char (&gamename)[128]);

private:
	buffer_t message_last;
	std::deque<buffer_t> message_queue;
};

} // namespace net
DEVILUTION_END_NAMESPACE
