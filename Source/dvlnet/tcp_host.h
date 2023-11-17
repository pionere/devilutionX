#pragma once
#ifndef NOHOSTING
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "frame_queue.h"
#include "base.h"
#include "dvlnet/tcp_server.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcp_host_client;

class tcp_host_server : public tcp_server {
public:
	tcp_host_server(tcp_host_client* client, asio::io_context& ioc, buffer_t info, unsigned serverType);

	bool send_packet(packet& pkt) override;

private:
	tcp_host_client* local_client;
};

class tcp_host_client : public base {
public:
	tcp_host_client(int srvType);

	bool create_game(const char* addrstr, unsigned port, const char* passwd, _uigamedata* gameData, char (&errorText)[256]) override;
	bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size) override;
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) override;
	void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size) override;
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) override;
	uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]) override;
	unsigned SNetGetTurnsInTransit() override;

	~tcp_host_client() override = default;

	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN+ 1]) override;

	void receive_packet(packet& pkt);

protected:
	void send_packet(packet& pkt) override;
	void poll() override;
	void close() override;

private:
	asio::io_context ioc;
	tcp_host_server* local_server = NULL;
	uint32_t hostTurn;
	int serverType;

};

} //namespace net
DEVILUTION_END_NAMESPACE
#endif // NOHOSTING
