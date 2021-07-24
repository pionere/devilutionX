#include "tcp_server.h"

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

tcp_server::scc tcp_server::make_connection()
{
	return std::make_shared<client_connection>(ioc);
}

plr_t tcp_server::next_free()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (connections[i] == NULL)
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
		try {
			auto pkt = pktfty.make_in_packet(con->recv_queue.read_packet());
			con->timeout = TIMEOUT_ACTIVE;
			if (con->pnum == PLR_BROADCAST) {
				handle_recv_newplr(con, *pkt);
			} else {
				handle_recv_packet(*pkt);
			}
		} catch (dvlnet_exception &e) {
			SDL_Log("Network error: %s", e.what());
			drop_connection(con);
			return;
		}
	}
	start_recv(con);
}

void tcp_server::send_connect(const scc &con)
{
	auto pkt = pktfty.make_out_packet<PT_CONNECT>(PLR_MASTER, PLR_BROADCAST,
	    con->pnum);
	send_packet(*pkt);
}

void tcp_server::handle_recv_newplr(const scc &con, packet &pkt)
{
	auto pnum = next_free();
	if (pnum == MAX_PLRS)
		throw server_exception();
	auto reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(PLR_MASTER, PLR_BROADCAST,
	    pkt.cookie(), pnum,
	    game_init_info);
	start_send(con, *reply);
	con->pnum = pnum;
	connections[pnum] = con;
	send_connect(con);
}

void tcp_server::handle_recv_packet(packet &pkt)
{
	send_packet(pkt);
}

void tcp_server::send_packet(packet &pkt)
{
	plr_t dest = pkt.dest();
	plr_t src = pkt.src();

	if (dest == PLR_BROADCAST) {
		for (auto i = 0; i < MAX_PLRS; ++i)
			if (i != src && connections[i] != NULL)
				start_send(connections[i], pkt);
	} else {
		if (dest >= MAX_PLRS)
			throw server_exception();
		if ((dest != src) && connections[dest] != NULL)
			start_send(connections[dest], pkt);
	}
}

void tcp_server::start_send(const scc &con, packet &pkt)
{
	const auto *frame = new buffer_t(frame_queue::make_frame(pkt.data()));
	auto buf = asio::buffer(*frame);
	asio::async_write(con->socket, buf,
	    [this, con, frame](const asio::error_code &ec, size_t bytesSent) {
		    handle_send(con, ec, bytesSent);
		    delete frame;
	    });
}

void tcp_server::handle_send(const scc &con, const asio::error_code &ec, size_t bytesSent)
{
	// empty for now
}

void tcp_server::start_accept()
{
	auto nextcon = make_connection();
	acceptor->async_accept(nextcon->socket,
	    std::bind(&tcp_server::handle_accept,
	        this, nextcon,
	        std::placeholders::_1));
}

void tcp_server::handle_accept(const scc &con, const asio::error_code &ec)
{
	if (ec)
		return;
	if (next_free() == MAX_PLRS) {
		drop_connection(con);
	} else {
		asio::ip::tcp::no_delay option(true);
		con->socket.set_option(option);
		con->timeout = TIMEOUT_CONNECT;
		start_recv(con);
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

	scc expired_connections[MAX_PLRS] = { };
	n = 0;
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
	if (con->pnum != PLR_BROADCAST) {
		auto pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST,
		    con->pnum, (leaveinfo_t)LEAVE_DROP);
		connections[con->pnum] = NULL;
		send_packet(*pkt);
		// TODO: investigate if it is really ok for the server to
		//       drop a client directly.
	}
	con->socket.close();
}

void tcp_server::close()
{
	if (acceptor == NULL)
		return;
	acceptor->close();
	connTimer.cancel();

	ioc.poll();

	delete acceptor;
	acceptor = NULL;
}

} // namespace net
DEVILUTION_END_NAMESPACE
