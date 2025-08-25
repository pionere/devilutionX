#include "zt_client.h"
#ifdef ZEROTIER

DEVILUTION_BEGIN_NAMESPACE
namespace net {

plr_t zt_client::get_master()
{
	for (plr_t i = 0; i < MAX_PLRS; i++)
		if (proto.active_connections[i].status != CS_INACTIVE) {
			// assert(proto.active_connections[i].peer);
			return i;
		}
	return plr_self;
}

plr_t zt_client::next_free_conn()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (proto.active_connections[i].status == CS_INACTIVE && ghost_connections[i] == 0) {
			// assert(!proto.active_connections[i].peer);
			break;
		}
	}
	return i < game_init_info.ngMaxPlayers ? i : MAX_PLRS;
}

bool zt_client::wait_network()
{
	// wait for ZeroTier for 5 seconds
	for (int i = 500; ; ) {
		if (proto.network_online())
			return true;
		if (--i == 0)
			break;
		SDL_Delay(10);
	}
	return false;
}

void zt_client::disconnect_net(plr_t pnum)
{
	proto.disconnect(pnum);

	if (get_master() == plr_self) {
		ghost_connections[pnum] = SDL_GetTicks() + NET_TIMEOUT_GHOST * NET_TIMEOUT_BASE * 1000;
	}
}

bool zt_client::wait_firstpeer(endpoint& peer)
{
	send_info_request();

	// wait for peer for 2.5 seconds
	for (int i = 250; ; ) {
		poll();
		auto git = game_list.find(gamename);
		if (git != game_list.end()) {
			peer = git->second.master;
			return true;
		}
		if (--i == 0)
			break;
		SDL_Delay(10);
	}
	return false;
}

void zt_client::send_info_request()
{
	packet* pkt = pktfty.make_out_packet<PT_INFO_REQUEST>(PLR_BROADCAST, PLR_MASTER);
	proto.send_oob_mc(pkt->encrypted_data());
	delete pkt;
}

bool zt_client::wait_join()
{
	endpoint peer;
	if (!wait_firstpeer(peer))
		return false;

	randombytes_buf(reinterpret_cast<unsigned char*>(&cookie_self), sizeof(cookie_t));
	packet* pkt = pktfty.make_out_packet<PT_JOIN_REQUEST>(PLR_BROADCAST, PLR_MASTER, cookie_self);
	proto.send_oob(peer, pkt->encrypted_data());
	delete pkt;
	// wait for reply for 2.5 seconds
	for (int i = 250; ; ) {
		poll();
		if (plr_self != PLR_BROADCAST)
			return true; // join successful
		if (--i == 0)
			break;
		SDL_Delay(10);
	}
	return false;
}

bool zt_client::setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	bool createGame;

	setup_password(passwd);

	createGame = gameData != NULL;
	if (createGame) {
		setup_gameinfo(gameData);
	}
	//addrstr = "fd80:56c2:e21c:0:199:931d:b14:c4d2";
	memcpy(gamename.data(), addrstr, gamename.size());
	if (wait_network()) {
		if (createGame) {
			plr_self = 0;
			connected_table[plr_self] = CON_CONNECTED;
			proto.accept_self(plr_self);
			return true;
		}
		// join game
		if (wait_join()) {
			return true;
		}
	}
	copy_cstr(errorText, "Unable to connect");
	close();
	return false;
}

void zt_client::send_packet(packet& pkt)
{
	plr_t dest = pkt.pktDest();
	plr_t src = pkt.pktSrc();

	if (dest == PLR_BROADCAST) {
		for (plr_t i = 0; i < MAX_PLRS; i++)
			if (i != src && proto.active_connections[i].status != CS_INACTIVE && proto.active_connections[i].status != CS_ACTIVE_SELF) {
				// assert(proto.active_connections[i].peer);
				proto.send(i, pkt.encrypted_data());
			}
	} else {
		if (dest >= MAX_PLRS) {
			// DoLog("Invalid destination %d", dest);
			return;
		}
		if ((dest != src) && proto.active_connections[dest].status != CS_INACTIVE) {
			// assert(proto.active_connections[dest].peer);
			proto.send(dest, pkt.encrypted_data());
		}
	}
}

bool zt_client::recv_connect(packet& pkt)
{ 
	if (!base_client::recv_connect(pkt))
		return false;

	plr_t pnum = pkt.pktConnectPlr();
	// assert(!proto.active_connections[pnum].peer)

	auto sit = pkt.pktConnectAddrBegin();
	if (pkt.pktConnectAddrEnd() - sit == endpoint::str_len()) {
		proto.connect_ep(&*sit, pnum);
	}
	return true;
}

bool zt_client::recv_accept(packet& pkt)
{
	if (!base_client::recv_accept(pkt)) {
		return false;
	}

	// assert(pkt.pktType() == PT_JOIN_ACCEPT);
	auto it = pkt.pktJoinAccAddrsBegin();
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		auto sit = it;
		while (true) {
			if (it == pkt.pktJoinAccAddrsEnd()) {
				return false; // bad format -> skip
			}
			if (*it == ' ') {
				break;
			}
			it++;
		}
		if (it - sit == endpoint::str_len()) {
			if (pnum != plr_self) {
				proto.accept_ep(&*sit, pnum);
			} else {
				proto.accept_self(pnum);
			}
		}
		it++;
	}
	return true;
}

void zt_client::poll()
{
	proto.poll(this);

	for (Uint32 &gc : ghost_connections) {
		if (gc != 0 && gc < SDL_GetTicks()) {
			gc = 0;
		}
	}
}

void zt_client::handle_recv_oob(buffer_t& data, const endpoint& sender)
{
	packet* pkt = pktfty.make_in_packet(data);
	if (pkt != NULL)
		recv_ctrl(*pkt, sender);
	delete pkt;
}

void zt_client::handle_recv(buffer_t& data, int pnum)
{
	packet* pkt = pktfty.make_in_packet(data);
	if (pkt != NULL)
		handle_recv_packet(*pkt, pnum);
	else
		proto.disconnect(pnum);
	delete pkt;
}

void zt_client::handle_join_request(packet& pkt, const endpoint& sender)
{
	plr_t i, pnum, pmask;
	packet* reply;

	pnum = next_free_conn();
	if (pnum >= MAX_PLRS) {
		// already full
		return;
	}
	proto.connect_ep(sender.addr.data(), pnum);
	turn_t conTurn = last_recv_turn() + NET_JOIN_WINDOW;
	// reply to the new player
	buffer_t addrs;
	pmask = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (proto.active_connections[i].status != CS_INACTIVE) {
			// assert(proto.active_connections[i].peer);
			static_assert(sizeof(pmask) * 8 >= MAX_PLRS, "handle_join_request can not send the active connections to the client.");
			pmask |= 1 << i;
			proto.active_connections[i].peer.to_buffer(addrs);
		}
		addrs.push_back(' ');
	}
	reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(plr_self, PLR_BROADCAST, pkt.pktJoinReqCookie(), pnum, (const BYTE*)&game_init_info, pmask, conTurn, (const BYTE*)addrs.data(), (unsigned)addrs.size());
	proto.send_oob(sender, reply->encrypted_data());
	delete reply;
	// notify the old players
	reply = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST, pnum, conTurn, (const BYTE*)sender.addr.data(), endpoint::str_len());
	send_packet(*reply);
	recv_local(*reply); // TODO: handle CS_ACTIVE_SELF in proto.send ?
	delete reply;
}

void zt_client::recv_ctrl(packet& pkt, const endpoint& sender)
{
	packet_type pkt_type = pkt.pktType();
	if (pkt_type == PT_INFO_REPLY) {
		const SNetZtGame& gameInfo = pkt.pktGameInfo();
		gamename_t gname;
		memcpy(gname.data(), &gameInfo.ngName[0], sizeof(gameInfo.ngName));
		auto git = game_list.find(gamename);
		if (git == game_list.end()) {
			git = game_list.insert({ gname, { gameInfo, sender, 0 } }).first;
		} else {
			git->second.ztGamedata = gameInfo;
			git->second.master = sender;
		}
		git->second.timeout = SDL_GetTicks() + NET_TIMEOUT_GAME * 1000;
	} else if (pkt_type == PT_JOIN_REQUEST) {
		if ((plr_self != PLR_BROADCAST) && (get_master() == plr_self)) {
			handle_join_request(pkt, sender);
		}
	} else if (pkt_type == PT_INFO_REQUEST) {
		if ((plr_self != PLR_BROADCAST) && (get_master() == plr_self)) {
			SNetZtGame gameInfo;
			assert(sizeof(gameInfo.ngName) == gamename.size());
			memcpy(&gameInfo.ngName[0], gamename.data(), sizeof(gameInfo.ngName));
			memcpy(&gameInfo.ngData, &game_init_info, sizeof(game_init_info));
			SNetPlrInfoEvent ev;
			ev.neHdr.eventid = EVENT_TYPE_PLAYER_INFO;
			ev.nePlayers = &gameInfo.ngPlayers[0];
			run_event_handler(&ev.neHdr);
			packet* reply = pktfty.make_out_packet<PT_INFO_REPLY>(PLR_BROADCAST, PLR_MASTER, (const BYTE*)&gameInfo);
			proto.send_oob(sender, reply->encrypted_data());
			delete reply;
		}
	} else if (pkt_type == PT_JOIN_ACCEPT) {
		if (plr_self == PLR_BROADCAST && game_list[gamename].master == sender) {
			recv_local(pkt);
		}
	}
}

void zt_client::handle_recv_packet(packet& pkt, int pnum)
{
	plr_t src = pkt.pktSrc();

	// assert(plr_self != PLR_BROADCAST);
	if (src == PLR_MASTER) {
		src = get_master();
	} else if (src >= MAX_PLRS) {
		return; // packet with invalid source -> drop
	}
	if (pnum != src) {
		return; // packet with mismatching source -> drop
	}
	recv_local(pkt);
}

bool zt_client::network_ready()
{
	return proto.network_online();
}

void zt_client::get_gamelist(std::vector<SNetZtGame>& games)
{
	// assert(network_ready());
	poll();
	send_info_request();
	games.clear();
	Uint32 now = SDL_GetTicks();
	for (auto it = game_list.begin(); it != game_list.end(); ) {
		if (SDL_TICKS_PASSED(now, it->second.timeout)) {
			it = game_list.erase(it);
		} else {
			games.push_back(it->second.ztGamedata);
			it++;
		}
	}
}

void zt_client::close()
{
	// game_list.clear();
	proto.close();

	memset(ghost_connections, 0, sizeof(ghost_connections));

	base_client::close();
}

void zt_client::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	proto.make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
