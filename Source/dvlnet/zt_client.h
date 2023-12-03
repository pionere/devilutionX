#pragma once
#ifdef ZEROTIER
#include <string>
#include <map>
#include <memory>

#include "base_client.h"
#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

template <class P>
class zt_client : public base_client {
public:
	bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;
	void get_gamelist(std::vector<SNetZtGame>& games);
	bool network_ready();

	virtual ~zt_client() = default;

protected:
	void poll() override;
	void send_packet(packet& pkt) override;
	bool recv_connect(packet& pkt) override;
	bool recv_accept(packet& pkt) override;
	void disconnect_net(plr_t pnum) override;
	void close() override;

private:
	P proto;
	typedef typename P::endpoint endpoint;
	typedef std::array<char, NET_MAX_GAMENAME_LEN + 1> gamename_t;
	typedef struct game_details {
		SNetZtGame ztGamedata;
		endpoint master;
		Uint32 timeout;
	} game_details;

	gamename_t gamename;
	std::map<gamename_t, game_details> game_list;

	plr_t get_master();
	void disconnect_peer(const endpoint& peer);
	void send_info_request();
	void handle_join_request(packet& pkt, const endpoint& sender);
	void handle_recv_packet(packet& pkt, const endpoint& sender);
	void recv_ctrl(packet& pkt, const endpoint& sender);

	bool wait_network();
	bool wait_join();
	bool wait_firstpeer(endpoint& peer);
};

template <class P>
plr_t zt_client<P>::get_master()
{
	for (plr_t i = 0; i < MAX_PLRS; i++)
		if (proto.active_connections[i].status != CS_INACTIVE) {
			// assert(proto.active_connections[i].peer);
			return i;
		}
	return plr_self;
}

template <class P>
void zt_client<P>::disconnect_peer(const endpoint& peer)
{
	for (plr_t i = 0; i < MAX_PLRS; i++) {
		if (proto.active_connections[i].peer == peer) {
			proto.disconnect(i);
		}
	}
}

template <class P>
bool zt_client<P>::wait_network()
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

template <class P>
void zt_client<P>::disconnect_net(plr_t pnum)
{
	proto.disconnect(pnum);
}

template <class P>
bool zt_client<P>::wait_firstpeer(endpoint& peer)
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

template <class P>
void zt_client<P>::send_info_request()
{
	packet* pkt = pktfty.make_out_packet<PT_INFO_REQUEST>(PLR_BROADCAST, PLR_MASTER);
	proto.send_oob_mc(pkt->encrypted_data());
	delete pkt;
}

template <class P>
bool zt_client<P>::wait_join()
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

template <class P>
bool zt_client<P>::setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
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

template <class P>
void zt_client<P>::send_packet(packet& pkt)
{
	plr_t dest = pkt.pktDest();
	plr_t src = pkt.pktSrc();

	if (dest == PLR_BROADCAST) {
		for (plr_t i = 0; i < MAX_PLRS; i++)
			if (i != src && proto.active_connections[i].status != CS_INACTIVE) {
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

template <class P>
bool zt_client<P>::recv_connect(packet& pkt)
{ 
	if (!base_client::recv_connect(pkt))
		return false;

	plr_t pnum = pkt.pktConnectPlr();
	// assert(!proto.active_connections[pnum].peer)

	auto sit = pkt.pktConnectAddrBegin();
	if (pkt.pktConnectAddrEnd() - sit == proto.active_connections[pnum].peer.addr.size()) {
		proto.connect_ep(&*sit, pnum);
	}
	return true;
}

template <class P>
bool zt_client<P>::recv_accept(packet& pkt)
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
		if (it - sit == proto.active_connections[pnum].peer.addr.size()) {
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

template <class P>
void zt_client<P>::poll()
{
	buffer_t pkt_buf;
	endpoint sender;
	while (proto.recv(sender, pkt_buf)) { // read until kernel buffer is empty?
		packet* pkt = pktfty.make_in_packet(pkt_buf);
		if (pkt != NULL)
			handle_recv_packet(*pkt, sender);
		else
			disconnect_peer(sender);
		delete pkt;
	}
	while (proto.get_disconnected(sender)) {
		disconnect_peer(sender);
	}
}

template <class P>
void zt_client<P>::handle_join_request(packet& pkt, const endpoint& sender)
{
	plr_t i, pnum, pmask;
	packet* reply;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (pnum != plr_self && proto.active_connections[pnum].status == CS_INACTIVE) {
			// assert(!proto.active_connections[pnum].peer);
			break;
		}
	}
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
	reply = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST, pnum, conTurn, (const BYTE*)sender.addr.data(), (unsigned)sender.addr.size());
	send_packet(*reply);
	delete reply;
}

template <class P>
void zt_client<P>::recv_ctrl(packet& pkt, const endpoint& sender)
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
	}
}

template <class P>
void zt_client<P>::handle_recv_packet(packet& pkt, const endpoint& sender)
{
	plr_t src = pkt.pktSrc();

	if (src == PLR_BROADCAST) {
		recv_ctrl(pkt, sender);
		return;
	}
	if (plr_self == PLR_BROADCAST) {
		if (pkt.pktType() != PT_JOIN_ACCEPT)
			return; // non-global packet and we are not in game -> drop
		if (game_list[gamename].master != sender)
			return; // join accept, but from an unknown sender -> drop
	} else {
		if (src == PLR_MASTER) {
			src = get_master();
		} else if (src >= MAX_PLRS) {
			return; // packet with invalid source -> drop
		}
		if (sender != proto.active_connections[src].peer) {
			return; // packet with mismatching source -> drop
		}
	}
	recv_local(pkt);
}

template <class P>
bool zt_client<P>::network_ready()
{
	return proto.network_online();
}

template <class P>
void zt_client<P>::get_gamelist(std::vector<SNetZtGame>& games)
{
	// assert(network_ready());
	poll();
	send_info_request();
	games.clear();
	Uint32 now = SDL_GetTicks();
	for (auto it = game_list.begin(); it != game_list.end(); ) {
		if (it->second.timeout < now) {
			it = game_list.erase(it);
		} else {
			games.push_back(it->second.ztGamedata);
			it++;
		}
	}
}

template <class P>
void zt_client<P>::close()
{
	// game_list.clear();
	proto.close();

	base_client::close();
}

template <class P>
void zt_client<P>::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	proto.make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
