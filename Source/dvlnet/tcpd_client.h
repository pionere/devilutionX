#pragma once
#ifdef TCPIP
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "dvlnet/frame_queue.h"
#include "dvlnet/base.h"
#include "dvlnet/tcp_server.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcpd_client : public base {
public:
	bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info);
	bool join_game(const char* addrstr, unsigned port, const char* passwd);

	virtual void SNetLeaveGame(int reason);

	virtual ~tcpd_client() = default;

	virtual void make_default_gamename(char (&gamename)[128]);

protected:
	virtual void poll();
	virtual void send_packet(packet &pkt);
	void recv_connect(packet &pkt);

private:
	frame_queue recv_queue;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);

	asio::io_context ioc;
	asio::ip::tcp::socket sock = asio::ip::tcp::socket(ioc);
	tcp_server* local_server = NULL;

	asio::ip::tcp::acceptor acceptor = asio::ip::tcp::acceptor(ioc);
	asio::steady_timer connTimer = asio::steady_timer(ioc);
	tcp_server::scc nextcon;
	tcp_server::scc pending_connections[MAX_PLRS] = { };
	tcp_server::scc connections[MAX_PLRS] = { };

	void start_timeout();
	void handle_timeout(const asio::error_code &ec);
	plr_t next_free_conn();
	plr_t next_free_queue();
	void start_accept_conn();
	void handle_accept_conn(bool valid, const asio::error_code &ec);
	void start_recv_conn(const tcp_server::scc &con);
	void handle_recv_conn(const tcp_server::scc &con, const asio::error_code &ec, size_t bytesRead);
	void start_send(const tcp_server::scc &con, packet &pkt);
	bool handle_recv_newplr(const tcp_server::scc &con, packet &pkt);
	bool handle_recv_packet(const tcp_server::scc &con, packet &pkt);
	void drop_connection(const tcp_server::scc &con);
	void disconnect_net(plr_t pnum);

	void handle_recv(const asio::error_code &ec, size_t bytesRead);
	void start_recv();
	void close();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif