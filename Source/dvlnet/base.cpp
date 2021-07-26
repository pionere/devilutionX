#include "base.h"

#include <cstring>
#include <memory>
#include "utils/stubs.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void base::setup_gameinfo(buffer_t info)
{
	game_init_info = std::move(info);
}

void base::setup_password(const char* passwd)
{
	pktfty.setup_password(passwd);
}

void base::run_event_handler(SNetEvent &ev)
{
	auto f = registered_handlers[ev.eventid];
	if (f != NULL) {
		f(&ev);
	}
}

void base::disconnect_net(plr_t pnum)
{
}

void base::recv_accept(packet &pkt)
{
	if (plr_self != PLR_BROADCAST || pkt.pktJoinAccCookie() != cookie_self) {
		// ignore the packet if player id is set or the cookie does not match
		return;
	}
	plr_self = pkt.pktJoinAccPlr();
	if (plr_self >= MAX_PLRS) {
		// Invalid player id -> ignore
		plr_self = PLR_BROADCAST;
		return;
	}
	connected_table[plr_self] = true;
	auto &pkt_info = pkt.pktJoinAccInfo();
#ifdef ZEROTIER
	//if (pkt_info.size() != sizeof(SNetGameData)) {
	//	ABORT();
	//}
	// we joined and did not create
	game_init_info = buffer_t((BYTE*)&pkt_info, (BYTE*)&pkt_info + sizeof(SNetGameData));
#endif
	SNetEvent ev;
	ev.eventid = EVENT_TYPE_JOIN_ACCEPTED;
	ev.playerid = plr_self;
	ev._eData = (BYTE*)&pkt_info;
	ev.databytes = sizeof(SNetGameData);
	run_event_handler(ev);
}

void base::clear_msg(plr_t pnum)
{
	turn_queue[pnum].clear();
	message_queue.erase(std::remove_if(message_queue.begin(),
	                        message_queue.end(),
	                        [&](message_t &msg) {
		                        return msg.sender == pnum;
	                        }),
	    message_queue.end());
}

void base::disconnect_plr(plr_t pnum, leaveinfo_t leaveinfo)
{
	SNetEvent ev;
	ev.eventid = EVENT_TYPE_PLAYER_LEAVE_GAME;
	ev.playerid = pnum;
	ev._eData = reinterpret_cast<BYTE*>(&leaveinfo);
	ev.databytes = sizeof(leaveinfo);
	run_event_handler(ev);
	if (pnum < MAX_PLRS) {
		connected_table[pnum] = false;
		disconnect_net(pnum);
		clear_msg(pnum);
	}
}

void base::recv_disconnect(packet &pkt)
{
	plr_t pkt_plr = pkt.pktDisconnectPlr();
	leaveinfo_t leaveinfo = pkt.pktDisconnectInfo();

	if (pkt_plr != plr_self) {
		if (pkt_plr < MAX_PLRS && connected_table[pkt_plr]) {
			disconnect_plr(pkt_plr, leaveinfo);
		} else if (pkt_plr == PLR_MASTER) {
			// server down
			for (pkt_plr = 0; pkt_plr < MAX_PLRS; pkt_plr++) {
				if (pkt_plr != plr_self && connected_table[pkt_plr]) {
					disconnect_plr(pkt_plr, leaveinfo);
				}
			}
			disconnect_plr(SNPLAYER_MASTER, leaveinfo);
		}
	} else {
		ABORT(); // we were dropped by the owner?!?
	}
}

void base::recv_local(packet &pkt)
{
	plr_t pkt_plr = pkt.pktSrc();
	if (pkt_plr < MAX_PLRS) {
		connected_table[pkt_plr] = true;
	}
	switch (pkt.pktType()) {
	case PT_MESSAGE:
		message_queue.emplace_back(pkt_plr, buffer_t(pkt.pktMessageBegin(), pkt.pktMessageEnd()));
		break;
	case PT_TURN:
		// TODO: validate pkt_plr if the server can not be trusted?
		//if (pkt_plr < MAX_PLRS) {
			turn_queue[pkt_plr].push_back(pkt.pktTurn());
		//}
		break;
	case PT_JOIN_ACCEPT:
		recv_accept(pkt);
		break;
	case PT_CONNECT:
		connected_table[pkt.pktConnectPlr()] = true; // this can probably be removed
		break;
	case PT_DISCONNECT:
		recv_disconnect(pkt);
		break;
	default:
		break;
		// otherwise drop
	}
}

bool base::SNetReceiveMessage(int* sender, BYTE** data, unsigned* size)
{
	poll();
	if (message_queue.empty())
		return false;
	message_last = message_queue.front();
	message_queue.pop_front();
	*sender = message_last.sender;
	*size = message_last.payload.size();
	*data = message_last.payload.data();
	return true;
}

void base::SNetSendMessage(int receiver, const BYTE* data, unsigned size)
{
	if (receiver == SNPLAYER_ALL || receiver == plr_self) {
		message_queue.emplace_back(plr_self, buffer_t(data, data + size));
		if (receiver == plr_self)
			return;
	}
	plr_t dest;
	if (receiver == SNPLAYER_ALL/* || receiver == SNPLAYER_OTHERS*/)
		dest = PLR_BROADCAST;
	else {
		assert((unsigned)receiver < MAX_PLRS);
		dest = receiver;
	}
	auto pkt = pktfty.make_out_packet<PT_MESSAGE>(plr_self, dest, data, size);
	send_packet(*pkt);
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
	auto pkt = pktfty.make_out_packet<PT_TURN>(plr_self, PLR_BROADCAST, turn);
	send_packet(*pkt);
	turn_queue[plr_self].push_back(pkt->pktTurn());
}

/*void base::SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	//caps->size = 0;                  // unused
	caps->flags = 0;                 // unused
	caps->maxmessagesize = MAX_NETMSG_SIZE; // the largest message to send during delta-load
	caps->maxqueuesize = 0;          // unused
	caps->maxplayers = MAX_PLRS;     // unused (part of SNetGameData)
	caps->bytessec = 1000000;        // estimated speed of the connection (to determine if wait is necessary during delta load)
	caps->latencyms = 0;             // unused
	caps->defaultturnssec = 10;      // number of turns to process in a second (to determine the net-update-rate)
	caps->defaultturnsintransit = 1; // maximum acceptable number of turns in queue?
}*/

void base::SNetUnregisterEventHandler(int evtype)
{
	registered_handlers[evtype] = NULL;
}

/*
 *  EVENT_TYPE_PLAYER_LEAVE_GAME:
 *    handled by the engine, raised when a player leaves the game
 *  EVENT_TYPE_JOIN_ACCEPTED:
 *    handled by UI, raised during SNetCreateGame/SNetJoinGame
 *  EVENT_TYPE_PLAYER_MESSAGE:
 *    not implemented
 */
void base::SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	registered_handlers[evtype] = func;
}

void base::SNetLeaveGame(int reason)
{
	int i;

	auto pkt = pktfty.make_out_packet<PT_DISCONNECT>(plr_self, PLR_BROADCAST,
	    plr_self, (leaveinfo_t)reason);
	send_packet(*pkt);

	message_queue.clear();
	for (i = 0; i < MAX_PLRS; i++)
		turn_queue[i].clear();
}

void base::SNetDropPlayer(int playerid)
{
	auto pkt = pktfty.make_out_packet<PT_DISCONNECT>(plr_self,
	    PLR_BROADCAST,
	    (plr_t)playerid,
	    (leaveinfo_t)LEAVE_DROP);
	send_packet(*pkt);
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
