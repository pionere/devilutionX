#pragma once

#include <array>
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
	//virtual bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info) = 0;
	//virtual bool join_game(const char* addrstr, unsigned port, const char* passwd) = 0;

	virtual bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* size);
	virtual void SNetSendMessage(int receiver, const BYTE* data, unsigned size);
	virtual bool SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn);
	virtual void SNetRegisterEventHandler(int evtype, SEVTHANDLER func);
	virtual void SNetUnregisterEventHandler(int evtype);
	virtual void SNetLeaveGame(int reason);
	virtual void SNetDropPlayer(int playerid);
	virtual uint32_t SNetGetOwnerTurnsWaiting();
	virtual uint32_t SNetGetTurnsInTransit();

	virtual ~base() = default;

protected:
	SEVTHANDLER registered_handlers[NUM_EVT_TYPES] = { };
	buffer_t game_init_info;

	struct message_t {
		int sender; // change int to something else in devilution code later
		buffer_t payload;
		message_t()
		    : sender(-1)
		    , payload({})
		{
		}
		message_t(int s, buffer_t p)
		    : sender(s)
		    , payload(p)
		{
		}
	};

	message_t message_last;
	std::deque<message_t> message_queue;
	std::array<turn_t, MAX_PLRS> turn_last = {};
	std::array<std::deque<turn_t>, MAX_PLRS> turn_queue;
	std::array<bool, MAX_PLRS> connected_table = {};

	plr_t plr_self = PLR_BROADCAST;
	cookie_t cookie_self = 0;

	packet_factory pktfty;

	void setup_password(const char* passwd);
	void setup_gameinfo(buffer_t info);
	virtual void poll() = 0;
	virtual void send_packet(packet &pkt) = 0;
	virtual void disconnect_net(plr_t pnum);
	void handle_accept(packet &pkt);
	void recv_local(packet &pkt);
	void run_event_handler(SNetEvent &ev);

private:
	plr_t get_owner();
	void clear_msg(plr_t pnum);
};

} // namespace net
DEVILUTION_END_NAMESPACE
