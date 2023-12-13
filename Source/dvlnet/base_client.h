#pragma once
#ifndef NONET
#include <deque>
#include <map>
#include <memory>
#include <string>

#include "abstract_net.h"
#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

typedef enum connection_status {
	CON_CONNECTED = 0x01, // connection is active
	CON_LEAVING   = 0x02, // the player initiated a disconnect
} connection_status;

class base_client : public abstract_net {
public:
	// bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	SNetMsgPkt* SNetReceiveMessage() override;
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size) override;
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) override;
	void SNetSendTurn(turn_t turn, const BYTE* data, unsigned size) override;
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) override;
	unsigned SNetGetTurnsInTransit() override;
	void SNetRegisterEventHandler(int evtype, SEVTHANDLER func) override;
	void SNetUnregisterEventHandler(int evtype) override;
	void SNetLeaveGame() override;
	void SNetDropPlayer(int playerid) override;
	void SNetDisconnect() override;

	~base_client() override = default;

	// void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;

	void recv_local(packet& pkt);
	turn_t last_recv_turn() const;

protected:
	SEVTHANDLER registered_handlers[NUM_EVT_TYPES] = { };
	SNetGameData game_init_info;

	std::deque<SNetMessage> message_queue;
	std::deque<SNetTurn> turn_queue[MAX_PLRS] = { };
	int connected_table[MAX_PLRS] = { }; // connection_status

	plr_t plr_self = PLR_BROADCAST;
	cookie_t cookie_self;
	turn_t lastRecvTurn = -1;
	packet_factory pktfty;

	void setup_password(const char* passwd);
	void setup_gameinfo(_uigamedata* gameData);
	void run_event_handler(SNetEventHdr* ev);
	virtual void poll() = 0;
	virtual void send_packet(packet& pkt) = 0;
	virtual void disconnect_net(plr_t pnum);
	virtual bool recv_connect(packet& pkt);
	virtual bool recv_accept(packet& pkt);
	virtual void close();

private:
	void recv_disconnect(packet& pkt);
	void disconnect_plr(plr_t pnum);
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // !NONET