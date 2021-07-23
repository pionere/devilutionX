#pragma once

#include <memory>
#include <array>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "packet.h"
#include "abstract_net.h"
#include "frame_queue.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class server_exception : public dvlnet_exception {
public:
	const char *what() const throw() override
	{
		return "Invalid player ID";
	}
};

class tcp_server {
public:
	tcp_server(asio::io_context &ioc, const char* bindAddr,
	    unsigned short port, const char* passwd, buffer_t info);
	void close();
	virtual ~tcp_server() = default;

	static void make_default_gamename(char (&gamename)[128]);
private:
	static constexpr int TIMEOUT_CONNECT = 30;
	static constexpr int TIMEOUT_ACTIVE = 60;

	struct client_connection {
		frame_queue recv_queue;
		buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);
		plr_t pnum = PLR_BROADCAST;
		asio::ip::tcp::socket socket;
		asio::steady_timer timer;
		int timeout;
		client_connection(asio::io_context &ioc)
		    : socket(ioc)
		    , timer(ioc)
		{
		}
	};

	typedef std::shared_ptr<client_connection> scc;

	asio::io_context &ioc;
	packet_factory pktfty;
	asio::ip::tcp::acceptor* acceptor = NULL;
	scc connections[MAX_PLRS];
	buffer_t game_init_info;

	scc make_connection();
	plr_t next_free();
	void start_accept();
	void handle_accept(const scc &con, const asio::error_code &ec);
	void start_recv(const scc &con);
	void handle_recv(const scc &con, const asio::error_code &ec, net_size_t bytes_read);
	void handle_recv_newplr(const scc &con, packet &pkt);
	void handle_recv_packet(packet &pkt);
	void send_connect(const scc &con);
	void send_packet(packet &pkt);
	void start_send(const scc &con, packet &pkt);
	void handle_send(const scc &con, const asio::error_code &ec, net_size_t bytes_sent);
	void start_timeout(const scc &con);
	void handle_timeout(const scc &con, const asio::error_code &ec);
	void drop_connection(const scc &con);
};

} //namespace net
DEVILUTION_END_NAMESPACE
