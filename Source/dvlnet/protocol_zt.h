#pragma once
#ifdef ZEROTIER
#include <string>
#include <set>
#include <atomic>
#include <deque>
#include <map>
#include <array>
#include <algorithm>

#include "dvlnet/frame_queue.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

class protocol_zt {
public:
	class endpoint {
	public:
		std::array<unsigned char, 16> addr = {};

		explicit operator bool() const
		{
			auto empty = std::array<unsigned char, 16> {};
			return (addr != empty);
		}

		bool operator==(const endpoint& rhs) const
		{
			return addr == rhs.addr;
		}

		bool operator!=(const endpoint& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator<(const endpoint& rhs) const
		{
			return addr < rhs.addr;
		}

		void from_string(const std::string& str);
		void from_addr(const unsigned char* src_addr);
		void to_addr(unsigned char* dest_addr) const;
	};

	protocol_zt();
	~protocol_zt();
	void disconnect(const endpoint& peer);
	bool send(const endpoint& peer, const buffer_t& data);
	bool send_oob(const endpoint& peer, const buffer_t& data) const;
	bool send_oob_mc(const buffer_t& data) const;
	bool recv(endpoint& peer, buffer_t& data);
	bool get_disconnected(endpoint& peer);
	bool network_online();
	static void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]);

private:
	static constexpr uint32_t PKTBUF_LEN = 65536;
	static constexpr uint16_t DEFAULT_PORT = 6112;

	struct peer_state {
		int fd = -1;
		std::deque<buffer_t*> send_queue;
		frame_queue recv_queue;
	};

	std::deque<std::pair<endpoint, buffer_t>> oob_recv_queue;
	std::deque<endpoint> disconnect_queue;

	std::map<endpoint, peer_state> peer_list;
	int fd_tcp = -1;
	int fd_udp = -1;

	static uint64_t current_ms();
	void close_all();

	static void set_nonblock(int fd);
	static void set_nodelay(int fd);
	static void set_reuseaddr(int fd);

	bool send_queued_peer(const endpoint& peer);
	bool recv_peer(const endpoint& peer);
	bool send_queued_all();
	bool recv_from_peers();
	bool recv_from_udp();
	bool accept_all();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
