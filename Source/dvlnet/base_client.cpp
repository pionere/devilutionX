#include "base_client.h"
#ifndef NONET
#include <cstring>
#include <memory>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void base_client::setup_gameinfo(_uigamedata* gameData)
{
	SNetGameData* netData = &game_init_info;

	netData->ngVersionId = gameData->aeVersionId;
	netData->ngSeed = gameData->aeSeed;
	netData->ngDifficulty = gameData->aeDifficulty;
	netData->ngTickRate = gameData->aeTickRate;
	netData->ngNetUpdateRate = gameData->aeNetUpdateRate;
	netData->ngMaxPlayers = gameData->aeMaxPlayers;
}

void base_client::setup_password(const char* passwd)
{
	pktfty.setup_password(passwd);
}

void base_client::run_event_handler(SNetEventHdr* ev)
{
	auto f = registered_handlers[ev->eventid];
	if (f != NULL) {
		f(ev);
	}
}

void base_client::disconnect_net(plr_t pnum)
{
}

bool base_client::recv_connect(packet& pkt)
{
	if (pkt.pktSrc() != PLR_MASTER)
		return false;

	plr_t pnum = pkt.pktConnectPlr();
	turn_t conTurn = pkt.pktConnectTurn();

	if (pnum >= MAX_PLRS || pnum == plr_self)
		return false;

	if (connected_table[pnum])
		return false;
	// assert(pkt.pktType() == PT_CONNECT);
	connected_table[pnum] = CON_CONNECTED;
	assert(turn_queue[pnum].empty());

	unsigned limit = NET_JOIN_WINDOW * 2;
	for (turn_t turn = lastRecvTurn + 1; turn != conTurn + 1; turn++) {
		turn_queue[pnum].emplace_back(turn, buffer_t());
		if (--limit == 0) {
			break;
		}
	}
	return true;
}

bool base_client::recv_accept(packet& pkt)
{
	plr_t pnum, pmask;
	turn_t turn;
	// assert(pkt.pktType() == PT_JOIN_ACCEPT);
	if (plr_self != PLR_BROADCAST || pkt.pktJoinAccCookie() != cookie_self) {
		// ignore the packet if player id is set or the cookie does not match
		return false;
	}
	auto& pkt_info = pkt.pktJoinAccInfo();
	if (GAME_VERSION != pkt_info.ngVersionId) {
		// Invalid game version -> ignore
		DoLog("Invalid game version (%d) received from %d. (current version: %d)", NULL, 0, pkt_info.ngVersionId, pkt.pktSrc(), GAME_VERSION);
		return false;
	}
	pnum = pkt.pktJoinAccPlr();
	// assert(pnum < MAX_PLRS);
	pmask = pkt.pktJoinAccMsk();
	// assert(pmask & (1 << pnum));
	turn = pkt.pktJoinAccTurn();
	plr_self = pnum;
	for (int i = 0; i < MAX_PLRS; i++) {
		if (pmask & (1 << i)) {
			connected_table[i] = CON_CONNECTED;
		}
	}
	lastRecvTurn = turn;
#ifdef ZEROTIER
	// preserve game-info in case we become master later
	memcpy(&game_init_info, &pkt_info, sizeof(SNetGameData));
#endif
	SNetJoinEvent ev;
	ev.neHdr.eventid = EVENT_TYPE_JOIN_ACCEPTED;
	ev.neHdr.playerid = pnum;
	ev.neGameData = &pkt_info;
	ev.neTurn = turn;
	run_event_handler(&ev.neHdr);
	return true;
}

void base_client::disconnect_plr(plr_t pnum)
{
	SNetLeaveEvent ev;

	ev.neHdr.eventid = EVENT_TYPE_PLAYER_LEAVE_GAME;
	ev.neHdr.playerid = pnum;
	run_event_handler(&ev.neHdr);

	if (pnum < MAX_PLRS) {
		connected_table[pnum] |= CON_LEAVING;
		// add artificial disconnecting turn against misbehaving clients (TODO: could be out of sync...)
		BYTE disTurn[sizeof(TurnPktHdr) + 1];
		TurnPkt* pkt = (TurnPkt*)disTurn;
		pkt->hdr.wLen = (uint16_t)sizeof(disTurn);
		pkt->body[0] = CMD_DISCONNECT;
		turn_t plrLastTurn;
		if (!turn_queue[pnum].empty()) {
			plrLastTurn = turn_queue[pnum].back().turn_id + 1;
		} else {
			plrLastTurn = lastRecvTurn + 1;
		}
		turn_queue[pnum].emplace_back(plrLastTurn, buffer_t(&disTurn[0], &disTurn[0] + sizeof(disTurn)));
		disconnect_net(pnum);
	}
}

void base_client::recv_disconnect(packet& pkt)
{
	plr_t pkt_src = pkt.pktSrc();
	plr_t pkt_plr = pkt.pktDisconnectPlr();

	//if (pkt_plr == plr_self)
	//	return; // ignore self-disconnects of hosts
	if (pkt_plr != pkt_src && pkt_src != PLR_MASTER)
		return; // ignore other players attempt to disconnect each other/server
	if (pkt_plr < MAX_PLRS && connected_table[pkt_plr] == CON_CONNECTED) {
		disconnect_plr(pkt_plr);
	} else if (pkt_plr == PLR_MASTER) {
		// server down
		for (pkt_plr = 0; pkt_plr < MAX_PLRS; pkt_plr++) {
			if (pkt_plr != plr_self && connected_table[pkt_plr] == CON_CONNECTED) {
				disconnect_plr(pkt_plr);
			}
		}
		disconnect_plr(SNPLAYER_MASTER);
	}
}

void base_client::recv_local(packet& pkt)
{
	// FIXME: the server could still impersonate a player...
	plr_t pkt_plr = pkt.pktSrc();

	if (pkt_plr < MAX_PLRS && connected_table[pkt_plr] & CON_LEAVING)
		return;

	switch (pkt.pktType()) {
	case PT_MESSAGE:
		message_queue.emplace_back(pkt_plr, buffer_t(pkt.pktMessageBegin(), pkt.pktMessageEnd()));
		break;
	case PT_TURN:
		turn_queue[pkt_plr].emplace_back(pkt.pktTurn(), buffer_t(pkt.pktTurnBegin(), pkt.pktTurnEnd()));
		break;
	case PT_JOIN_ACCEPT:
		recv_accept(pkt);
		break;
	case PT_CONNECT:
		recv_connect(pkt);
		break;
	case PT_DISCONNECT:
		recv_disconnect(pkt);
		break;
	case PT_JOIN_REQUEST:
#ifdef ZEROTIER
	case PT_INFO_REQUEST:
	case PT_INFO_REPLY:
#endif
		break;
	default:
		ASSUME_UNREACHABLE;
		break;
	}
}

turn_t base_client::last_recv_turn() const
{
	return lastRecvTurn;
}

SNetMsgPkt* base_client::SNetReceiveMessage()
{
	poll();

	SNetMsgPkt* pkt = NULL;
	SNetMessage* pm;
	unsigned dwLen;

	if (message_queue.empty())
		return pkt;
	pm = &message_queue.front();

	dwLen = pm->payload.size();
	pkt = (SNetMsgPkt*)DiabloAllocPtr(dwLen + sizeof(SNetMsgPkt) - sizeof(pkt->data));
	pkt->nmpPlr = pm->sender;
	pkt->nmpLen = dwLen;
	memcpy(pkt->data, pm->payload.data(), dwLen);
	message_queue.pop_front();
	return pkt;
}

void base_client::SNetSendMessage(int receiver, const BYTE* data, unsigned size)
{
	if (receiver == SNPLAYER_ALL || receiver == plr_self) {
		message_queue.emplace_back(plr_self, buffer_t(data, data + size));
		if (receiver == plr_self)
			return;
	}
	plr_t dest;
	if (receiver == SNPLAYER_ALL /* || receiver == SNPLAYER_OTHERS*/) {
		dest = PLR_BROADCAST;
	} else {
		assert((unsigned)receiver < MAX_PLRS);
		dest = receiver;
	}
	packet* pkt = pktfty.make_out_packet<PT_MESSAGE>(plr_self, dest, data, size);
	send_packet(*pkt);
	delete pkt;
}

#define LEAVING_TURN_SIZE (sizeof(TurnPktHdr) + 1)
SNetTurnPkt* base_client::SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	SNetTurnPkt* pkt;
	SNetTurn* pt;
	int i;
	BYTE* data;
	unsigned dwLen;
	turn_t turn = 0;

	dwLen = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (status[i] & PCS_TURN_ARRIVED) {
			pt = &turn_queue[i].front();
			//       pnum           size
			dwLen += sizeof(BYTE) + sizeof(unsigned);
			turn = pt->turn_id;
			dwLen += pt->payload.size();
		}
	}
	lastRecvTurn = turn;
	pkt = (SNetTurnPkt*)DiabloAllocPtr(dwLen + sizeof(SNetTurnPkt) - sizeof(pkt->data));
	pkt->ntpTurn = turn;
	pkt->ntpLen = dwLen;
	data = pkt->data;
	for (i = 0; i < MAX_PLRS; i++) {
		if (status[i] & PCS_TURN_ARRIVED) {
			*data = i;
			data++;
			pt = &turn_queue[i].front();
			dwLen = pt->payload.size();
			*(unsigned*)data = dwLen;
			data += sizeof(unsigned);
			memcpy(data, pt->payload.data(), dwLen);
			data += dwLen;
			turn_queue[i].pop_front();
		}
	}
	return pkt;
}

void base_client::SNetSendTurn(turn_t turn, const BYTE* data, unsigned size)
{
	turn_queue[plr_self].emplace_back(turn, buffer_t(data, data + size));
	packet* pkt = pktfty.make_out_packet<PT_TURN>(plr_self, PLR_BROADCAST, turn, data, size);
	send_packet(*pkt);
	delete pkt;
}

turn_status base_client::SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
	turn_status result;
	turn_t myturn, turn;
	int i;

	poll();
	memset(status, 0, sizeof(status));
	myturn = lastRecvTurn + 1;
	result = TS_ACTIVE; // or TS_LIVE
	for (i = 0; i < MAX_PLRS; i++) {
		if (!connected_table[i])
			continue;
		if (turn_queue[i].empty()) {
			if (connected_table[i] & CON_LEAVING) {
				connected_table[i] = 0;
			} else {
				status[i] = PCS_CONNECTED;
				result = TS_TIMEOUT;
			}
			continue;
		}
		status[i] = PCS_CONNECTED | PCS_ACTIVE | PCS_TURN_ARRIVED;
		turn = turn_queue[i].front().turn_id;
		if (turn == myturn) {
			continue;
		}
		if (turn < myturn) {
			// drop obsolete turns
			// TODO: report the drop (if not in MSG_GAME_DELTA_WAIT)
			turn_queue[i].pop_front();
			status[i] = 0;
			i--;
			continue;
		}
		status[i] = PCS_CONNECTED | PCS_ACTIVE | PCS_DESYNC;
		if (result == TS_ACTIVE) // or TS_LIVE
			result = TS_DESYNC;
	}
	return result;
}

unsigned base_client::SNetGetTurnsInTransit()
{
	return turn_queue[plr_self].size();
}

/*void base_client::SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	//caps->size = 0;                  // unused
	caps->flags = 0;                 // unused
	caps->maxmessagesize = NET_LARGE_MSG_SIZE; // the largest message to send during delta-load
	caps->maxqueuesize = 0;          // unused
	caps->maxplayers = MAX_PLRS;     // unused (part of SNetGameData)
	caps->bytessec = 1000000;        // estimated speed of the connection (to determine if wait is necessary during delta load)
	caps->latencyms = 0;             // unused
	caps->defaultturnssec = 10;      // number of turns to process in a second (to determine the net-update-rate)
	caps->defaultturnsintransit = 1; // maximum acceptable number of turns in queue?
}*/

void base_client::SNetUnregisterEventHandler(int evtype)
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
void base_client::SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	registered_handlers[evtype] = func;
}

void base_client::close()
{
	int i;

	message_queue.clear();
	for (i = 0; i < MAX_PLRS; i++)
		turn_queue[i].clear();
	pktfty.clear_password();
	// prepare the client for possible re-connection
	lastRecvTurn = -1;
	plr_self = PLR_BROADCAST;
	memset(connected_table, 0, sizeof(connected_table));
}

void base_client::SNetLeaveGame()
{
	if (plr_self != PLR_MASTER) { // do not send disconnect in case of tcp_host (the server is going to send it)
		packet* pkt = pktfty.make_out_packet<PT_DISCONNECT>(plr_self, PLR_BROADCAST, plr_self);
		send_packet(*pkt);
		delete pkt;
	}

	poll();
	close();
}

void base_client::SNetDropPlayer(int playerid)
{
	packet* pkt = pktfty.make_out_packet<PT_DISCONNECT>(plr_self, PLR_BROADCAST, (plr_t)playerid);
	send_packet(*pkt);
	recv_local(*pkt);
	delete pkt;
}

void base_client::SNetDisconnect()
{
	packet* pkt;

	pkt = pktfty.make_out_packet<PT_DISCONNECT>(plr_self, PLR_BROADCAST, plr_self);
	send_packet(*pkt);
	delete pkt;

	pkt = pktfty.make_fake_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST, PLR_MASTER);
	recv_local(*pkt);
	delete pkt;
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // !NONET