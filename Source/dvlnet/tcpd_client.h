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
	bool recv_accept(packet& pkt) override;
	void disconnect_net(plr_t pnum) override;
	void close() override;

private:
	asio::ip::tcp::acceptor acceptor = asio::ip::tcp::acceptor(ioc);
	asio::steady_timer connTimer = asio::steady_timer(ioc);
	scc nextcon;
	scc pending_connections[MAX_PLRS] = { };
	scc active_connections[MAX_PLRS] = { };

	void start_timeout();
	void handle_timeout(const asio::error_code& ec);
	plr_t next_free_conn();
	plr_t next_free_queue();
	void start_accept_conn();
	void handle_accept_conn(bool valid, const asio::error_code& ec);
	void start_recv_conn(const scc& con);
	void handle_recv_conn(const scc& con, const asio::error_code& ec, size_t bytesRead);
	void start_send(const scc& con, packet& pkt);
	bool handle_recv_newplr(const scc& con, packet& pkt);
	bool handle_recv_packet(const scc& con, packet& pkt);
	void drop_connection(const scc& con);
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
