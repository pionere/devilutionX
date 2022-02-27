#pragma once

#include <deque>

#include "abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class loopback : public abstract_net {
public:
	bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info, char (&errorText)[256]);
	bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]);
	bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* size);
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size);
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]);
	void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size);
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]);
	uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]);
//#ifdef ADAPTIVE_NETUPDATE
	unsigned SNetGetTurnsInTransit();
//#endif
	void SNetRegisterEventHandler(int evtype, SEVTHANDLER func) { };
	void SNetUnregisterEventHandler(int evtype) { };
	void SNetLeaveGame(int reason);
	void SNetDropPlayer(int playerid);

	loopback() = default;
	~loopback() = default;

	void make_default_gamename(char (&gamename)[128]);

private:
	buffer_t message_last;
	std::deque<buffer_t> message_queue;
	std::deque<SNetTurn> turn_queue;
};

} // namespace net
DEVILUTION_END_NAMESPACE
