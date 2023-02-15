#include "tcp_client.h"
#ifdef TCPIP
#include <string>
#include <SDL.h>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <system_error>

#include <asio/connect.hpp>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

bool tcp_client::create_game(const char* addrstr, unsigned port, const char* passwd, _uigamedata* gameData, char (&errorText)[256])
{
	setup_gameinfo(gameData);
	local_server = new tcp_server(ioc, game_init_info, SRV_BASIC);
	if (local_server->setup_server(addrstr, port, passwd, errorText)) {
		return join_game(addrstr, port, passwd, errorText);
	}
	close();
	return false;
}

bool tcp_client::join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	int i;
	constexpr int MS_SLEEP = 10;
	constexpr int NUM_SLEEP = 250;

	setup_password(passwd);
	plr_self = PLR_BROADCAST;
	memset(connected_table, 0, sizeof(connected_table));
	randombytes_buf(reinterpret_cast<unsigned char*>(&cookie_self),
	    sizeof(cookie_t));
	asio::error_code err;
	tcp_server::connect_socket(sock, addrstr, port, ioc, err);
	if (err) {
		SStrCopy(errorText, err.message().c_str(), lengthof(errorText));
		close();
		return false;
	}
	start_recv();

	auto pkt = pktfty.make_out_packet<PT_JOIN_REQUEST>(PLR_BROADCAST, PLR_MASTER, cookie_self);
	send_packet(*pkt);
	for (i = 0; i < NUM_SLEEP; i++) {
		poll();
		if (plr_self != PLR_BROADCAST)
			return true; // join successful
		SDL_Delay(MS_SLEEP);
	}
	if (i == NUM_SLEEP)
		copy_cstr(errorText, "Unable to connect");
	close();
	return false;
}

void tcp_client::poll()
{
	asio::error_code err;
	ioc.poll(err);
	assert(!err);
}

void tcp_client::handle_recv(const asio::error_code& ec, size_t bytesRead)
{
	if (ec || bytesRead == 0) {
		// error in recv from server
		// returning and doing nothing should be the same
		// as if all connections to other clients were lost
		return;
	}
	recv_buffer.resize(bytesRead);
	recv_queue.write(std::move(recv_buffer));
	recv_buffer.resize(frame_queue::MAX_FRAME_SIZE);
	while (recv_queue.packet_ready()) {
		auto pkt = pktfty.make_in_packet(recv_queue.read_packet());
		// TODO: validate server-messages? drop server connection?
		assert(pkt != NULL);
		recv_local(*pkt);
	}
	start_recv();
}

void tcp_client::start_recv()
{
	sock.async_receive(asio::buffer(recv_buffer),
	    std::bind(&tcp_client::handle_recv, this,
	        std::placeholders::_1, std::placeholders::_2));
}

void tcp_client::send_packet(packet& pkt)
{
	const auto* frame = frame_queue::make_frame(pkt.encrypted_data());
	auto buf = asio::buffer(*frame);
	asio::async_write(sock, buf, [frame](const asio::error_code& ec, size_t bytesSent) {
		delete frame;
	});
}

void tcp_client::close()
{
	// close the server
	if (local_server != NULL) {
		local_server->close();
		delete local_server;
		local_server = NULL;
	}
	// close the client
	recv_queue.clear();
	asio::error_code err;
	sock.shutdown(asio::socket_base::shutdown_both, err);
	err.clear();
	sock.close(err);
	poll();
	// prepare the client for possible re-connection
	ioc.restart();
}

void tcp_client::SNetLeaveGame(int reason)
{
	base::SNetLeaveGame(reason);
	poll();
	close();
}

void tcp_client::make_default_gamename(char (&gamename)[128])
{
	tcp_server::make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // TCPIP
