#pragma once

#include <memory>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "frame_queue.h"
#include "base.h"
#include "tcp_server.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcp_client : public base {
public:
	bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info);
	bool join_game(const char* addrstr, unsigned port, const char* passwd);

	virtual void poll();
	virtual void send(packet &pkt);

	virtual void SNetLeaveGame(int reason);

	virtual ~tcp_client() = default;

	virtual void make_default_gamename(char (&gamename)[128]);

private:
	frame_queue recv_queue;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);

	asio::io_context ioc;
	asio::ip::tcp::socket sock = asio::ip::tcp::socket(ioc);
	tcp_server* local_server = NULL;

	void handle_recv(const asio::error_code &ec, net_size_t bytesRead);
	void start_recv();
	void handle_send(const asio::error_code &ec, net_size_t bytesSent);
	void close();
};

} // namespace net
DEVILUTION_END_NAMESPACE
