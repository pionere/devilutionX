#pragma once
#ifdef TCPIP
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "frame_queue.h"
#include "base_client.h"
#include "tcp_server.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcp_client : public base_client {
public:
	bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	~tcp_client() override = default;

	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;

protected:
	bool join_game();
	void poll() override;
	void send_packet(packet& pkt) override;
	void close() override;

protected:
	frame_queue recv_queue;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);

	asio::io_context ioc;
	asio::ip::tcp::socket sock = asio::ip::tcp::socket(ioc);
	tcp_server* local_server = NULL;

	void handle_recv(const asio::error_code& ec, size_t bytesRead);
	void start_recv();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
