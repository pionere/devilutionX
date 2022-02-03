#pragma once
#ifdef TCPIP
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
	bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info, char (&errorText)[256]);
	bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]);

	virtual void SNetLeaveGame(int reason);

	virtual ~tcp_client() = default;

	virtual void make_default_gamename(char (&gamename)[128]);

protected:
	virtual void poll();
	virtual void send_packet(packet &pkt);

private:
	frame_queue recv_queue;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);

	asio::io_context ioc;
	asio::ip::tcp::socket sock = asio::ip::tcp::socket(ioc);
	tcp_server* local_server = NULL;

	void handle_recv(const asio::error_code &ec, size_t bytesRead);
	void start_recv();
	void close();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP