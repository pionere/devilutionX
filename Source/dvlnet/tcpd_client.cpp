#include "tcpd_client.h"
#ifdef TCPIP
#include <SDL.h>
#include <exception>
#include <functional>
#include <memory>
#include <sodium.h>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <asio/connect.hpp>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

bool tcpd_client::create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info)
{
	setup_gameinfo(std::move(info));
	local_server = new tcp_server(ioc, game_init_info, SRV_DIRECT);
	if (local_server->setup_server(addrstr, port, passwd)) {
		return join_game(addrstr, port, passwd);
	}
	close();
	return false;
}

bool tcpd_client::join_game(const char* addrstr, unsigned port, const char* passwd)
{
	int i;
	constexpr int MS_SLEEP = 10;
	constexpr int NUM_SLEEP = 250;

	memset(connected_table, 0, sizeof(connected_table));
	plr_self = PLR_BROADCAST;
	randombytes_buf(reinterpret_cast<unsigned char *>(&cookie_self),
		sizeof(cookie_t));
	setup_password(passwd);
	// connect to the server
	asio::error_code err;
	tcp_server::connect_socket(sock, addrstr, port, ioc, err);
	if (err) {
		SDL_SetError("%s", err.message().c_str());
		close();
		return false;
	}
	// setup acceptor for the direct connection to other players
	const auto &ep = sock.local_endpoint(err);
	assert(!err);
	tcp_server::connect_acceptor(acceptor, ep, err);
	if (err) {
		SDL_SetError("%s", err.message().c_str());
		close();
		return false;
	}
	// start the communication
	start_recv();
	start_accept_conn();
	start_timeout();

	auto pkt = pktfty.make_out_packet<PT_JOIN_REQUEST>(PLR_BROADCAST,
		PLR_MASTER, cookie_self);
	send_packet(*pkt);
	for (i = 0; i < NUM_SLEEP; i++) {
		poll();
		if (plr_self != PLR_BROADCAST)
			return true; // join successful
		SDL_Delay(MS_SLEEP);
	}	
	if (i == NUM_SLEEP)
		SDL_SetError("Unable to connect");
	close();
	return false;
}

void tcpd_client::poll()
{
	asio::error_code err;
	ioc.poll(err);
	assert(!err);
}

void tcpd_client::start_timeout()
{
	connTimer.expires_after(std::chrono::seconds(1));
	connTimer.async_wait(std::bind(&tcpd_client::handle_timeout, this,
		std::placeholders::_1));
}

void tcpd_client::handle_timeout(const asio::error_code &ec)
{
	int i, n;

	if (ec)
		return;

	// TODO: add timeout to the server connection?

	tcp_server::scc expired_connections[2 * MAX_PLRS] = { };
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
	for (i = 0; i < n; i++)
		drop_connection(expired_connections[i]);
	start_timeout();
}

plr_t tcpd_client::next_free_conn()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (connections[i] == NULL)
			break;
	return i;
}

plr_t tcpd_client::next_free_queue()
{
	plr_t i;

	for (i = 0; i < MAX_PLRS; i++)
		if (pending_connections[i] == NULL)
			break;
	return i;
}

void tcpd_client::recv_connect(packet &pkt)
{
	plr_t pnum = pkt.pktConnectPlr();
	if (pnum == plr_self || pnum >= MAX_PLRS || connections[pnum] != NULL)
		return;

	std::string addrstr = std::string(pkt.pktConnectAddrBegin(), pkt.pktConnectAddrEnd());
	int offset = addrstr.length() - tcp_server::PORT_LENGTH;
	int port = SDL_atoi(addrstr.data() + offset);
	addrstr[offset - 1] = '\0';

	auto cliCon = tcp_server::make_connection(ioc);
	asio::error_code err;
	tcp_server::connect_socket(cliCon->socket, addrstr.c_str(), port, ioc,  err);
	if (err) {
		DoLog("Failed to connect %s", err.message().c_str());
		return;
	}
	cliCon->pnum = pnum;
	cliCon->timeout = tcp_server::TIMEOUT_ACTIVE;
	connections[pnum] = cliCon;
	start_recv_conn(cliCon);
	auto joinPkt = pktfty.make_out_packet<PT_JOIN_REQUEST>(plr_self,
		PLR_BROADCAST, cookie_self);
	start_send(cliCon, *joinPkt);
}

void tcpd_client::start_accept_conn()
{
	if (next_free_queue() != MAX_PLRS) {
		nextcon = tcp_server::make_connection(ioc);
		acceptor.async_accept(nextcon->socket,
			std::bind(&tcpd_client::handle_accept_conn,
				this, true, std::placeholders::_1));
	} else {
		nextcon = NULL;
		connTimer.expires_after(std::chrono::seconds(10));
		connTimer.async_wait(std::bind(&tcpd_client::handle_accept_conn,
			this, false, std::placeholders::_1));
	}
}

void tcpd_client::handle_accept_conn(bool valid, const asio::error_code &ec)
{
	if (ec)
		return;

	if (valid) {
		asio::error_code err;
		asio::ip::tcp::no_delay option(true);
		nextcon->socket.set_option(option, err);
		assert(!err);
		nextcon->timeout = tcp_server::TIMEOUT_CONNECT;
		pending_connections[next_free_queue()] = nextcon;
		start_recv_conn(nextcon);
	}
	start_accept_conn();
}

void tcpd_client::start_recv_conn(const tcp_server::scc &con)
{
	con->socket.async_receive(asio::buffer(con->recv_buffer),
		std::bind(&tcpd_client::handle_recv_conn, this, con,
			std::placeholders::_1, std::placeholders::_2));
}

void tcpd_client::handle_recv_conn(const tcp_server::scc &con, const asio::error_code &ec, size_t bytesRead)
{
	if (ec || bytesRead == 0) {
		drop_connection(con);
		return;
	}
	con->timeout = tcp_server::TIMEOUT_ACTIVE;
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
	start_recv_conn(con);
}

void tcpd_client::start_send(const tcp_server::scc &con, packet &pkt)
{
	const auto *frame = new buffer_t(frame_queue::make_frame(pkt.encrypted_data()));
	auto buf = asio::buffer(*frame);
	asio::async_write(con->socket, buf,
		[frame](const asio::error_code &ec, size_t bytesSent) {
			delete frame;
		});
}

bool tcpd_client::handle_recv_newplr(const tcp_server::scc &con, packet &pkt)
{
	plr_t i, pnum;
	
	if (pkt.pktType() != PT_JOIN_REQUEST) {
		// DoLog("Invalid join packet.");
		return false;
	}
	pnum = pkt.pktSrc();
	for (i = 0; i < MAX_PLRS; i++) {
		if (pending_connections[i] == con)
			break;
	}
	if (pnum >= MAX_PLRS || connections[pnum] != NULL || i == MAX_PLRS) {
		// DoLog(pnum == MAX_PLRS ? "Server is full." : "Dropped connection.");
		return false;
	}
	pending_connections[i] = NULL;
	connections[pnum] = con;
	con->pnum = pnum;
	/*auto reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(PLR_MASTER, PLR_BROADCAST,
		pkt.pktJoinReqCookie(), pnum, game_init_info);
	start_send(con, *reply);*/
	//send_connect(con);
	return true;
}

bool tcpd_client::handle_recv_packet(const tcp_server::scc &con, packet &pkt)
{
	if (con->pnum != PLR_BROADCAST) {
		if (con->pnum != pkt.pktSrc())
			return false;
		recv_local(pkt);
		return true;
	} else {
		return handle_recv_newplr(con, pkt);
	}
}

void tcpd_client::disconnect_net(plr_t pnum)
{
	// TODO: check connections and pending_connections?
	/*if (connections[pnum] != NULL) {
		drop_connection(connections[pnum]);
	}*/
}

void tcpd_client::drop_connection(const tcp_server::scc &con)
{
	plr_t i, pnum = con->pnum;

	if (pnum != PLR_BROADCAST) {
		// live connection
		if (connections[pnum] == con) {
			connections[pnum] = NULL;
			//auto pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST,
			//	pnum, (leaveinfo_t)LEAVE_DROP);
			//send_packet(*pkt);
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

void tcpd_client::handle_recv(const asio::error_code &ec, size_t bytesRead)
{
	if (ec || bytesRead == 0) {
		// error in recv from server
		// returning and doing nothing should be the same
		// as if all connections to other clients were lost
		return;
	}
	recv_buffer.resize(bytesRead);
	recv_queue.write(std::move(recv_buffer));
	recv_buffer.resize(frame_queue::MAX_FRAME_SIZE);
	while (recv_queue.packet_ready()) {
		auto pkt = pktfty.make_in_packet(recv_queue.read_packet());
		assert(pkt != NULL);
		recv_local(*pkt);
	}
	start_recv();
}

void tcpd_client::start_recv()
{
	sock.async_receive(asio::buffer(recv_buffer),
		std::bind(&tcpd_client::handle_recv, this,
			std::placeholders::_1, std::placeholders::_2));
}

void tcpd_client::send_packet(packet &pkt)
{
	if (pkt.pktType() == PT_TURN) {
		//plr_t dest = pkt.pktDest();
		//plr_t src = plr_self; //pkt.pktSrc();

		//assert(pkt.pktSrc() == src);
		//assert(dest == PLR_BROADCAST);
		//if (dest == PLR_BROADCAST) {
			for (int i = 0; i < MAX_PLRS; i++)
				if (/*i != src &&*/ connections[i] != NULL)
					start_send(connections[i], pkt);
		/*} else if (dest < MAX_PLRS) {
			if ((dest != src) && connections[dest] != NULL)
				start_send(connections[dest], pkt);
		}*/
	}

	const auto *frame = new buffer_t(frame_queue::make_frame(pkt.encrypted_data()));
	auto buf = asio::buffer(*frame);
	asio::async_write(sock, buf,
		[frame](const asio::error_code &ec, size_t bytesSent) {
			delete frame;
		});
}

void tcpd_client::close()
{
	int i;

	if (local_server != NULL) {
		local_server->close();
		delete local_server;
		local_server = NULL;
	}

	recv_queue.clear();

	asio::error_code err;
	sock.shutdown(asio::socket_base::shutdown_both, err);
	err.clear();
	sock.close(err);
	err.clear();

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
		if (connections[i] != NULL) {
			connections[i]->socket.shutdown(asio::socket_base::shutdown_both, err);
			err.clear();
			connections[i]->socket.close(err);
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

void tcpd_client::SNetLeaveGame(int reason)
{
	base::SNetLeaveGame(reason);
	poll();
	close();
}

void tcpd_client::make_default_gamename(char (&gamename)[128])
{
	tcp_server::make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP