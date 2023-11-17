#pragma once
#ifdef ZEROTIER
//#include <exception>
//#include <map>
#include <memory>
//#include <string>
//#include <vector>

#include "abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class cdwrap : public abstract_net {
private:
	std::unique_ptr<abstract_net> dvlnet_wrap;
	SEVTHANDLER registered_handlers[NUM_EVT_TYPES] = { };

	void reset();

public:
	bool create_game(const char* addrstr, unsigned port, const char* passwd, _uigamedata* gameData, char (&errorText)[256]) override;
	bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;
	bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* size) override;
	void SNetSendMessage(int receiver, const BYTE* data, unsigned int size) override;
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) override;
	void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size) override;
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) override;
	uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]) override;
	unsigned SNetGetTurnsInTransit() override;
	void SNetRegisterEventHandler(int evtype, SEVTHANDLER func) override;
	void SNetUnregisterEventHandler(int evtype) override;
	void SNetLeaveGame() override;
	void SNetDropPlayer(int playerid) override;
	void SNetDisconnect() override;
	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;

	cdwrap();
	~cdwrap() override = default;
};


} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
