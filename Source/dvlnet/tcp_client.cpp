#include "tcp_client.h"

#include <SDL.h>
#include <exception>
#include <functional>
#include <memory>
#include <sodium.h>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <asio/connect.hpp>

DEVILUTION_BEGIN_NAMESPACE
namespace net {

bool tcp_client::create(const std::string &addrstr, unsigned port, const std::string &passwd)
{
	try {
		local_server = std::make_unique<tcp_server>(ioc, addrstr, port, passwd);
		return join(local_server->localhost_self(), port, passwd);
	} catch (std::system_error &e) {
		SDL_SetError("%s", e.what());
		return false;
	}
}

bool tcp_client::join(const std::string &addrstr, unsigned port, const std::string &passwd)
{
	constexpr int MsSleep = 10;
	constexpr int NoSleep = 250;

	setup_password(passwd);
	try {
		std::string strPort = std::to_string(port);
		asio::connect(sock, resolver.resolve(addrstr, strPort));
		asio::ip::tcp::no_delay option(true);
		sock.set_option(option);
	} catch (std::exception &e) {
		SDL_SetError("%s", e.what());
		return false;
	}
	start_recv();
	{
		randombytes_buf(reinterpret_cast<unsigned char *>(&cookie_self),
		    sizeof(cookie_t));
		auto pkt = pktfty->make_out_packet<PT_JOIN_REQUEST>(PLR_BROADCAST,
		    PLR_MASTER, cookie_self,
		    game_init_info);
		send(*pkt);
		for (auto i = 0; i < NoSleep; ++i) {
			try {
				poll();
			} catch (const std::runtime_error &e) {
				if (plr_self != PLR_BROADCAST) {
					connected_table[plr_self] = false;
					plr_self = PLR_BROADCAST;
				}
				SDL_SetError("%s", e.what());
				return false;
			}
			if (plr_self != PLR_BROADCAST)
				return true; // join successful
			SDL_Delay(MsSleep);
		}
	}
	SDL_SetError("Unable to connect");
	return false;
}

void tcp_client::poll()
{
	ioc.poll();
}

void tcp_client::handle_recv(const asio::error_code &error, net_size_t bytesRead)
{
	if (error) {
		// error in recv from server
		// returning and doing nothing should be the same
		// as if all connections to other clients were lost
		return;
	}
	if (bytesRead == 0) {
		throw std::runtime_error("error: read 0 bytes from server");
	}
	recv_buffer.resize(bytesRead);
	recv_queue.write(std::move(recv_buffer));
	recv_buffer.resize(frame_queue::max_frame_size);
	while (recv_queue.packet_ready()) {
		auto pkt = pktfty->make_in_packet(recv_queue.read_packet());
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

void tcp_client::handle_send(const asio::error_code &error, net_size_t bytesSent)
{
	// empty for now
}

void tcp_client::send(packet &pkt)
{
	const auto *frame = new buffer_t(frame_queue::make_frame(pkt.data()));
	auto buf = asio::buffer(*frame);
	asio::async_write(sock, buf, [this, frame](const asio::error_code &error, size_t bytesSent) {
		handle_send(error, bytesSent);
		delete frame;
	});
}

void tcp_client::SNetLeaveGame(int reason)
{
	base::SNetLeaveGame(reason);
	poll();
	if (local_server != NULL)
		local_server->close();
	sock.close();
}

std::string tcp_client::make_default_gamename()
{
	char pszGameName[128] = "0.0.0.0";

	getIniValue("Network", "Bind Address", pszGameName, sizeof(pszGameName) - 1);
	return std::string(pszGameName);
}

tcp_client::~tcp_client()
{
}

} // namespace net
DEVILUTION_END_NAMESPACE
