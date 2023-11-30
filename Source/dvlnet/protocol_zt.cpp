#include "protocol_zt.h"
#ifdef ZEROTIER
#include <random>

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#else
#include "utils/sdl2_backports.h"
#endif

#include <lwip/igmp.h>
#include <lwip/mld6.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>

#include "dvlnet/zerotier_native.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

protocol_zt::protocol_zt()
{
	zerotier_network_start();
}

void protocol_zt::set_nonblock(int fd)
{
	static_assert(O_NONBLOCK == 1, "O_NONBLOCK == 1 not satisfied");
	auto mode = lwip_fcntl(fd, F_GETFL, 0);
	mode |= O_NONBLOCK;
	lwip_fcntl(fd, F_SETFL, mode);
}

void protocol_zt::set_nodelay(int fd)
{
	const int yes = 1;
	lwip_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&yes, sizeof(yes));
}

void protocol_zt::set_reuseaddr(int fd)
{
	const int yes = 1;
	lwip_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes));
}

bool protocol_zt::network_online()
{
	if (!zerotier_network_ready())
		return false;

	struct sockaddr_in6 in6 {
	};
	in6.sin6_port = htons(DEFAULT_PORT);
	in6.sin6_family = AF_INET6;
	in6.sin6_addr = in6addr_any;

	if (fd_udp == -1) {
		fd_udp = lwip_socket(AF_INET6, SOCK_DGRAM, 0);
		set_reuseaddr(fd_udp);
		auto ret = lwip_bind(fd_udp, (struct sockaddr*)&in6, sizeof(in6));
		if (ret < 0) {
			DoLog("lwip, (udp) bind: %s", strerror(errno));
			return false;
		}
		set_nonblock(fd_udp);
	}
	if (fd_tcp == -1) {
		fd_tcp = lwip_socket(AF_INET6, SOCK_STREAM, 0);
		set_reuseaddr(fd_tcp);
		auto r1 = lwip_bind(fd_tcp, (struct sockaddr*)&in6, sizeof(in6));
		if (r1 < 0) {
			DoLog("lwip, (tcp) bind: %s", strerror(errno));
			return false;
		}
		auto r2 = lwip_listen(fd_tcp, 10);
		if (r2 < 0) {
			DoLog("lwip, listen: %s", strerror(errno));
			return false;
		}
		set_nonblock(fd_tcp);
		set_nodelay(fd_tcp);
	}
	return true;
}

bool protocol_zt::send(const endpoint& peer, const buffer_t& data)
{
	peer_list[peer].send_queue.push_back(frame_queue::make_frame(data));
	return true;
}

bool protocol_zt::send_oob(const endpoint& peer, const buffer_t& data) const
{
	struct sockaddr_in6 in6 {
	};
	in6.sin6_port = htons(DEFAULT_PORT);
	in6.sin6_family = AF_INET6;
	peer.to_addr(reinterpret_cast<unsigned char*>(in6.sin6_addr.s6_addr));
	lwip_sendto(fd_udp, data.data(), data.size(), 0, (const struct sockaddr*)&in6, sizeof(in6));
	return true;
}

bool protocol_zt::send_oob_mc(const buffer_t& data) const
{
	endpoint mc;
	mc.from_addr(dvl_multicast_addr);
	return send_oob(mc, data);
}

bool protocol_zt::send_queued_peer(const endpoint& peer)
{
	peer_state& ps = peer_list[peer];
	if (ps.fd == -1) {
		ps.fd = lwip_socket(AF_INET6, SOCK_STREAM, 0);
		set_nodelay(ps.fd);
		set_nonblock(ps.fd);
		struct sockaddr_in6 in6 {
		};
		in6.sin6_port = htons(DEFAULT_PORT);
		in6.sin6_family = AF_INET6;
		peer.to_addr(reinterpret_cast<unsigned char*>(in6.sin6_addr.s6_addr));
		lwip_connect(ps.fd, (const struct sockaddr*)&in6, sizeof(in6));
	}
	while (!ps.send_queue.empty()) {
		buffer_t* frame = ps.send_queue.front();
		size_t len = frame->size();
		auto r = lwip_send(ps.fd, frame->data(), len, 0);
		if (r < 0) {
			// handle error
			return false;
		}
		if (decltype(len)(r) < len) {
			// partial send
			auto it = frame->begin();
			frame->erase(it, it + r);
			return true;
		}
		if (decltype(len)(r) == len) {
			delete frame;
			ps.send_queue.pop_front();
		} else {
			return false;
		}
	}
	return true;
}

bool protocol_zt::recv_peer(const endpoint& peer)
{
	BYTE buf[PKTBUF_LEN];
	peer_state& ps = peer_list[peer];
	while (true) {
		auto len = lwip_recv(ps.fd, buf, sizeof(buf), 0);
		if (len >= 0) {
			ps.recv_queue.write(buffer_t(buf, buf + len));
		} else {
			return errno == EAGAIN || errno == EWOULDBLOCK;
		}
	}
}

bool protocol_zt::send_queued_all()
{
	for (auto& peer : peer_list) {
		if (!send_queued_peer(peer.first)) {
			// handle error?
		}
	}
	return true;
}

bool protocol_zt::recv_from_peers()
{
	for (auto& peer : peer_list) {
		if (peer.second.fd != -1) {
			if (!recv_peer(peer.first)) {
				disconnect_queue.push_back(peer.first);
			}
		}
	}
	return true;
}

bool protocol_zt::recv_from_udp()
{
	BYTE buf[PKTBUF_LEN];
	struct sockaddr_in6 in6 {
	};
	socklen_t addrlen = sizeof(in6);
	auto len = lwip_recvfrom(fd_udp, buf, sizeof(buf), 0, (struct sockaddr*)&in6, &addrlen);
	if (len < 0)
		return false;
	buffer_t data(buf, buf + len);
	endpoint ep;
	ep.from_addr(reinterpret_cast<const unsigned char*>(in6.sin6_addr.s6_addr));
	oob_recv_queue.emplace_back(ep, std::move(data));
	return true;
}

bool protocol_zt::accept_all()
{
	struct sockaddr_in6 in6 {
	};
	socklen_t addrlen = sizeof(in6);
	while (true) {
		auto newfd = lwip_accept(fd_tcp, (struct sockaddr*)&in6, &addrlen);
		if (newfd < 0)
			break;
		endpoint ep;
		ep.from_addr(reinterpret_cast<const unsigned char*>(in6.sin6_addr.s6_addr));
		peer_state& ps = peer_list[ep];
		if (ps.fd != -1) {
			DoLog("protocol_zt::accept_all: WARNING: overwriting connection");
			lwip_close(ps.fd);
		}
		ps.fd = newfd;
		set_nonblock(newfd);
		set_nodelay(newfd);
	}
	return true;
}

bool protocol_zt::recv(endpoint& peer, buffer_t& data)
{
	accept_all();
	send_queued_all();
	recv_from_peers();
	recv_from_udp();

	if (!oob_recv_queue.empty()) {
		peer = oob_recv_queue.front().first;
		data = oob_recv_queue.front().second;
		oob_recv_queue.pop_front();
		return true;
	}

	for (auto& p : peer_list) {
		if (p.second.recv_queue.packet_ready()) {
			peer = p.first;
			data = p.second.recv_queue.read_packet();
			return true;
		}
	}
	return false;
}

bool protocol_zt::get_disconnected(endpoint& peer)
{
	if (!disconnect_queue.empty()) {
		peer = disconnect_queue.front();
		disconnect_queue.pop_front();
		return true;
	}
	return false;
}

void protocol_zt::disconnect(const endpoint& peer)
{
	auto it = peer_list.find(peer);
	if (it != peer_list.end()) {
		if (it->second.fd != -1) {
			if (lwip_close(it->second.fd) < 0) {
				DoLog("lwip_close: %s", strerror(errno));
			}
		}
		peer_list.erase(it);
	}
}

void protocol_zt::close()
{
	for (auto& peer : peer_list) {
		if (peer.second.fd != -1)
			lwip_close(peer.second.fd);
		for (auto frame : peer.second.send_queue) {
			delete frame;
		}
		peer.second.send_queue.clear();
	}
	peer_list.clear();
	disconnect_queue.clear();
}

protocol_zt::~protocol_zt()
{
	if (fd_tcp != -1) {
		lwip_close(fd_tcp);
		fd_tcp = -1;
	}
	if (fd_udp != -1) {
		lwip_close(fd_udp);
		fd_udp = -1;
	}
	close();
	zerotier_network_stop();
}

void protocol_zt::endpoint::from_string(const std::string& str)
{
	ip_addr_t a;
	if (ipaddr_aton(str.c_str(), &a) == 0)
		return;
	if (!IP_IS_V6_VAL(a))
		return;
	from_addr(reinterpret_cast<const unsigned char*>(a.u_addr.ip6.addr));
}

void protocol_zt::endpoint::from_addr(const unsigned char* src_addr)
{
	memcpy(addr.data(), src_addr, sizeof(addr)); 
}

void protocol_zt::endpoint::to_addr(unsigned char* dest_addr) const
{
	memcpy(dest_addr, addr.data(), sizeof(addr)); 
}

void protocol_zt::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	int i;

	std::string allowedChars = "abcdefghkopqrstuvwxyz";
	std::random_device rd;
	std::uniform_int_distribution<int> dist(0, allowedChars.size() - 1);
	for (i = 0; i < 5; i++) {
		gamename[i] = allowedChars.at(dist(rd));
	}
	gamename[i] = '\0';
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
