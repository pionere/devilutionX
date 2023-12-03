#pragma once
#ifdef ZEROTIER
#include <map>

#include "base_client.h"
#include "protocol_zt.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class zt_client : public base_client {
public:
	bool setup_game(_uigamedata* gameData, const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]) override;

	void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]) override;
	void get_gamelist(std::vector<SNetZtGame>& games);
	bool network_ready();

	virtual ~zt_client() = default;

protected:
	void poll() override;
	void send_packet(packet& pkt) override;
	bool recv_connect(packet& pkt) override;
	bool recv_accept(packet& pkt) override;
	void disconnect_net(plr_t pnum) override;
	void close() override;

private:
	protocol_zt proto;
	typedef typename protocol_zt::endpoint endpoint;
	typedef std::array<char, NET_MAX_GAMENAME_LEN + 1> gamename_t;
	typedef struct game_details {
		SNetZtGame ztGamedata;
		endpoint master;
		Uint32 timeout;
	} game_details;

	gamename_t gamename;
	std::map<gamename_t, game_details> game_list;

	plr_t get_master();
	plr_t next_free_conn();
	void disconnect_peer(const endpoint& peer);
	void send_info_request();
	void handle_join_request(packet& pkt, const endpoint& sender);
	void handle_recv_packet(packet& pkt, const endpoint& sender);
	void recv_ctrl(packet& pkt, const endpoint& sender);

	bool wait_network();
	bool wait_join();
	bool wait_firstpeer(endpoint& peer);
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
