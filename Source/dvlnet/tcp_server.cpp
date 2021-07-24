#include "tcp_server.h"
#ifdef TCPIP
#include <chrono>
#include <functional>
#include <memory>
#include <utility>

#include "base.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

tcp_server::tcp_server(asio::io_context &ioc, const char* bindAddr,
    unsigned short port, const char* passwd, buffer_t info)
    : ioc(ioc)
    , connTimer(ioc)
    , game_init_info(info)
{
	pktfty.setup_password(passwd);
	auto addr = asio::ip::address::from_string(bindAddr);
	auto ep = asio::ip::tcp::endpoint(addr, port);
	acceptor = new asio::ip::tcp::acceptor(ioc, ep, true);
	start_accept();
	start_timeout();
}

void tcp_server::make_default_gamename(char (&gamename)[128])
{
	if (!getIniValue("Network", "Bind Address", gamename, sizeof(gamename) - 1)) {
		copy_cstr(gamename, "127.0.0.1");
		//SStrCopy(gamename, asio::ip::address_v4::loopback().to_string().c_str(), sizeof(gamename));
	}
}

tcp_server::scc tcp_server::make_connection(asio::io_context &ioc)
{
	return std::make_shared<client_connection>(ioc);
}

plr_t tcp_server::next_free_conn()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (connections[i] == NULL)
			break;
	return i < ((SNetGameData*)game_init_info.data())->bMaxPlayers ? i : MAX_PLRS;
}

plr_t tcp_server::next_free_queue()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (pending_connections[i] == NULL)
			break;
	return i;
}

void tcp_server::start_recv(const scc &con)
{
	con->socket.async_receive(asio::buffer(con->recv_buffer),
	    std::bind(&tcp_server::handle_recv, this, con,
	        std::placeholders::_1,
	        std::placeholders::_2));
}

void tcp_server::handle_recv(const scc &con, const asio::error_code &ec, size_t bytesRead)
{
	if (ec || bytesRead == 0) {
		drop_connection(con);
		return;
	}
	con->recv_buffer.resize(bytesRead);
	con->recv_queue.write(std::move(con->recv_buffer));
	con->recv_buffer.resize(frame_queue::MAX_FRAME_SIZE);
	while (con->recv_queue.packet_ready()) {
		auto pkt = pktfty.make_in_packet(con->recv_queue.read_packet());
		if (pkt == NULL || !handle_recv_packet(con, *pkt)) {
			drop_connection(con);
			return;
		}
	}
	start_recv(con);
}

/*void tcp_server::send_connect(const scc &con)
{
	auto pkt = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST,
	    con->pnum);
	send_packet(*pkt);
}*/

bool tcp_server::handle_recv_newplr(const scc &con, packet &pkt)
{
	plr_t i, pnum;

	if (pkt.pktType() != PT_JOIN_REQUEST) {
		// SDL_Log("Invalid join packet.");
		return false;
	}
	pnum = next_free_conn();
	for (i = 0; i < MAX_PLRS; i++) {
		if (pending_connections[i] == con)
			break;
	}
	if (pnum == MAX_PLRS || i == MAX_PLRS) {
		// SDL_Log(pnum == MAX_PLRS ? "Server is full." : "Dropped connection.");
		return false;
	}
	pending_connections[i] = NULL;
	connections[pnum] = con;
	con->pnum = pnum;
	auto reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(PLR_MASTER, PLR_BROADCAST,
	    pkt.pktJoinReqCookie(), pnum,
	    game_init_info);
	start_send(con, *reply);
	//send_connect(con);
	return true;
}

bool tcp_server::handle_recv_packet(const scc &con, packet &pkt)
{
	if (con->pnum != PLR_BROADCAST) {
		return con->pnum == pkt.pktSrc() && send_packet(pkt);
	} else {
		return handle_recv_newplr(con, pkt);
	}
}

bool tcp_server::send_packet(packet &pkt)
{
	plr_t dest = pkt.pktDest();
	plr_t src = pkt.pktSrc();

	if (dest == PLR_BROADCAST) {
		for (int i = 0; i < MAX_PLRS; i++)
			if (i != src && connections[i] != NULL)
				start_send(connections[i], pkt);
	} else {
		if (dest >= MAX_PLRS) {
			// SDL_Log("Invalid destination %d", dest);
			return false;
		}
		if ((dest != src) && connections[dest] != NULL)
			start_send(connections[dest], pkt);
	}
	return true;
}

void tcp_server::start_send(const scc &con, packet &pkt)
{
	const auto *frame = new buffer_t(frame_queue::make_frame(pkt.encrypted_data()));
	auto buf = asio::buffer(*frame);
	asio::async_write(con->socket, buf,
		[frame](const asio::error_code &ec, size_t bytesSent) {
			delete frame;
		});
}

void tcp_server::start_accept()
{
	if (next_free_queue() != MAX_PLRS) {
		nextcon = make_connection(ioc);
		acceptor->async_accept(nextcon->socket,
			std::bind(&tcp_server::handle_accept,
				this, true,
				std::placeholders::_1));
	} else {
		nextcon = NULL;
		connTimer.expires_after(std::chrono::seconds(10));
		connTimer.async_wait(std::bind(&tcp_server::handle_accept,
			this, false,
			std::placeholders::_1));
	}
}

void tcp_server::handle_accept(bool valid, const asio::error_code &ec)
{
	if (ec)
		return;
	if (valid) {
		asio::ip::tcp::no_delay option(true);
		nextcon->socket.set_option(option);
		nextcon->timeout = TIMEOUT_CONNECT;
		pending_connections[next_free_queue()] = nextcon;
		start_recv(nextcon);
	}
	start_accept();
}

void tcp_server::start_timeout()
{
	connTimer.expires_after(std::chrono::seconds(1));
	connTimer.async_wait(std::bind(&tcp_server::handle_timeout, this,
		std::placeholders::_1));
}

void tcp_server::handle_timeout(const asio::error_code &ec)
{
	int i, n;

	if (ec)
		return;

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
		if (connections[i] != NULL) {
			if (connections[i]->timeout > 0) {
				connections[i]->timeout--;
			} else {
				expired_connections[n] = connections[i];
				n++;
			}
		}
	}
	for (i = 0; i < n; i++)
		drop_connection(expired_connections[i]);
	start_timeout();
}

void tcp_server::drop_connection(const scc &con)
{
	plr_t i, pnum = con->pnum;

	if (pnum != PLR_BROADCAST) {
		// live connection
		if (connections[pnum] == con) {
			connections[pnum] = NULL;
			// notify the other clients
			auto pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST,
				pnum, (leaveinfo_t)LEAVE_DROP);
			send_packet(*pkt);
		}
	} else {
		// pending connection
		for (i = 0; i < MAX_PLRS; i++) {
			if (pending_connections[i] == con) {
				pending_connections[i] = NULL;
			}
		}
	}
	con->socket.close();
}

void tcp_server::close()
{
	int i;

	if (acceptor == NULL)
		return;

	auto pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST,
		PLR_MASTER, (leaveinfo_t)LEAVE_DROP);
	send_packet(*pkt);

	acceptor->close();
	connTimer.cancel();

	ioc.poll();
	if (nextcon != NULL)
		nextcon->socket.close();
	for (i = 0; i < MAX_PLRS; i++) {
		if (connections[i] != NULL) {
			connections[i]->socket.shutdown(asio::socket_base::shutdown_both);
			connections[i]->socket.close();
		}
	}
	for (i = 0; i < MAX_PLRS; i++) {
		if (pending_connections[i] != NULL) {
			pending_connections[i]->socket.close();
		}
	}
	ioc.poll();

	delete acceptor;
	acceptor = NULL;
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP