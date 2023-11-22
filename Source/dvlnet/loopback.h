#pragma once

#include <deque>

#include "abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class loopback : public abstract_net {
public:
	bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;
	SNetMsgPkt* SNetReceiveMessage() override;
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size) override;
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) override;
	void SNetSendTurn(turn_t turn, const BYTE* data, unsigned size) override;
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) override;
	unsigned SNetGetTurnsInTransit() override;
	void SNetRegisterEventHandler(int evtype, SEVTHANDLER func) override { };
	void SNetUnregisterEventHandler(int evtype) override { };
	void SNetLeaveGame() override;
	void SNetDropPlayer(int playerid) override;
	void SNetDisconnect() override;

	loopback() = default;
	~loopback() override = default;

	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;

private:
	std::deque<buffer_t> message_queue;
	std::deque<SNetTurn> turn_queue;
};

} // namespace net
DEVILUTION_END_NAMESPACE
