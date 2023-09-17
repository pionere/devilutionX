#pragma once
#ifdef TCPIP
#include <string>
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "packet.h"
#include "frame_queue.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcp_server {
	friend class tcpd_client;

public:
	tcp_server(asio::io_context& ioc, buffer_t info, unsigned serverType);
	bool setup_server(const char* bindAddr, unsigned short port, const char* passwd, char (&errorText)[256]);
	void close();
	~tcp_server() = default;

	static void make_default_gamename(char (&gamename)[128]);
	static void connect_acceptor(asio::ip::tcp::acceptor& acceptor, const asio::ip::tcp::endpoint& ep, asio::error_code& ec);
	static void connect_socket(asio::ip::tcp::socket& sock, const char* addrstr, unsigned port, asio::io_context& ioc, asio::error_code& ec);

private:
	static constexpr int TIMEOUT_CONNECT = 30;
	static constexpr int TIMEOUT_ACTIVE = 60;
	static constexpr int PORT_LENGTH = 5;
	struct client_connection {
		frame_queue recv_queue;
		buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);
		plr_t pnum = PLR_BROADCAST;
		asio::ip::tcp::socket socket;
		int timeout;
		client_connection(asio::io_context& ioc)
		    : socket(ioc)
		{
		}
	};

	typedef std::shared_ptr<client_connection> scc;

	asio::io_context& ioc;
	asio::ip::tcp::acceptor acceptor;
	asio::steady_timer connTimer;
	packet_factory pktfty;
	scc nextcon;
	scc pending_connections[MAX_PLRS] = { };
	scc connections[MAX_PLRS] = { };
	buffer_t game_init_info;
	unsigned serverType;

	static scc make_connection(asio::io_context& ioc);
	static void endpoint_to_string(const scc& con, std::string& addr);

	plr_t next_free_conn();
	plr_t next_free_queue();
	void start_accept();
	void handle_accept(bool valid, const asio::error_code& ec);
	void start_recv(const scc& con);
	void handle_recv(const scc& con, const asio::error_code& ec, size_t bytesRead);
	bool handle_recv_newplr(const scc& con, packet& pkt);
	bool handle_recv_packet(const scc& con, packet& pkt);
	//void send_connect(const scc& con);
	void start_send(const scc& con, packet& pkt);
	void start_timeout();
	void handle_timeout(const asio::error_code& ec);
	void drop_connection(const scc& con);

protected:
	virtual bool send_packet(packet& pkt);
};

} //namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
