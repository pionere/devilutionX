#include "loopback.h"
#include "utils/stubs.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

bool loopback::create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info)
{
	setup_gameinfo(std::move(info));
	auto reply = pktfty.make_fake_out_packet<PT_JOIN_ACCEPT>(PLR_MASTER, PLR_BROADCAST,
		cookie_self, plr_single,
		game_init_info);

	base::recv_local(*reply);
	return true;
}

bool loopback::join_game(const char* addrstr, unsigned port, const char* passwd)
{
	ABORT();
}

void loopback::poll()
{
	ABORT();
}

void loopback::send(packet &pkt)
{
	ABORT();
}

bool loopback::SNetReceiveMessage(int* sender, BYTE** data, unsigned* size)
{
	if (message_queue.empty())
		return false;
	message_last = message_queue.front();
	message_queue.pop();
	*sender = plr_single;
	*size = message_last.size();
	*data = message_last.data();
	return true;
}

void loopback::SNetSendMessage(int receiver, const BYTE* data, unsigned size)
{
	if (receiver == plr_single || receiver == SNPLAYER_ALL) {
		buffer_t message(data, data + size);
		message_queue.push(message);
	}
}

bool loopback::SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
{
	// todo: check that this is safe
	//for (auto i = 0; i < MAX_PLRS; ++i) {
	//	turns[i] = NULL;
	//	status[i] = 0;
	//}
	return true;
}

void loopback::SNetSendTurn(uint32_t turn)
{
}

void loopback::SNetLeaveGame(int reason)
{
}

void loopback::SNetDropPlayer(int playerid)
{
}

uint32_t loopback::SNetGetOwnerTurnsWaiting()
{
	return 0;
}

uint32_t loopback::SNetGetTurnsInTransit()
{
	return 0;
}

void loopback::make_default_gamename(char (&gamename)[128])
{
	copy_cstr(gamename, "loopback");
}

} // namespace net
DEVILUTION_END_NAMESPACE
