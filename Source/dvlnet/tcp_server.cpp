#include "tcp_server.h"
#ifdef TCPIP
#include <chrono>
#include <memory>

#include "storm/storm_cfg.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

scc make_shared_cc(asio::io_context& ioc)
{
	return std::make_shared<client_connection>(ioc);
}

tcp_server::tcp_server(base_client& client, asio::io_context& ioc, packet_factory& pf, SNetGameData& gameinfo, unsigned srvType)
    : local_client(client), ioc(ioc), acceptor(ioc), connTimer(ioc), pktfty(pf), game_init_info(gameinfo), serverType(srvType)
{
}

bool tcp_server::setup_server(const char* bindAddr, unsigned short port, char (&errorText)[256])
{
	asio::error_code err;
	auto addr = asio::ip::address::from_string(bindAddr, err);
	if (!err) {
		auto ep = asio::ip::tcp::endpoint(addr, port);
		connect_acceptor(acceptor, ep, err);
	}
	if (err) {
		SStrCopy(errorText, err.message().c_str(), lengthof(errorText));
		close();
		return false;
	}

	start_accept();
	start_timeout();
	return true;
}

void tcp_server::connect_acceptor(asio::ip::tcp::acceptor& acceptor, const asio::ip::tcp::endpoint& ep, asio::error_code& ec)
{
	acceptor.open(ep.protocol(), ec);
	if (ec)
		return;
	acceptor.set_option(asio::socket_base::reuse_address(true), ec);
	assert(!ec);
	acceptor.bind(ep, ec);
	if (ec)
		return;
	acceptor.listen(2 * MAX_PLRS, ec);
}

void tcp_server::connect_socket(asio::ip::tcp::socket& sock, const char* addrstr, unsigned port, asio::io_context& ioc, asio::error_code& ec)
{
	std::string strPort = std::to_string(port);
	auto resolver = asio::ip::tcp::resolver(ioc);
	auto addrList = resolver.resolve(addrstr, strPort, ec);
	if (ec)
		return;
	asio::connect(sock, addrList, ec);
	if (ec)
		return;
	asio::ip::tcp::no_delay option(true);
	sock.set_option(option, ec);
	assert(!ec);
}

void tcp_server::endpoint_to_buffer(const scc& con, buffer_t& buf)
{
	asio::error_code err;
	const auto& ep = con->socket.remote_endpoint(err);
	assert(!err);
	std::string addr = ep.address().to_string();
	for (auto it = addr.cbegin(); it != addr.cend(); it++) {
		buf.push_back(*it);
	}
	buf.push_back(':');
	char port[NET_TCP_PORT_LENGTH + 1];
	static_assert(NET_TCP_PORT_LENGTH == 5, "Bad port format in endpoint_to_buffer.");
	snprintf(port, sizeof(port), "%05d", ep.port());
	for (int i = 0; i < NET_TCP_PORT_LENGTH; i++) {
		buf.push_back(port[i]);
	}
}

void tcp_server::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	if (getIniValue("Network", "Bind Address", gamename, sizeof(gamename) - 1) <= 0) {
		copy_cstr(gamename, "127.0.0.1");
		//SStrCopy(gamename, asio::ip::address_v4::loopback().to_string().c_str(), sizeof(gamename));
	}
}

plr_t tcp_server::next_free_conn()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (active_connections[i] == NULL && ghost_connections[i] == 0)
			break;
	return i < game_init_info.ngMaxPlayers ? i : MAX_PLRS;
}

plr_t tcp_server::next_free_queue()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (pending_connections[i] == NULL)
			break;
	return i;
}

void tcp_server::start_recv(const scc& con)
{
	con->socket.async_receive(asio::buffer(con->recv_buffer),
	    std::bind(&tcp_server::handle_recv, this, con,
	        std::placeholders::_1,
	        std::placeholders::_2));
}

void tcp_server::handle_recv(const scc& con, const asio::error_code& ec, size_t bytesRead)
{
	if (ec || bytesRead == 0) {
		drop_connection(con);
		return;
	}
	con->timeout = NET_TIMEOUT_ACTIVE;
	con->recv_queue.write(con->recv_buffer, bytesRead);
	while (con->recv_queue.packet_ready()) {
		packet* pkt = pktfty.make_in_packet(con->recv_queue.read_packet());
		if (pkt == NULL || !handle_recv_packet(*pkt, con)) {
			delete pkt;
			drop_connection(con);
			return;
		}
		delete pkt;
	}
	start_recv(con);
}

bool tcp_server::recv_ctrl(packet& pkt, const scc& con)
{
	plr_t i, pnum, pmask;
	packet* reply;

	if (pkt.pktType() != PT_JOIN_REQUEST) {
		// DoLog("Invalid join packet.");
		return false;
	}
	pnum = next_free_conn();
	for (i = 0; i < MAX_PLRS; i++) {
		if (pending_connections[i] == con)
			break;
	}
	if (pnum == MAX_PLRS || i == MAX_PLRS) {
		// DoLog(pnum == MAX_PLRS ? "Server is full." : "Dropped connection.");
		return false;
	}
	pending_connections[i] = NULL;
	active_connections[pnum] = con;
	con->pnum = pnum;

	// select the connecting turn for the new client
	turn_t conTurn = local_client.last_recv_turn() + NET_JOIN_WINDOW;

	// reply to the new player
	bool sendAddr = serverType == SRV_DIRECT;
	buffer_t addrs;
	pmask = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (active_connections[i] != NULL) {
			static_assert(sizeof(pmask) * 8 >= MAX_PLRS, "handle_recv_newplr can not send the active connections to the client.");
			pmask |= 1 << i;
			if (sendAddr) {
				endpoint_to_buffer(active_connections[i], addrs);
			}
		}
		addrs.push_back(' ');
	}
	reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(PLR_MASTER, PLR_BROADCAST, pkt.pktJoinReqCookie(), pnum, (const BYTE*)&game_init_info, pmask, conTurn, (const BYTE*)addrs.data(), (unsigned)addrs.size());
	start_send(*reply, con);
	delete reply;
	// notify the old players
	reply = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST, pnum, conTurn, (const BYTE*)NULL, 0u);
	send_packet(*reply);
	delete reply;
	return true;
}

bool tcp_server::handle_recv_packet(packet& pkt, const scc& con)
{
	plr_t src = pkt.pktSrc();

	if (src != PLR_BROADCAST) {
		return src == con->pnum && send_packet(pkt);
	} else {
		return recv_ctrl(pkt, con);
	}
}

bool tcp_server::send_packet(packet& pkt)
{
	plr_t dest = pkt.pktDest();
	plr_t src = pkt.pktSrc();

	if (dest == PLR_BROADCAST) {
		for (int i = 0; i < MAX_PLRS; i++)
			if (i != src && active_connections[i] != NULL)
				start_send(pkt, active_connections[i]);
	} else {
		if (dest >= MAX_PLRS) {
			// DoLog("Invalid destination %d", dest);
			return false;
		}
		if ((dest != src) && active_connections[dest] != NULL)
			start_send(pkt, active_connections[dest]);
	}
	return true;
}

void tcp_server::start_send(packet& pkt, const scc& con)
{
	const buffer_t* frame = frame_queue::make_frame(pkt.encrypted_data());
	auto buf = asio::buffer(*frame);

	asio::async_write(con->socket, buf, [frame](const asio::error_code& ec, size_t bytesSent) {
	    delete frame;
	});
}

void tcp_server::start_accept()
{
	if (next_free_queue() != MAX_PLRS) {
		nextcon = make_shared_cc(ioc);
		acceptor.async_accept(nextcon->socket, std::bind(&tcp_server::handle_accept, this, true, std::placeholders::_1));
	} else {
		nextcon = NULL;
		connTimer.expires_after(std::chrono::seconds(NET_WAIT_PENDING));
		connTimer.async_wait(std::bind(&tcp_server::handle_accept, this, false, std::placeholders::_1));
	}
}

void tcp_server::handle_accept(bool valid, const asio::error_code& ec)
{
	if (ec)
		return;
	if (valid) {
		asio::error_code err;
		asio::ip::tcp::no_delay option(true);
		nextcon->socket.set_option(option, err);
		assert(!err);
		nextcon->timeout = NET_TIMEOUT_CONNECT;
		pending_connections[next_free_queue()] = nextcon;
		start_recv(nextcon);
	}
	start_accept();
}

void tcp_server::start_timeout()
{
	connTimer.expires_after(std::chrono::seconds(NET_TIMEOUT_BASE));
	connTimer.async_wait(std::bind(&tcp_server::handle_timeout, this, std::placeholders::_1));
}

void tcp_server::handle_timeout(const asio::error_code& ec)
{
	int i, n;

	if (ec)
		return;

	for (i = 0; i < MAX_PLRS; i++) {
		int gc = ghost_connections[i];
		if (gc != 0) {
			ghost_connections[i] = gc - 1;
		}
	}
	scc expired_connections[2 * MAX_PLRS] = { };
	n = 0;
	for (i = 0; i < MAX_PLRS; i++) {
		if (pending_connections[i] != NULL) {
			if (pending_connections[i]->timeout > 0) {
				pending_connections[i]->timeout--;
			} else {
				expired_connections[n] = pending_connections[i];
				n++;
			}
		}
	}
	for (i = 0; i < MAX_PLRS; i++) {
		if (active_connections[i] != NULL) {
			if (active_connections[i]->timeout > 0) {
				active_connections[i]->timeout--;
			} else {
				expired_connections[n] = active_connections[i];
				n++;
			}
		}
	}
	for (i = 0; i < n; i++)
		drop_connection(expired_connections[i]);
	start_timeout();
}

void tcp_server::drop_connection(const scc& con)
{
	plr_t i, pnum = con->pnum;

	if (pnum != PLR_BROADCAST) {
		// live connection
		if (active_connections[pnum] == con) {
			active_connections[pnum] = NULL;
			ghost_connections[pnum] = NET_TIMEOUT_GHOST;
			// notify the other clients
			packet* pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST, pnum);
			send_packet(*pkt);
			delete pkt;
		}
	} else {
		// pending connection
		for (i = 0; i < MAX_PLRS; i++) {
			if (pending_connections[i] == con) {
				pending_connections[i] = NULL;
			}
		}
	}
	asio::error_code err;
	con->socket.close(err);
}

void tcp_server::close()
{
	int i;
	asio::error_code err;

	if (acceptor.is_open()) {
		packet* pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST, PLR_MASTER);
		send_packet(*pkt);
		delete pkt;
		ioc.poll(err);
		err.clear();
	}

	acceptor.close(err);
	err.clear();
	connTimer.cancel(err);
	err.clear();

	ioc.poll(err);
	err.clear();
	if (nextcon != NULL) {
		nextcon->socket.close(err);
		err.clear();
	}
	for (i = 0; i < MAX_PLRS; i++) {
		if (active_connections[i] != NULL) {
			active_connections[i]->socket.shutdown(asio::socket_base::shutdown_both, err);
			err.clear();
			active_connections[i]->socket.close(err);
			err.clear();
		}
	}
	for (i = 0; i < MAX_PLRS; i++) {
		if (pending_connections[i] != NULL) {
			pending_connections[i]->socket.close(err);
			err.clear();
		}
	}
	ioc.poll(err);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
