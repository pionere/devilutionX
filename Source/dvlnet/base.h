#pragma once

#include <deque>
#include <map>
#include <memory>
#include <string>

#include "abstract_net.h"
#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class base : public abstract_net {
public:
	// bool create_game(const char* addrstr, unsigned port, const char* passwd, _uigamedata* gameData, char (&errorText)[256]) override;
	// bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* size) override;
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size) override;
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) override;
	void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size) override;
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) override;
	uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]) override;
	unsigned SNetGetTurnsInTransit() override;
	void SNetRegisterEventHandler(int evtype, SEVTHANDLER func) override;
	void SNetUnregisterEventHandler(int evtype) override;
	void SNetLeaveGame(int reason) override;
	void SNetDropPlayer(int playerid) override;

	~base() override = default;

	// void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;

protected:
	SEVTHANDLER registered_handlers[NUM_EVT_TYPES] = { };
	buffer_t game_init_info;

	SNetMessage message_last;
	std::deque<SNetMessage> message_queue;
	std::deque<SNetTurn> turn_queue[MAX_PLRS] = { };
	bool connected_table[MAX_PLRS] = { };

	plr_t plr_self;
	cookie_t cookie_self;

	packet_factory pktfty;

	void setup_password(const char* passwd);
	void setup_gameinfo(_uigamedata* gameData);
	virtual void poll() = 0;
	virtual void send_packet(packet& pkt) = 0;
	virtual void disconnect_net(plr_t pnum);
	void recv_local(packet& pkt);
	virtual void recv_connect(packet& pkt);

private:
	void recv_accept(packet& pkt);
	void recv_disconnect(packet& pkt);
	void run_event_handler(SNetEvent& ev);
	void disconnect_plr(plr_t pnum, leaveinfo_t leaveinfo);
};

} // namespace net
DEVILUTION_END_NAMESPACE
