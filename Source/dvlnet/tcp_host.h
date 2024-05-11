#pragma once
#ifndef NOHOSTING
#include <asio/ts/internet.hpp>
#include <asio/ts/io_context.hpp>
#include <asio/ts/net.hpp>

#include "frame_queue.h"
#include "base_client.h"
#include "dvlnet/tcp_server.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class tcp_host_server : public tcp_server {
public:
	tcp_host_server(base_client& client, asio::io_context& ioc, packet_factory& pktfty, SNetGameData& gameinfo, unsigned serverType);

	bool send_packet(packet& pkt) override;
};

class tcp_host_client : public base_client {
public:
	tcp_host_client(int srvType);

	bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;
	void SNetSendMessage(int receiver, const BYTE* data, unsigned size) override;
	SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]) override;
	void SNetSendTurn(turn_t turn, const BYTE* data, unsigned size) override;
	turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]) override;
	unsigned SNetGetTurnsInTransit() override;

	~tcp_host_client() override = default;

	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN+ 1]) override;

protected:
	void send_packet(packet& pkt) override;
	void poll() override;
	void close() override;

private:
	asio::io_context ioc;
	tcp_host_server* local_server = NULL;
	int serverType; // server_type
};

} //namespace net
DEVILUTION_END_NAMESPACE
#endif // NOHOSTING
