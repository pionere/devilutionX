#pragma once
#ifdef TCPIP
#include "tcp_client.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcpd_client : public tcp_client {
public:
	bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	~tcpd_client() override = default;

protected:
	void send_packet(packet& pkt) override;
	void recv_connect(packet& pkt) override;
	void disconnect_net(plr_t pnum) override;
	void close() override;

private:
	asio::ip::tcp::acceptor acceptor = asio::ip::tcp::acceptor(ioc);
	asio::steady_timer connTimer = asio::steady_timer(ioc);
	tcp_server::scc nextcon;
	tcp_server::scc pending_connections[MAX_PLRS] = { };
	tcp_server::scc active_connections[MAX_PLRS] = { };

	void start_timeout();
	void handle_timeout(const asio::error_code& ec);
	plr_t next_free_conn();
	plr_t next_free_queue();
	void start_accept_conn();
	void handle_accept_conn(bool valid, const asio::error_code& ec);
	void start_recv_conn(const tcp_server::scc& con);
	void handle_recv_conn(const tcp_server::scc& con, const asio::error_code& ec, size_t bytesRead);
	void start_send(const tcp_server::scc& con, packet& pkt);
	bool handle_recv_newplr(const tcp_server::scc& con, packet& pkt);
	bool handle_recv_packet(const tcp_server::scc& con, packet& pkt);
	void drop_connection(const tcp_server::scc& con);
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
