#pragma once

#include <string>
#include <set>
#include <map>
#include <memory>

#include "dvlnet/base.h"
#include "dvlnet/packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

template <class P>
class base_protocol : public base {
public:
	virtual int create(std::string addrstr, std::string passwd);
	virtual int join(std::string addrstr, std::string passwd);
	virtual void poll();
	virtual void send(packet &pkt);
	virtual void disconnect_net(plr_t pnum);

	virtual void SNetLeaveGame(int type);

	virtual std::string make_default_gamename();
	virtual void send_info_request();
	virtual std::vector<std::string> get_gamelist();

	virtual ~base_protocol() = default;

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
	void recv_ingame(packet &pkt, endpoint sender);

	bool wait_network();
	bool wait_firstpeer();
	void wait_join();
};

template <class P>
plr_t base_protocol<P>::get_master()
{
	plr_t ret = plr_self;
	for (plr_t i = 0; i < MAX_PLRS; ++i)
		if (peers[i])
			ret = std::min(ret, i);
	return ret;
}

template <class P>
bool base_protocol<P>::wait_network()
{
	// wait for ZeroTier for 5 seconds
	for (auto i = 0; i < 500; ++i) {
		if (proto.network_online())
			break;
		SDL_Delay(10);
	}
	return proto.network_online();
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
	auto pkt = pktfty->make_out_packet<PT_INFO_REQUEST>(PLR_BROADCAST,
	    PLR_MASTER);
	proto.send_oob_mc(pkt->data());
}

template <class P>
void base_protocol<P>::wait_join()
{
	randombytes_buf(reinterpret_cast<unsigned char *>(&cookie_self),
	    sizeof(cookie_t));
	auto pkt = pktfty->make_out_packet<PT_JOIN_REQUEST>(PLR_BROADCAST,
	    PLR_MASTER, cookie_self, game_init_info);
	proto.send(firstpeer, pkt->data());
	for (auto i = 0; i < 500; ++i) {
		recv();
		if (plr_self != PLR_BROADCAST)
			break; // join successful
		SDL_Delay(10);
	}
}

template <class P>
int base_protocol<P>::create(std::string addrstr, std::string passwd)
{
	setup_password(passwd);
	gamename = addrstr;

	if (wait_network()) {
		plr_self = 0;
		connected_table[plr_self] = true;
	}

	return (plr_self == PLR_BROADCAST ? MAX_PLRS : plr_self);
}

template <class P>
int base_protocol<P>::join(std::string addrstr, std::string passwd)
{
	//addrstr = "fd80:56c2:e21c:0:199:931d:b14:c4d2";
	setup_password(passwd);
	gamename = addrstr;
	if (wait_network())
		if (wait_firstpeer())
			wait_join();
	return (plr_self == PLR_BROADCAST ? MAX_PLRS : plr_self);
}

template <class P>
void base_protocol<P>::poll()
{
	recv();
}

template <class P>
void base_protocol<P>::send(packet &pkt)
{
	plr_t pkt_plr = pkt.dest();

	if (pkt_plr < MAX_PLRS) {
		if (pkt_plr == mypnum)
			return;
		if (peers[pkt_plr])
			proto.send(peers[pkt_plr], pkt.data());
	} else if (pkt_plr == PLR_BROADCAST) {
		for (auto &peer : peers)
			if (peer)
				proto.send(peer, pkt.data());
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
			try {
				auto pkt = pktfty->make_in_packet(pkt_buf);
				recv_decrypted(*pkt, sender);
			} catch (packet_exception &e) {
				// drop packet
				proto.disconnect(sender);
				SDL_Log("%s", e.what());
			}
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
		SDL_Log("%s", e.what());
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
			auto infopkt = pktfty->make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST, j, peers[j].serialize());
			proto.send(sender, infopkt->data());
		}
	}
	auto reply = pktfty->make_out_packet<PT_JOIN_ACCEPT>(plr_self, PLR_BROADCAST,
	    pkt.cookie(), i,
	    game_init_info);
	proto.send(sender, reply->data());
}

template <class P>
void base_protocol<P>::recv_decrypted(packet &pkt, endpoint sender)
{
	if (pkt.src() == PLR_BROADCAST && pkt.dest() == PLR_MASTER && pkt.type() == PT_INFO_REPLY) {
		auto &pkt_info = pkt.info();
		std::string pname;
		pname.resize(pkt_info.size());
		std::memcpy(&pname[0], pkt_info.data(), pkt_info.size());
		game_list[pname] = sender;
		return;
	}
	recv_ingame(pkt, sender);
}

template <class P>
void base_protocol<P>::recv_ingame(packet &pkt, endpoint sender)
{
	plr_t pkt_plr = pkt.src();

	if (pkt_plr == PLR_BROADCAST && pkt.dest() == PLR_MASTER) {
		packet_type pkt_type = pkt.type();
		if (pkt_type == PT_JOIN_REQUEST) {
			handle_join_request(pkt, sender);
		} else if (pkt_type == PT_INFO_REQUEST) {
			if ((plr_self != PLR_BROADCAST) && (get_master() == plr_self)) {
				buffer_t buf;
				buf.resize(gamename.size());
				std::memcpy(buf.data(), &gamename[0], gamename.size());
				auto reply = pktfty->make_out_packet<PT_INFO_REPLY>(PLR_BROADCAST,
				    PLR_MASTER,
				    buf);
				proto.send_oob(sender, reply->data());
			}
		}
		return;
	} else if (pkt_plr == PLR_MASTER && pkt.type() == PT_CONNECT) {
		// addrinfo packets
		pkt_plr = pkt.newplr();
		connected_table[pkt_plr] = true;
		peers[pkt_plr].unserialize(pkt.info());
		return;
	} else if (pkt_plr >= MAX_PLRS) {
		// normal packets
		ABORT();
	}
	connected_table[pkt_plr] = true;
	peers[pkt_plr] = sender;
	pkt_plr = pkt.dest();
	if (pkt_plr != plr_self && pkt_plr != PLR_BROADCAST)
		return; //packet not for us, drop
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
std::string base_protocol<P>::make_default_gamename()
{
	return proto.make_default_gamename();
}

} // namespace net
DEVILUTION_END_NAMESPACE
