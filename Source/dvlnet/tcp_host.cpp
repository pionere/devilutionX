#include "tcp_host.h"
#ifndef NOHOSTING

DEVILUTION_BEGIN_NAMESPACE
namespace net {

tcp_host_server::tcp_host_server(base_client& client, asio::io_context& ioc, packet_factory& pktfty, SNetGameData& gameinfo, unsigned srvType)
    : tcp_server(client, ioc, pktfty, gameinfo, srvType)
{
}

bool tcp_host_server::send_packet(packet& pkt)
{
	if (!tcp_server::send_packet(pkt)) {
		return false;
	}
	local_client.recv_local(pkt);
	return true;
}

tcp_host_client::tcp_host_client(int srvType)
{
	serverType = srvType;
}

bool tcp_host_client::setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	setup_password(passwd);
	assert(gameData != NULL);
	setup_gameinfo(gameData);
	local_server = new tcp_host_server(*this, ioc, pktfty, game_init_info, serverType);
	if (local_server->setup_server(addrstr, port, errorText)) {
		plr_self = PLR_MASTER;
		return true;
	}
	close();
	return false;
}

void tcp_host_client::poll()
{
	asio::error_code err;
	ioc.poll(err);
	assert(!err);
}

void tcp_host_client::SNetSendMessage(int receiver, const BYTE* data, unsigned size)
{
	plr_t dest;
	if (receiver == SNPLAYER_ALL /* || receiver == SNPLAYER_OTHERS*/) {
		dest = PLR_BROADCAST;
	} else {
		assert((unsigned)receiver < MAX_PLRS);
		dest = receiver;
	}
	// assert(dest != plr_self);
	packet* pkt = pktfty.make_out_packet<PT_MESSAGE>(plr_self, dest, data, size);
	send_packet(*pkt);
	delete pkt;
}

void tcp_host_client::send_packet(packet& pkt)
{
	local_server->send_packet(pkt);
}

SNetTurnPkt* tcp_host_client::SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	plr_t i = 0;

	while (true) {
		if (status[i] & PCS_TURN_ARRIVED) {
			break;
		}
		if (++i < MAX_PLRS) {
			continue;
		}
		// no active turn
		lastRecvTurn++;

		SNetTurnPkt* result = (SNetTurnPkt*)DiabloAllocPtr(0 + sizeof(SNetTurnPkt) - sizeof(result->data));
		result->ntpLen = 0;
		result->ntpTurn = lastRecvTurn;
		return result;
	}

	return base_client::SNetReceiveTurn(status);
}

void tcp_host_client::SNetSendTurn(turn_t turn, const BYTE* data, unsigned size)
{
}

turn_status tcp_host_client::SNetPollTurns(unsigned (&status)[MAX_PLRS])
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
			// TODO: report the drop (if !payload.empty())
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

unsigned tcp_host_client::SNetGetTurnsInTransit()
{
	assert(0);
	return 0;
}

void tcp_host_client::close()
{
	// close the server
	if (local_server != NULL) {
		local_server->close();
		delete local_server;
		local_server = NULL;
	}

	base_client::close();

	// prepare the host for possible re-connection
	ioc.restart();
}

void tcp_host_client::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	tcp_server::make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // NOHOSTING
