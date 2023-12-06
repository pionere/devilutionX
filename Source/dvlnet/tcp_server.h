#pragma once
#ifdef TCPIP
#include <string>
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "base_client.h"
#include "packet.h"
#include "frame_queue.h"

// define throw_exception to compile asio with ASIO_NO_EXCEPTIONS
#if 1
#include <asio/detail/throw_exception.hpp>

namespace asio::detail {
template <typename Exception>
void throw_exception(Exception const &e)
{
	asio_error(dvl::ERR_APP_ASIO, e.what());
}

} // namespace asio::detail
#endif

DEVILUTION_BEGIN_NAMESPACE
namespace net {

typedef struct client_connection {
	frame_queue recv_queue;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);
	plr_t pnum = PLR_BROADCAST;
	asio::ip::tcp::socket socket;
	int timeout;
	client_connection(asio::io_context& ioc)
		: socket(ioc)
	{
	}
} client_connection;
typedef std::shared_ptr<client_connection> scc;
scc make_shared_cc(asio::io_context& ioc);

class tcp_server {
public:
	tcp_server(base_client& client, asio::io_context& ioc, packet_factory& pktfty, SNetGameData& gameinfo, unsigned serverType);
	bool setup_server(const char* bindAddr, unsigned short port, char (&errorText)[256]);
	void close();
	virtual ~tcp_server() = default;

	static void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]);
	static void connect_acceptor(asio::ip::tcp::acceptor& acceptor, const asio::ip::tcp::endpoint& ep, asio::error_code& ec);
	static void connect_socket(asio::ip::tcp::socket& sock, const char* addrstr, unsigned port, asio::io_context& ioc, asio::error_code& ec);

protected:
	base_client& local_client; // TODO: tcp_client would be better, but tcp_host_client is not one...

	virtual bool send_packet(packet& pkt);

private:
	asio::io_context& ioc;
	asio::ip::tcp::acceptor acceptor;
	asio::steady_timer connTimer;
	packet_factory& pktfty;
	scc nextcon;
	scc pending_connections[MAX_PLRS] = { };
	scc active_connections[MAX_PLRS] = { };
	int ghost_connections[MAX_PLRS] = { };
	SNetGameData& game_init_info;
	unsigned serverType;

	static void endpoint_to_buffer(const scc& con, buffer_t& buf);

	plr_t next_free_conn();
	plr_t next_free_queue();
	void start_accept();
	void handle_accept(bool valid, const asio::error_code& ec);
	void start_recv(const scc& con);
	void handle_recv(const scc& con, const asio::error_code& ec, size_t bytesRead);
	bool recv_ctrl(packet& pkt, const scc& con);
	bool handle_recv_packet(packet& pkt, const scc& con);
	//void send_connect(const scc& con);
	void start_send(packet& pkt, const scc& con);
	void start_timeout();
	void handle_timeout(const asio::error_code& ec);
	void drop_connection(const scc& con);
};

} //namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
