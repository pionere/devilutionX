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
	tcp_host_server(tcp_host_client* client, asio::io_context &ioc, buffer_t info, unsigned serverType);

	bool send_packet(packet &pkt);
private:
	tcp_host_client* local_client;
};

class tcp_host_client : public base {
public:
	tcp_host_client(int srvType);

	virtual bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info, char (&errorText)[256]);
	virtual bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]);
	virtual void SNetSendMessage(int receiver, const BYTE* data, unsigned size);
	virtual SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size);
	virtual turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]);
	virtual uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]);
	virtual unsigned SNetGetTurnsInTransit();
	virtual void SNetLeaveGame(int reason);

	virtual ~tcp_host_client() = default;

	virtual void make_default_gamename(char (&gamename)[128]);

	void receive_packet(packet &pkt);

protected:
	virtual void send_packet(packet &pkt);
	virtual void poll();
private:
	asio::io_context ioc;
	tcp_host_server* local_server = NULL;
	uint32_t hostTurn;
	int serverType;

	void close();
};

} //namespace net
DEVILUTION_END_NAMESPACE
#endif // NOHOSTING