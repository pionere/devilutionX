#pragma once

#include <array>
#include <deque>
#include <map>
#include <memory>
#include <string>

#include "dvlnet/abstract_net.h"
#include "dvlnet/packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class base : public abstract_net {
public:
	virtual bool create(const std::string &addrstr, unsigned port, const std::string &passwd) = 0;
	virtual bool join(const std::string &addrstr, unsigned port, const std::string &passwd) = 0;

	virtual bool SNetReceiveMessage(int *sender, char **data, unsigned *size);
	virtual void SNetSendMessage(int dest, const void *data, unsigned size);
	virtual bool SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn);
	virtual void SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func);
	virtual void SNetUnregisterEventHandler(event_type evtype, SEVTHANDLER func);
	virtual void SNetLeaveGame(int reason);
	virtual void SNetDropPlayer(int playerid);
	virtual uint32_t SNetGetOwnerTurnsWaiting();
	virtual uint32_t SNetGetTurnsInTransit();

	virtual void poll() = 0;
	virtual void send(packet &pkt) = 0;
	virtual void disconnect_net(plr_t pnum);

	void setup_gameinfo(buffer_t info);

	virtual void setup_password(std::string pw);

	virtual ~base() = default;

protected:
	std::map<event_type, SEVTHANDLER> registered_handlers;
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

	std::unique_ptr<packet_factory> pktfty;

	void handle_accept(packet &pkt);
	void recv_local(packet &pkt);
	void run_event_handler(_SNETEVENT &ev);

private:
	plr_t get_owner();
	void clear_msg(plr_t pnum);
};

} // namespace net
DEVILUTION_END_NAMESPACE
