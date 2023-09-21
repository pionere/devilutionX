#pragma once
#ifdef TCPIP
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "frame_queue.h"
#include "base.h"
#include "tcp_server.h"

#include <asio/detail/throw_exception.hpp>

#define ErrAsio(message) dvl::app_fatal("ASIO Error: %s", message)

/*namespace dvl {

extern void app_fatal(const char* pszFmt, ...);

} // namespace dvl*/

namespace asio::detail {

template <typename Exception>
void throw_exception(Exception const &e)
{
  ErrAsio(e.what());
}

} // namespace asio::detail

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcp_client : public base {
public:
	bool create_game(const char* addrstr, unsigned port, const char* passwd, _uigamedata* gameData, char (&errorText)[256]) override;
	bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	void SNetLeaveGame(int reason) override;

	~tcp_client() override = default;

	void make_default_gamename(char (&gamename)[128]) override;

protected:
	void poll() override;
	void send_packet(packet& pkt) override;

private:
	frame_queue recv_queue;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);

	asio::io_context ioc;
	asio::ip::tcp::socket sock = asio::ip::tcp::socket(ioc);
	tcp_server* local_server = NULL;

	void handle_recv(const asio::error_code& ec, size_t bytesRead);
	void start_recv();
	void close();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
