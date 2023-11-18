#include "base_client.h"

#include <cstring>
#include <memory>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

void base_client::setup_gameinfo(_uigamedata* gameData)
{
	SNetGameData* netData;

	game_init_info = buffer_t(sizeof(SNetGameData));

	netData = (SNetGameData*)game_init_info.data();
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

void base_client::run_event_handler(SNetEvent& ev)
{
	auto f = registered_handlers[ev.eventid];
	if (f != NULL) {
		f(&ev);
	}
}

void base_client::disconnect_net(plr_t pnum)
{
}

void base_client::recv_connect(packet& pkt)
{
	plr_t pkt_src = pkt.pktSrc();

	if (pkt_src < MAX_PLRS)
		connected_table[pkt_src] = CON_CONNECTED;
}

void base_client::recv_accept(packet& pkt)
{
	if (plr_self != PLR_BROADCAST || pkt.pktJoinAccCookie() != cookie_self) {
		// ignore the packet if player id is set or the cookie does not match
		return;
	}
	plr_self = pkt.pktJoinAccPlr();
	if (plr_self >= MAX_PLRS) {
		// Invalid player id -> ignore
		DoLog("Invalid player id (%d) received from %d.", NULL, 0, plr_self, pkt.pktSrc());
		plr_self = PLR_BROADCAST;
		return;
	}
	auto& pkt_info = pkt.pktJoinAccInfo();
	if (GAME_VERSION != pkt_info.ngVersionId) {
		// Invalid game version -> ignore
		DoLog("Invalid game version (%d) received from %d. (current version: %d)", NULL, 0, pkt_info.ngVersionId, pkt.pktSrc(), GAME_VERSION);
		plr_self = PLR_BROADCAST;
		return;
	}
	plr_t pmask = pkt.pktJoinAccMsk();
	// assert(pmask & (1 << plr_self));
	for (int i = 0; i < MAX_PLRS; i++) {
		if (pmask & (1 << i)) {
			connected_table[i] = CON_CONNECTED;
		}
	}
#ifdef ZEROTIER
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

void base_client::disconnect_plr(plr_t pnum)
{
	SNetEvent ev;

	ev.eventid = EVENT_TYPE_PLAYER_LEAVE_GAME;
	ev.playerid = pnum;
	ev.databytes = 0;

	run_event_handler(ev);

	if (pnum < MAX_PLRS) {
		connected_table[pnum] |= CON_LEAVING;
		turn_queue[pnum].emplace_back(0, buffer_t());
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

	switch (pkt.pktType()) {
	case PT_MESSAGE:
		net_assert(pkt_plr < MAX_PLRS || pkt_plr == SNPLAYER_MASTER);
		message_queue.emplace_back(pkt_plr, buffer_t(pkt.pktMessageBegin(), pkt.pktMessageEnd()));
		break;
	case PT_TURN:
		net_assert(pkt_plr < MAX_PLRS);
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
	default:
		break;
		// otherwise drop
	}
}

bool base_client::SNetReceiveMessage(int* sender, BYTE** data, unsigned* size)
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
	uint32_t turn = 0;

	dwLen = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (status[i] & PCS_TURN_ARRIVED) {
			pt = &turn_queue[i].front();
			//       pnum           size
			dwLen += sizeof(BYTE) + sizeof(unsigned);
			if (pt->turn_id != 0) {
				turn = pt->turn_id;
				dwLen += pt->payload.size();
			} else if ((connected_table[i] & CON_LEAVING) && turn_queue[i].size() == 1) {
				dwLen += LEAVING_TURN_SIZE;
			}
		}
	}
	pkt = (SNetTurnPkt*)DiabloAllocPtr(dwLen + sizeof(SNetTurnPkt) - sizeof(pkt->data));
	pkt->nmpTurn = turn;
	pkt->nmpLen = dwLen;
	data = pkt->data;
	for (i = 0; i < MAX_PLRS; i++) {
		if (status[i] & PCS_TURN_ARRIVED) {
			*data = i;
			data++;
			pt = &turn_queue[i].front();
			if (pt->turn_id != 0) {
				dwLen = pt->payload.size();
				*(unsigned*)data = dwLen;
				data += sizeof(unsigned);
				memcpy(data, pt->payload.data(), dwLen);
				data += dwLen;
			} else if ((connected_table[i] & CON_LEAVING) && turn_queue[i].size() == 1) {
				// assert(pt->payload.size() == 0);
				*(unsigned*)data = LEAVING_TURN_SIZE;
				data += sizeof(unsigned);
				TurnPkt* leaveCmdTurn = (TurnPkt*)data;
				leaveCmdTurn->hdr.wLen = (uint16_t)LEAVING_TURN_SIZE;
				leaveCmdTurn->body[0] = CMD_DISCONNECT;
				data += LEAVING_TURN_SIZE;
			} else {
				*(unsigned*)data = 0;
				data += sizeof(unsigned);
			}
			turn_queue[i].pop_front();
		}
	}
	return pkt;
}

void base_client::SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size)
{
	turn_queue[plr_self].emplace_back(SwapLE32(turn), buffer_t(data, data + size));
	static_assert(sizeof(turn_t) == sizeof(uint32_t), "SNetSendTurn: sizemismatch between turn_t and turn");
	packet* pkt = pktfty.make_out_packet<PT_TURN>(plr_self, PLR_BROADCAST, turn, data, size);
	send_packet(*pkt);
	delete pkt;
}

turn_status base_client::SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
	constexpr int SPLIT_LIMIT = 100; // the number of turns after the desync is unresolvable
	turn_status result;
	turn_t myturn, minturn, turn;
	int i, j;

	poll();
	memset(status, 0, sizeof(status));
	// TODO: do not assume plr_self has a turn?
	assert(!turn_queue[plr_self].empty());
	myturn = SwapLE32(turn_queue[plr_self].front().turn_id);
	status[plr_self] = (myturn != 0 ? 0 : PCS_JOINED) | PCS_CONNECTED | PCS_ACTIVE | PCS_TURN_ARRIVED;
	result = TS_ACTIVE; // or TS_LIVE
	for (i = 0; i < MAX_PLRS; i++) {
		if (i == plr_self || !connected_table[i])
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
		turn = SwapLE32(turn_queue[i].front().turn_id);
		if (turn == myturn) {
			continue;
		}
		if (turn < myturn) {
			// drop obsolete turns (except initial turns)
			if (turn == 0) {
				status[i] |= PCS_JOINED;
			} else {
				// TODO: report the drop (if !payload.empty())
				turn_queue[i].pop_front();
				status[i] = 0;
				i--;
			}
			continue;
		}
		if (result == TS_ACTIVE) // or TS_LIVE
			result = TS_DESYNC;
	}
	if (result == TS_DESYNC) {
		if (myturn == 0)
			result = TS_ACTIVE; // or TS_LIVE
		// find the highest turn
		// TODO: prevent non-host players from running forward?
		minturn = 0;
		for (i = 0; i < MAX_PLRS; i++) {
			if (!connected_table[i])
				continue;
			turn = SwapLE32(turn_queue[i].front().turn_id);
			if (turn > minturn)
				minturn = turn;
		}
		// find the lowest turn which is in SPLIT_LIMIT distance from the highest one
		for (i = 0; i < MAX_PLRS; i++) {
			if (!connected_table[i])
				continue;
			turn = SwapLE32(turn_queue[i].front().turn_id);
			if (turn == 0 || minturn == turn)
				continue;
			result = TS_DESYNC;
			if (minturn > turn && (minturn - turn < SPLIT_LIMIT)) {
				minturn = turn;
				for (j = 0; j < i; j++)
					if (!(status[j] & PCS_JOINED))
						status[j] &= ~PCS_TURN_ARRIVED;
			} else {
				status[i] &= ~PCS_TURN_ARRIVED;
			}
		}
	}
	return result;
}

uint32_t base_client::SNetLastTurn(unsigned (&status)[MAX_PLRS])
{
	int i;
	turn_t minturn = 0, turn;

	for (i = 0; i < MAX_PLRS; i++) {
		if (status[i] & PCS_TURN_ARRIVED) {
			turn = SwapLE32(turn_queue[i].front().turn_id);
			if (turn != 0) {
				minturn = turn;
				break;
			}
		}
	}
	if (!(status[plr_self] & PCS_TURN_ARRIVED))
		// TODO: report the drop (if !payload.empty())
		turn_queue[plr_self].clear();
	return minturn;
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

	message_last.payload.clear();
	message_queue.clear();
	for (i = 0; i < MAX_PLRS; i++)
		turn_queue[i].clear();
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
