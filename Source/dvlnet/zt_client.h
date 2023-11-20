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
	virtual bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]);

	virtual void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]);
	virtual void get_gamelist(std::vector<std::string>& games);

	virtual ~zt_client() = default;

protected:
	void poll() override;
	void send_packet(packet& pkt) override;
	void disconnect_net(plr_t pnum) override;
	void close() override;

private:
	P proto;
	typedef typename P::endpoint endpoint;

	std::string gamename;
	std::map<std::string, endpoint> game_list;
	std::array<endpoint, MAX_PLRS> peers;

	plr_t get_master();
	void disconnect_peer(const endpoint& peer);
	void send_info_request();
	void handle_join_request(packet& pkt, endpoint sender);
	void recv_decrypted(packet& pkt, endpoint sender);

	bool wait_network();
	bool wait_join();
	bool wait_firstpeer(endpoint& peer);
};

template <class P>
plr_t zt_client<P>::get_master()
{
	for (plr_t i = 0; i < MAX_PLRS; i++)
		if (peers[i])
			return i;
	return plr_self;
}

template <class P>
void zt_client<P>::disconnect_peer(const endpoint& peer)
{
	proto.disconnect(peer);

	for (plr_t i = 0; i < MAX_PLRS; i++) {
		if (peers[i] == peer) {
			peers[i] = endpoint();
		}
	}
}

template <class P>
bool zt_client<P>::wait_network()
{
	// wait for ZeroTier for 5 seconds
	for (int i = 0; i < 500; ++i) {
		if (proto.network_online())
			return true;
		SDL_Delay(10);
	}
	return false;
}

template <class P>
void zt_client<P>::disconnect_net(plr_t pnum)
{
	disconnect_peer(peers[pnum]);
}

template <class P>
bool zt_client<P>::wait_firstpeer(endpoint& peer)
{
	// wait for peer for 5 seconds
	for (int i = 0; i < 500; i++) {
		poll();
		if (game_list.count(gamename)) {
			peer = game_list[gamename];
			return true;
		}
		send_info_request();
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
	proto.send(peer, pkt->encrypted_data());
	delete pkt;
	for (int i = 0; i < 500; ++i) {
		poll();
		if (plr_self != PLR_BROADCAST)
			return true; // join successful
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
	plr_self = PLR_BROADCAST;
	gamename = std::string(addrstr);
	if (wait_network()) {
		if (createGame) {
			plr_self = 0;
			connected_table[plr_self] = CON_CONNECTED;
			return true;
		}
		// join game
		if (wait_join()) {
			return true;
		}
	}
	snprintf(errorText, 256, "Connection timed out.");
	return false;
}

template <class P>
void zt_client<P>::send_packet(packet& pkt)
{
	plr_t dest = pkt.pktDest();
	plr_t src = pkt.pktSrc();

	if (dest == PLR_BROADCAST) {
		for (plr_t i = 0; i < MAX_PLRS; i++)
			if (i != src && peers[i] != NULL)
				proto.send(peers[i], pkt.encrypted_data());
	} else {
		if (dest >= MAX_PLRS) {
			// DoLog("Invalid destination %d", dest);
			return;
		}
		if ((dest != src) && peers[dest])
			proto.send(peers[dest], pkt.encrypted_data());
	}
}

template <class P>
void zt_client<P>::poll()
{
	buffer_t pkt_buf;
	endpoint sender;
	while (proto.recv(sender, pkt_buf)) { // read until kernel buffer is empty?
		packet* pkt = pktfty.make_in_packet(pkt_buf);
		if (pkt != NULL)
			recv_decrypted(*pkt, sender);
		else
			disconnect_peer(sender);
		delete pkt;
	}
	while (proto.get_disconnected(sender)) {
		for (plr_t i = 0; i < MAX_PLRS; i++) {
			if (peers[i] == sender) {
				disconnect_net(i);
				break;
			}
		}
	}
}

template <class P>
void zt_client<P>::handle_join_request(packet& pkt, endpoint sender)
{
	plr_t i, pnum, pmask;
	packet* reply;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (pnum != plr_self && !peers[pnum]) {
			peers[pnum] = sender;
			break;
		}
	}
	if (pnum >= MAX_PLRS) {
		//already full
		return;
	}
	// reply to the new player
	pmask = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (peers[i]) {
			static_assert(sizeof(pmask) * 8 >= MAX_PLRS, "handle_join_request can not send the active connections to the client.");
			pmask |= 1 << i;
		}
	}
	reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(plr_self, PLR_BROADCAST, pkt.pktJoinReqCookie(), pnum, game_init_info.data(), pmask);
	proto.send(sender, reply->encrypted_data());
	delete reply;
	// notify the old players
	reply = pktfty.make_out_packet<PT_CONNECT>(pnum, PLR_BROADCAST, PLR_MASTER, NULL, 0);
	send_packet(*reply);
	delete reply;
	// send the addresses of the old players to the new player            TODO: send with PT_JOIN_ACCEPT?
	pmask &= ~((1 << pnum) | (1 << plr_self));
	for (plr_t i = 0; i < MAX_PLRS; i++) {
		if (pmask & (1 << i)) {
			reply = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST, i, peers[i].addr.data(), peers[i].addr.size());
			proto.send(sender, reply->encrypted_data());
			delete reply;
		}
	}
}

template <class P>
void zt_client<P>::recv_decrypted(packet& pkt, endpoint sender)
{
	plr_t pkt_plr = pkt.pktSrc();

	if (pkt_plr == PLR_BROADCAST && pkt.pktDest() == PLR_MASTER) {
		packet_type pkt_type = pkt.pktType();
		if (pkt_type == PT_INFO_REPLY) {
			std::string pname(pkt.pktInfoReplyNameBegin(), pkt.pktInfoReplyNameEnd());
			game_list[pname] = sender;
		} else if (pkt_type == PT_JOIN_REQUEST) {
			handle_join_request(pkt, sender);
		} else if (pkt_type == PT_INFO_REQUEST) {
			if ((plr_self != PLR_BROADCAST) && (get_master() == plr_self)) {
				packet* reply = pktfty.make_out_packet<PT_INFO_REPLY>(PLR_BROADCAST, PLR_MASTER, gamename.c_str(), gamename.size());
				proto.send_oob(sender, reply->encrypted_data());
				delete reply;
			}
		}
		return;
	} else if (pkt_plr == PLR_MASTER && pkt.pktType() == PT_CONNECT) {
		// addrinfo packets
		pkt_plr = pkt.pktConnectPlr();
		connected_table[pkt_plr] |= CON_CONNECTED;
		auto addr = buffer_t(pkt.pktConnectAddrBegin(), pkt.pktConnectAddrEnd()));
		if (addr.size() == 16)
			memcpy(peers[pkt_plr].addr.data(), addr.data(), 16);
		return;
	} else if (pkt_plr >= MAX_PLRS) {
		return; // drop packet with invalid source
	} else if (peers[pkt_plr] != sender) {
		if (peers[pkt_plr])
			return; // drop packet with mismatching sender/source
		peers[pkt_plr] = sender;
	}
	connected_table[pkt_plr] |= CON_CONNECTED;
	pkt_plr = pkt.pktDest();
	if (pkt_plr != plr_self && pkt_plr != PLR_BROADCAST)
		return; // packet not for us, drop
	recv_local(pkt);
}

template <class P>
void zt_client<P>::get_gamelist(std::vector<std::string>& games)
{
	send_info_request();
	poll();
	for (auto& s : game_list) {
		games.push_back(s.first);
	}
}

template <class P>
void zt_client<P>::close()
{
	base_client::close();

	// proto.close();
}

template <class P>
void zt_client<P>::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	proto.make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
