#include "tcpd_client.h"
#ifdef TCPIP
#include <SDL.h>
#include <memory>
#include <sodium.h>

#include <asio/connect.hpp>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

bool tcpd_client::setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	setup_password(passwd);

	if (gameData != NULL) {
		setup_gameinfo(gameData);
		local_server = new tcp_server(*this, ioc, pktfty, game_init_info, SRV_DIRECT);
		if (!local_server->setup_server(addrstr, port, errorText)) {
			close();
			return false;
		}
	}

	plr_self = PLR_BROADCAST;
	memset(connected_table, 0, sizeof(connected_table));
	randombytes_buf(reinterpret_cast<unsigned char*>(&cookie_self), sizeof(cookie_t));
	// connect to the server
	asio::error_code err;
	tcp_server::connect_socket(sock, addrstr, port, ioc, err);
	if (!err) {
		// setup acceptor for the direct connection to other players
		const auto& ep = sock.local_endpoint(err);
		assert(!err);
		tcp_server::connect_acceptor(acceptor, ep, err);
	}
	if (err) {
		SStrCopy(errorText, err.message().c_str(), lengthof(errorText));
		close();
		return false;
	}
	start_recv();
	start_accept_conn();
	start_timeout();

	if (join_game()) {
		return true;
	}
	copy_cstr(errorText, "Unable to connect");
	close();
	return false;
}

void tcpd_client::start_timeout()
{
	connTimer.expires_after(std::chrono::seconds(NET_TIMEOUT_BASE));
	connTimer.async_wait(std::bind(&tcpd_client::handle_timeout, this, std::placeholders::_1));
}

void tcpd_client::handle_timeout(const asio::error_code& ec)
{
	int i, n;

	if (ec)
		return;

	// TODO: add timeout to the server connection?

	scc expired_connections[2 * MAX_PLRS] = { };
	n = 0;
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
		if (active_connections[i] == NULL)
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

void tcpd_client::recv_connect(packet& pkt)
{
	base_client::recv_connect(pkt);

	plr_t pnum = pkt.pktConnectPlr();
	if (pnum == plr_self || pnum >= MAX_PLRS || active_connections[pnum] != NULL)
		return;

	std::string addrstr = std::string(pkt.pktConnectAddrBegin(), pkt.pktConnectAddrEnd());
	int offset = addrstr.length() - NET_TCP_PORT_LENGTH;
	int port = SDL_atoi(addrstr.data() + offset);
	addrstr[offset - 1] = '\0';

	auto cliCon = make_shared_cc(ioc);
	asio::error_code err;
	tcp_server::connect_socket(cliCon->socket, addrstr.c_str(), port, ioc, err);
	if (err) {
		DoLog("Failed to connect %s", err.message().c_str());
		return;
	}
	cliCon->pnum = pnum;
	cliCon->timeout = NET_TIMEOUT_ACTIVE;
	active_connections[pnum] = cliCon;
	start_recv_conn(cliCon);
	packet* joinPkt = pktfty.make_out_packet<PT_JOIN_REQUEST>(plr_self, PLR_BROADCAST, cookie_self);
	start_send(cliCon, *joinPkt);
	delete joinPkt;
}

void tcpd_client::start_accept_conn()
{
	if (next_free_queue() != MAX_PLRS) {
		nextcon = make_shared_cc(ioc);
		acceptor.async_accept(nextcon->socket, std::bind(&tcpd_client::handle_accept_conn, this, true, std::placeholders::_1));
	} else {
		nextcon = NULL;
		connTimer.expires_after(std::chrono::seconds(NET_WAIT_PENDING));
		connTimer.async_wait(std::bind(&tcpd_client::handle_accept_conn, this, false, std::placeholders::_1));
	}
}

void tcpd_client::handle_accept_conn(bool valid, const asio::error_code& ec)
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
		start_recv_conn(nextcon);
	}
	start_accept_conn();
}

void tcpd_client::start_recv_conn(const scc& con)
{
	con->socket.async_receive(asio::buffer(con->recv_buffer),
		std::bind(&tcpd_client::handle_recv_conn, this, con,
			std::placeholders::_1, std::placeholders::_2));
}

void tcpd_client::handle_recv_conn(const scc& con, const asio::error_code& ec, size_t bytesRead)
{
	if (ec || bytesRead == 0) {
		drop_connection(con);
		return;
	}
	con->timeout = NET_TIMEOUT_ACTIVE;
	con->recv_buffer.resize(bytesRead);
	con->recv_queue.write(std::move(con->recv_buffer));
	con->recv_buffer.resize(frame_queue::MAX_FRAME_SIZE);
	while (con->recv_queue.packet_ready()) {
		packet* pkt = pktfty.make_in_packet(con->recv_queue.read_packet());
		if (pkt == NULL || !handle_recv_packet(con, *pkt)) {
			delete pkt;
			drop_connection(con);
			return;
		}
		delete pkt;
	}
	start_recv_conn(con);
}

void tcpd_client::start_send(const scc& con, packet& pkt)
{
	const buffer_t* frame = frame_queue::make_frame(pkt.encrypted_data());
	auto buf = asio::buffer(*frame);

	asio::async_write(con->socket, buf,	[frame](const asio::error_code& ec, size_t bytesSent) {
	    delete frame;
	});
}

bool tcpd_client::handle_recv_newplr(const scc& con, packet& pkt)
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
	if (pnum >= MAX_PLRS || active_connections[pnum] != NULL || i == MAX_PLRS) {
		// DoLog(pnum == MAX_PLRS ? "Server is full." : "Dropped connection.");
		return false;
	}
	pending_connections[i] = NULL;
	active_connections[pnum] = con;
	con->pnum = pnum;
	/*packet* reply = pktfty.make_out_packet<PT_JOIN_ACCEPT>(PLR_MASTER, PLR_BROADCAST, pkt.pktJoinReqCookie(), pnum, game_init_info);
	start_send(con, *reply);
	delete reply;*/
	//send_connect(con);
	return true;
}

bool tcpd_client::handle_recv_packet(const scc& con, packet& pkt)
{
	plr_t pkt_plr = con->pnum;

	if (pkt_plr != PLR_BROADCAST) {
		if (pkt_plr != pkt.pktSrc())
			return false;
		recv_local(pkt);
		// ensure the disconnecting turn is the last one
		if (connected_table[pkt_plr] & CON_LEAVING) {
			size_t numTurns = turn_queue[pkt_plr].size();
			if (numTurns > 1) {
				SNetTurn *turnLast = &turn_queue[pkt_plr][numTurns - 1];
				SNetTurn *turnPrev = &turn_queue[pkt_plr][numTurns - 2];
				auto turnId = turnLast->turn_id;
				if (turnId != 0) {
					// assert(turnPrev->turn_id == 0);
					turnLast->turn_id = 0; // turnPrev->turn_id
					turnPrev->turn_id = turnId;
					turnPrev->payload.swap(turnLast->payload);
				}
			}
		}
		return true;
	} else {
		return handle_recv_newplr(con, pkt);
	}
}

void tcpd_client::disconnect_net(plr_t pnum)
{
	// TODO: check active_connections and pending_connections?
	/*if (active_connections[pnum] != NULL) {
		drop_connection(active_connections[pnum]);
	}*/
}

void tcpd_client::drop_connection(const scc& con)
{
	plr_t i, pnum = con->pnum;

	if (pnum != PLR_BROADCAST) {
		// live connection
		if (active_connections[pnum] == con) {
			active_connections[pnum] = NULL;
			//packet* pkt = pktfty.make_out_packet<PT_DISCONNECT>(PLR_MASTER, PLR_BROADCAST, pnum, (leaveinfo_t)LEAVE_DROP);
			//send_packet(*pkt);
			//delete pkt;
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

void tcpd_client::send_packet(packet& pkt)
{
	if (pkt.pktType() == PT_TURN && pkt.pktTurn() != 0) {
		//plr_t dest = pkt.pktDest();
		//plr_t src = plr_self; //pkt.pktSrc();

		//assert(pkt.pktSrc() == src);
		//assert(dest == PLR_BROADCAST);
		//if (dest == PLR_BROADCAST) {
			for (int i = 0; i < MAX_PLRS; i++)
				if (/*i != src &&*/ active_connections[i] != NULL)
					start_send(active_connections[i], pkt);
		/*} else if (dest < MAX_PLRS) {
			if ((dest != src) && active_connections[dest] != NULL)
				start_send(active_connections[dest], pkt);
		}*/
	}

	tcp_client::send_packet(pkt);
}

void tcpd_client::close()
{
	int i;

	base_client::close();

	// close the server
	if (local_server != NULL) {
		// local_server->close();
		delete local_server;
		local_server = NULL;
	}
	// close the client
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
	recv_queue.clear();
	// prepare the client for possible re-connection
	ioc.restart();
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
