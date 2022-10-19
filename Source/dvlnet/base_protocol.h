#pragma once
#ifdef ZEROTIER
#include <string>
#include <map>
#include <memory>

#include "base.h"
#include "packet.h"
#include "utils/stubs.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

template <class P>
class base_protocol : public base {
public:
	virtual bool create_game(const char* addrstr, unsigned port, const char* passwd, SNetGameData* gameData, char (&errorText)[256]);
	virtual bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]);

	virtual void SNetLeaveGame(int reason);

	virtual void make_default_gamename(char (&gamename)[128]);
	virtual void send_info_request();
	virtual std::vector<std::string> get_gamelist();

	virtual ~base_protocol() = default;
protected:
	virtual void poll();
	virtual void send_packet(packet &pkt);
	virtual void disconnect_net(plr_t pnum);
private:
	P proto;
	typedef typename P::endpoint endpoint;

	endpoint firstpeer;
	std::string gamename;
	std::map<std::string, endpoint> game_list;
	std::array<endpoint, MAX_PLRS> peers;

	plr_t get_master();
	void recv();
	void handle_join_request(packet &pkt, endpoint sender);
	void recv_decrypted(packet &pkt, endpoint sender);

	bool wait_network();
	bool wait_firstpeer();
	void wait_join();
};

template <class P>
plr_t base_protocol<P>::get_master()
{
	for (plr_t i = 0; i < MAX_PLRS; ++i)
		if (peers[i])
			return i;
	return plr_self;
}

template <class P>
bool base_protocol<P>::wait_network()
{
	// wait for ZeroTier for 5 seconds
	for (auto i = 0; i < 500; ++i) {
		SDL_Delay(10);
		if (proto.network_online())
			return true;
	}
	return false;
}

template <class P>
void base_protocol<P>::disconnect_net(plr_t pnum)
{
	proto.disconnect(peers[pnum]);
	peers[pnum] = endpoint();
}

template <class P>
bool base_protocol<P>::wait_firstpeer()
{
	// wait for peer for 5 seconds
	for (auto i = 0; i < 500; ++i) {
		if (game_list.count(gamename)) {
			firstpeer = game_list[gamename];
			break;
		}
		send_info_request();
		recv();
		SDL_Delay(10);
	}
	return (bool)firstpeer;
}

template <class P>
void base_protocol<P>::send_info_request()
{
	auto pkt = pktfty.make_out_packet<PT_INFO_REQUEST>(PLR_BROADCAST,
	    PLR_MASTER);
	proto.send_oob_mc(pkt->encrypted_data());
}

template <class P>
void base_protocol<P>::wait_join()
{
	randombytes_buf(reinterpret_cast<unsigned char *>(&cookie_self),
	    sizeof(cookie_t));
	auto pkt = pktfty.make_out_packet<PT_JOIN_REQUEST>(PLR_BROADCAST,
	    PLR_MASTER, cookie_self);
	proto.send(firstpeer, pkt->encrypted_data());
	for (auto i = 0; i < 500; ++i) {
		recv();
		if (plr_self != PLR_BROADCAST)
			break; // join successful
		SDL_Delay(10);
	}
}

template <class P>
bool base_protocol<P>::create_game(const char* addrstr, unsigned port, const char* passwd, SNetGameData* gameData, char (&errorText)[256])
{
	setup_gameinfo(gameData);
	// join_game
	setup_password(passwd);
	gamename = std::string(addrstr);

	plr_self = PLR_BROADCAST;
	if (wait_network()) {
		plr_self = 0;
		connected_table[plr_self] = true;
		return true;
	}
	snprintf(errorText, 256, "Connection timed out.");
	return false;
}

template <class P>
bool base_protocol<P>::join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	//addrstr = "fd80:56c2:e21c:0:199:931d:b14:c4d2";
	plr_self = PLR_BROADCAST;
	setup_password(passwd);
	gamename = std::string(addrstr);
	if (wait_network() && wait_firstpeer()) {
		wait_join();
		if (plr_self != PLR_BROADCAST) {
			return true;
		}
	}
	snprintf(errorText, 256, "Connection timed out.");
	return false;
}

template <class P>
void base_protocol<P>::poll()
{
	recv();
}

template <class P>
void base_protocol<P>::send_packet(packet &pkt)
{
	plr_t pkt_plr = pkt.pktDest();

	if (pkt_plr < MAX_PLRS) {
		if (pkt_plr == mypnum)
			return;
		if (peers[pkt_plr])
			proto.send(peers[pkt_plr], pkt.encrypted_data());
	} else if (pkt_plr == PLR_BROADCAST) {
		for (auto &peer : peers)
			if (peer)
				proto.send(peer, pkt.encrypted_data());
	} else if (pkt_plr == PLR_MASTER) {
		throw dvlnet_exception();
	} else {
		throw dvlnet_exception();
	}
}

template <class P>
void base_protocol<P>::recv()
{
	try {
		buffer_t pkt_buf;
		endpoint sender;
		while (proto.recv(sender, pkt_buf)) { // read until kernel buffer is empty?
			auto pkt = pktfty.make_in_packet(pkt_buf);
			if (pkt != NULL)
				recv_decrypted(*pkt, sender);
			else // drop invalid packet
				proto.disconnect(sender);
		}
		while (proto.get_disconnected(sender)) {
			for (plr_t i = 0; i < MAX_PLRS; ++i) {
				if (peers[i] == sender) {
					disconnect_net(i);
					break;
				}
			}
		}
	} catch (std::exception &e) {
		DoLog(e.what());
		return;
	}
}

template <class P>
void base_protocol<P>::handle_join_request(packet &pkt, endpoint sender)
{
	plr_t i;
	for (i = 0; i < MAX_PLRS; ++i) {
		if (i != plr_self && !peers[i]) {
			peers[i] = sender;
			break;
		}
	}
	if (i >= MAX_PLRS) {
		//already full
		return;
	}
	for (plr_t j = 0; j < MAX_PLRS; ++j) {
		if ((j != plr_self) && (j != i) && peers[j]) {
			auto infopkt = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST, j, peers[j].serialize());
			proto.send(sender, infopkt->encrypted_data());
		}
	}
	auto reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(plr_self, PLR_BROADCAST,
	    pkt.pktJoinReqCookie(), i,
	    game_init_info);
	proto.send(sender, reply->encrypted_data());
}

template <class P>
void base_protocol<P>::recv_decrypted(packet &pkt, endpoint sender)
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
				buffer_t buf;
				buf.resize(gamename.size());
				std::memcpy(buf.data(), &gamename[0], gamename.size());
				auto reply = pktfty.make_out_packet<PT_INFO_REPLY>(PLR_BROADCAST,
				    PLR_MASTER,
				    buf);
				proto.send_oob(sender, reply->encrypted_data());
			}
		}
		return;
	} else if (pkt_plr == PLR_MASTER && pkt.pktType() == PT_CONNECT) {
		// addrinfo packets
		pkt_plr = pkt.pktConnectPlr();
		connected_table[pkt_plr] = true;
		//.unserialize(pkt.info());
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
	connected_table[pkt_plr] = true;
	pkt_plr = pkt.pktDest();
	if (pkt_plr != plr_self && pkt_plr != PLR_BROADCAST)
		return; // packet not for us, drop
	recv_local(pkt);
}

template <class P>
std::vector<std::string> base_protocol<P>::get_gamelist()
{
	recv();
	std::vector<std::string> ret;
	for (auto &s : game_list) {
		ret.push_back(s.first);
	}
	return ret;
}

template <class P>
void base_protocol<P>::SNetLeaveGame(int reason)
{
	base::SNetLeaveGame(reason);
	recv();
}

template <class P>
void base_protocol<P>::make_default_gamename(char (&gamename)[128])
{
	proto.make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif