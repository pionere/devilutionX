#include "protocol_zt.h"
#ifdef ZEROTIER
#include <random>

#include <lwip/sockets.h>

#include "dvlnet/zerotier_lwip.h"
#include "dvlnet/zerotier_native.h"
#include "dvlnet/zt_client.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

protocol_zt::protocol_zt()
{
	zerotier_network_start();
}

void protocol_zt::set_nonblock(int fd)
{
	static_assert(O_NONBLOCK == 1, "O_NONBLOCK == 1 not satisfied");
	// auto mode = lwip_fcntl(fd, F_GETFL, 0);
	// mode |= O_NONBLOCK;
	// lwip_fcntl(fd, F_SETFL, mode);
	lwip_fcntl(fd, F_SETFL, O_NONBLOCK);
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
		set_nonblock(fd_udp);
		auto ret = lwip_bind(fd_udp, (struct sockaddr*)&in6, sizeof(in6));
		if (ret < 0) {
			DoLog("lwip, (udp) bind: %s", strerror(errno));
			return false;
		}
	}
	if (fd_tcp == -1) {
		fd_tcp = lwip_socket(AF_INET6, SOCK_STREAM, 0);
		set_reuseaddr(fd_tcp);
		set_nonblock(fd_tcp);
		set_nodelay(fd_tcp);
		auto r1 = lwip_bind(fd_tcp, (struct sockaddr*)&in6, sizeof(in6));
		if (r1 < 0) {
			DoLog("lwip, (tcp) bind: %s", strerror(errno));
			return false;
		}
		auto r2 = lwip_listen(fd_tcp, 2 * MAX_PLRS);
		if (r2 < 0) {
			DoLog("lwip, listen: %s", strerror(errno));
			return false;
		}
	}
	return true;
}

void protocol_zt::send(int pnum, const buffer_t& data)
{
	active_connections[pnum].send_frame_queue.push_back(frame_queue::make_frame(data));
}

bool protocol_zt::send_oob(const endpoint& peer, const buffer_t& data) const
{
	struct sockaddr_in6 in6 {
	};
	in6.sin6_port = htons(DEFAULT_PORT);
	in6.sin6_family = AF_INET6;
	peer.to_addr(in6.sin6_addr.un.u8_addr);
	lwip_sendto(fd_udp, data.data(), data.size(), 0, (const struct sockaddr*)&in6, sizeof(in6));
	return true;
}

bool protocol_zt::send_oob_mc(const buffer_t& data) const
{
	endpoint mc;
	mc.from_addr(dvl_multicast_addr);
	return send_oob(mc, data);
}

void protocol_zt::send_queued_peer(peer_connection& pc)
{
	while (!pc.send_frame_queue.empty()) {
		buffer_t* frame = pc.send_frame_queue.front();
		size_t len = frame->size();
		auto r = lwip_send(pc.sock, frame->data(), len, 0);
		if (r < 0) {
			break; // handle error?
		}
		if (decltype(len)(r) < len) {
			// partial send
			auto it = frame->begin();
			frame->erase(it, it + r);
			break;
		}
		// assert(decltype(len)(r) == len);
		delete frame;
		pc.send_frame_queue.pop_front();
	}
}

void protocol_zt::recv_peer(peer_connection& pc)
{
	while (true) {
		auto len = lwip_recv(pc.sock, recv_buffer.data(), frame_queue::MAX_FRAME_SIZE, 0);
		if (len >= 0) {
			pc.recv_queue.write(recv_buffer, len);
		} else {
			break; // handle error? errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOTCONN;
		}
	}
}

void protocol_zt::send_queued_all()
{
	for (peer_connection& ap : active_connections) {
		if (ap.sock != -1) {
			send_queued_peer(ap);
		}
	}
}

void protocol_zt::recv_from_peers(zt_client* client)
{
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		peer_connection& ap = active_connections[pnum];
		if (ap.sock != -1) {
			recv_peer(ap);
			while (ap.recv_queue.packet_ready()) {
				buffer_t pkt_buf = ap.recv_queue.read_packet();
				client->handle_recv(pkt_buf, pnum);
			}
		}
	}
}

void protocol_zt::recv_from_udp(zt_client* client)
{
	struct sockaddr_in6 in6 {
	};
	socklen_t addrlen = sizeof(in6);
	while (true) {
		auto len = lwip_recvfrom(fd_udp, recv_buffer.data(), frame_queue::MAX_FRAME_SIZE, 0, (struct sockaddr*)&in6, &addrlen);
		if (len < 0)
			break;
		endpoint ep;
		ep.from_addr(in6.sin6_addr.un.u8_addr);
		buffer_t pkt_buf = buffer_t(recv_buffer.begin(), recv_buffer.begin() + len);
		client->handle_recv_oob(pkt_buf, ep);
	}
}

void protocol_zt::accept_all()
{
	struct sockaddr_in6 in6 {
	};
	socklen_t addrlen = sizeof(in6);
	while (true) {
		auto newfd = lwip_accept(fd_tcp, (struct sockaddr*)&in6, &addrlen);
		if (newfd < 0)
			break;
		set_nonblock(newfd);
		set_nodelay(newfd);

		int i;
		for (i = 0; i < MAX_PLRS; i++) {
			if (pending_connections[i].sock == 0) {
				pending_connections[i].sock = newfd + 1;
				pending_connections[i].peer.from_addr(in6.sin6_addr.un.u8_addr);
				pending_connections[i].timeout = SDL_GetTicks() + NET_TIMEOUT_SOCKET;
				break;
			}
		}
		if (i >= MAX_PLRS) {
			lwip_close(newfd);
		}
	}

	for (pending_connection &pc : pending_connections) {
		if (pc.sock == 0)
			continue;
		for (peer_connection &ac : active_connections) {
			if (ac.status != CS_PENDING)
				continue;
			if (ac.peer == pc.peer) {
				ac.sock = pc.sock - 1;
				ac.status = CS_ACTIVE;
				pc.sock = 0;
				break;
			}
		}
		if (pc.sock != 0 && SDL_TICKS_PASSED(SDL_GetTicks(), pc.timeout)) {
			lwip_close(pc.sock);
			pc.sock = 0;
		}
	}
}

void protocol_zt::accept_self(int pnum)
{
	zts_sockaddr_storage myaddr;
	if (!zerotier_current_addr(&myaddr)) {
		DoLog("protocol_zt::get_local_addr zts_addr_get failed: %s", strerror(errno));
		return;
	}
	active_connections[pnum].peer.from_addr(((zts_sockaddr_in6*)&myaddr)->sin6_addr.un.u8_addr);
	active_connections[pnum].status = CS_ACTIVE_SELF;
	// assert(active_connections[pnum].sock == -1);
}

void protocol_zt::accept_ep(const unsigned char* addr, int pnum)
{
	// assert(active_connections[pnum].status == CS_INACTIVE);
	// assert(active_connections[pnum].sock == -1);
	active_connections[pnum].status = CS_PENDING;
	active_connections[pnum].peer.from_addr(addr);
}

void protocol_zt::connect_ep(const unsigned char* addr, int pnum)
{
	if (active_connections[pnum].status != CS_INACTIVE)
		return;

	accept_ep(addr, pnum);

	auto sock = lwip_socket(AF_INET6, SOCK_STREAM, 0);
	set_nodelay(sock);
	set_nonblock(sock);
	struct sockaddr_in6 in6 {
	};
	in6.sin6_port = htons(DEFAULT_PORT);
	in6.sin6_family = AF_INET6;
	memcpy(in6.sin6_addr.s6_addr, addr, sizeof(in6.sin6_addr.s6_addr));
	// print_ip6_addr(&in6);
	lwip_connect(sock, (const struct sockaddr*)&in6, sizeof(in6));
	active_connections[pnum].sock = sock;
	active_connections[pnum].status = CS_ACTIVE;
}

void protocol_zt::poll(zt_client* client)
{
	accept_all();

	recv_from_peers(client);
	recv_from_udp(client);

	send_queued_all();
}

void protocol_zt::disconnect(int pnum)
{
	peer_connection& ap = active_connections[pnum];
	if (ap.sock != -1) {
		lwip_close(ap.sock);
		ap.sock = -1;
	}
	for (auto frame : ap.send_frame_queue) {
		delete frame;
	}
	ap.peer.clear_addr();
	ap.send_frame_queue.clear();
	ap.status = CS_INACTIVE;
	ap.recv_queue.clear();
}

void protocol_zt::close()
{
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		disconnect(pnum);
	}
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
	zerotier_network_stop();
	close();
}

void protocol_zt::endpoint::clear_addr()
{
	memset(addr.data(), 0, endpoint::str_len());
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
	memcpy(addr.data(), src_addr, endpoint::str_len());
}

void protocol_zt::endpoint::to_addr(unsigned char* dest_addr) const
{
	memcpy(dest_addr, addr.data(), endpoint::str_len());
}

void protocol_zt::endpoint::to_buffer(buffer_t& buf) const
{
	buf.insert(buf.end(), addr.cbegin(), addr.cend());
}

void protocol_zt::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	int i;

	std::random_device rd;
	std::uniform_int_distribution<int> dist((int)'a', (int)'z');
	for (i = 0; i < NET_ZT_GAMENAME_LEN; i++) {
		gamename[i] = dist(rd);
	}
	gamename[NET_ZT_GAMENAME_LEN] = '\0';
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
