#include "base.h"

#include <cstring>
#include <memory>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void base::setup_gameinfo(buffer_t info)
{
	game_init_info = std::move(info);
}

void base::setup_password(std::string pw)
{
	pktfty = std::make_unique<packet_factory>(pw);
}

void base::run_event_handler(_SNETEVENT &ev)
{
	auto f = registered_handlers[static_cast<event_type>(ev.eventid)];
	if (f != NULL) {
		f(&ev);
	}
}

void base::disconnect_net(plr_t pnum)
{
}

void base::handle_accept(packet &pkt)
{
	if (plr_self != PLR_BROADCAST) {
		return; // already have player id - should not happen...
	}
	if (pkt.cookie() == cookie_self) {
		plr_self = pkt.newplr();
		connected_table[plr_self] = true;
	}
	auto &pkt_info = pkt.info();
#ifdef ZEROTIER
	if (pkt_info.size() != sizeof(_SNETGAMEDATA)) {
		ABORT();
	}
	// we joined and did not create
	game_init_info = pkt.info();
#endif
	_SNETEVENT ev;
	ev.eventid = EVENT_TYPE_JOIN_ACCEPTED;
	ev.playerid = plr_self;
	ev._eData = const_cast<unsigned char *>(pkt_info.data());
	ev.databytes = pkt_info.size();
	run_event_handler(ev);
}

void base::clear_msg(plr_t pnum)
{
	message_queue.erase(std::remove_if(message_queue.begin(),
	                        message_queue.end(),
	                        [&](message_t &msg) {
		                        return msg.sender == pnum;
	                        }),
	    message_queue.end());
}

void base::recv_local(packet &pkt)
{
	plr_t pkt_plr = pkt.src();
	if (pkt_plr < MAX_PLRS) {
		connected_table[pkt_plr] = true;
	}
	switch (pkt.type()) {
	case PT_MESSAGE:
		message_queue.emplace_back(message_t(pkt_plr, pkt.message()));
		break;
	case PT_TURN:
		turn_queue[pkt_plr].push_back(pkt.turn());
		break;
	case PT_JOIN_ACCEPT:
		handle_accept(pkt);
		break;
	case PT_CONNECT:
		connected_table[pkt.newplr()] = true; // this can probably be removed
		break;
	case PT_DISCONNECT:
		pkt_plr = pkt.newplr();
		if (pkt_plr != plr_self) {
			if (connected_table[pkt_plr]) {
				auto leaveinfo = pkt.leaveinfo();
				_SNETEVENT ev;
				ev.eventid = EVENT_TYPE_PLAYER_LEAVE_GAME;
				ev.playerid = pkt_plr;
				ev._eData = reinterpret_cast<unsigned char *>(&leaveinfo);
				ev.databytes = sizeof(leaveinfo);
				run_event_handler(ev);
				connected_table[pkt_plr] = false;
				disconnect_net(pkt_plr);
				clear_msg(pkt_plr);
				turn_queue[pkt_plr].clear();
			}
		} else {
			ABORT(); // we were dropped by the owner?!?
		}
		break;
	default:
		break;
		// otherwise drop
	}
}

bool base::SNetReceiveMessage(int *sender, char **data, unsigned *size)
{
	poll();
	if (message_queue.empty())
		return false;
	message_last = message_queue.front();
	message_queue.pop_front();
	*sender = message_last.sender;
	*size = message_last.payload.size();
	*data = reinterpret_cast<char *>(message_last.payload.data());
	return true;
}

void base::SNetSendMessage(int playerID, const void *data, unsigned size)
{
	if (playerID != SNPLAYER_ALL && playerID != SNPLAYER_OTHERS
	    && (playerID < 0 || playerID >= MAX_PLRS))
		ABORT();
	auto rawMessage = reinterpret_cast<const unsigned char *>(data);
	buffer_t message(rawMessage, rawMessage + size);
	if (playerID == plr_self || playerID == SNPLAYER_ALL)
		message_queue.emplace_back(plr_self, message);
	plr_t dest;
	if (playerID == SNPLAYER_ALL || playerID == SNPLAYER_OTHERS)
		dest = PLR_BROADCAST;
	else
		dest = playerID;
	if (dest != plr_self) {
		auto pkt = pktfty->make_out_packet<PT_MESSAGE>(plr_self, dest, message);
		send(*pkt);
	}
}

bool base::SNetReceiveTurns(uint32_t *(&data)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
{
	poll();
	bool allTurnsArrived = true;
	for (auto i = 0; i < MAX_PLRS; ++i) {
		status[i] = 0;
		if (connected_table[i]) {
			status[i] |= PS_CONNECTED;
			if (turn_queue[i].empty())
				allTurnsArrived = false;
		}
	}
	if (allTurnsArrived) {
		for (auto i = 0; i < MAX_PLRS; ++i) {
			if (connected_table[i]) {
				status[i] |= PS_ACTIVE | PS_TURN_ARRIVED;
				turn_last[i] = turn_queue[i].front();
				turn_queue[i].pop_front();
				data[i] = &turn_last[i];
			}
		}
		return true;
	}
	for (auto i = 0; i < MAX_PLRS; ++i) {
		if (connected_table[i]) {
			if (!turn_queue[i].empty()) {
				status[i] |= PS_ACTIVE;
			}
		}
	}
	return false;
}

void base::SNetSendTurn(uint32_t turn)
{
	static_assert(sizeof(turn_t) == sizeof(uint32_t), "SNetSendTurn: sizemismatch between turn_t and turn");
	auto pkt = pktfty->make_out_packet<PT_TURN>(plr_self, PLR_BROADCAST, turn);
	send(*pkt);
	turn_queue[plr_self].push_back(pkt->turn());
}

/*void base::SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	//caps->size = 0;                  // unused
	caps->flags = 0;                 // unused
	caps->maxmessagesize = MAX_NETMSG_SIZE; // the largest message to send during delta-load
	caps->maxqueuesize = 0;          // unused
	caps->maxplayers = MAX_PLRS;     // unused (part of _SNETGAMEDATA)
	caps->bytessec = 1000000;        // estimated speed of the connection (to determine if wait is necessary during delta load)
	caps->latencyms = 0;             // unused
	caps->defaultturnssec = 10;      // number of turns to process in a second (to determine the net-update-rate)
	caps->defaultturnsintransit = 1; // maximum acceptable number of turns in queue?
}*/

void base::SNetUnregisterEventHandler(event_type evtype, SEVTHANDLER func)
{
	registered_handlers.erase(evtype);
}

/*
 *  EVENT_TYPE_PLAYER_LEAVE_GAME:
 *    handled by the engine, raised when a player leaves the game
 *  EVENT_TYPE_JOIN_ACCEPTED:
 *    handled by UI, raised during SNetCreateGame/SNetJoinGame
 *  EVENT_TYPE_PLAYER_MESSAGE:
 *    not implemented
 */
void base::SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func)
{
	registered_handlers[evtype] = func;
}

void base::SNetLeaveGame(int reason)
{
	auto pkt = pktfty->make_out_packet<PT_DISCONNECT>(plr_self, PLR_BROADCAST,
	    plr_self, (leaveinfo_t)reason);
	send(*pkt);
}

void base::SNetDropPlayer(int playerid)
{
	auto pkt = pktfty->make_out_packet<PT_DISCONNECT>(plr_self,
	    PLR_BROADCAST,
	    (plr_t)playerid,
	    (leaveinfo_t)LEAVE_DROP);
	send(*pkt);
	recv_local(*pkt);
}

plr_t base::get_owner()
{
	for (auto i = 0; i < MAX_PLRS; ++i) {
		if (connected_table[i]) {
			return i;
		}
	}
	return PLR_BROADCAST; // should be unreachable
}

uint32_t base::SNetGetOwnerTurnsWaiting()
{
	return turn_queue[get_owner()].size();
}

uint32_t base::SNetGetTurnsInTransit()
{
	return turn_queue[plr_self].size();
}

} // namespace net
DEVILUTION_END_NAMESPACE
