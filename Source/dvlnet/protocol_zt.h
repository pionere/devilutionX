#pragma once
#ifdef ZEROTIER
#include <string>
#include <set>
#include <deque>
#include <map>
#include <array>
#include <algorithm>

#include "dvlnet/frame_queue.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

typedef enum client_status {
	CS_INACTIVE,
	CS_ACTIVE,
	CS_PENDING,
} client_status;

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
		void to_buffer(buffer_t& buf) const;
	};

	protocol_zt();
	~protocol_zt();
	void disconnect(int pnum);
	void send(int pnum, const buffer_t& data);
	bool send_oob(const endpoint& peer, const buffer_t& data) const;
	bool send_oob_mc(const buffer_t& data) const;
	bool recv(endpoint& peer, buffer_t& data);
	bool get_disconnected(endpoint& peer);
	void connect_ep(const endpoint& peer, int pnum);
	void accept_ep(const endpoint& peer, int pnum);
	bool network_online();
	static void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]);
	void close();

private:
	static constexpr uint16_t DEFAULT_PORT = 6112;

	typedef struct peer_connection {
		int status = CS_INACTIVE; // client_status
		int sock = -1;  // connected socket-id
		endpoint peer;
		std::deque<buffer_t*> send_frame_queue;
		frame_queue recv_queue;
	} peer_connection;
	typedef struct pending_connection {
		int sock; // connected socket-id + 1
		endpoint peer;
		uint32_t timeout;
	} pending_connection;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);
	std::deque<std::pair<endpoint, buffer_t>> oob_recv_queue;
	std::deque<endpoint> disconnect_queue;

	peer_connection active_connections[MAX_PLRS] = { };
	pending_connection pending_connections[MAX_PLRS] = { };
	int fd_tcp = -1;
	int fd_udp = -1;

	static void set_nonblock(int fd);
	static void set_nodelay(int fd);
	static void set_reuseaddr(int fd);

	bool send_queued_peer(peer_connection& pc);
	bool recv_peer(peer_connection& pc);
	bool send_queued_all();
	bool recv_from_peers();
	bool recv_from_udp();
	bool accept_all();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
