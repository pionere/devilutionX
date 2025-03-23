#pragma once
#ifdef ZEROTIER
#include <string>
#include <deque>
#include <array>

#include "dvlnet/frame_queue.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

typedef enum client_status {
	CS_INACTIVE,
	CS_ACTIVE,
	CS_ACTIVE_SELF,
	CS_PENDING,
} client_status;

class zt_client;

class protocol_zt {
public:
	typedef struct endpoint {
		std::array<unsigned char, 16> addr = { };
		static constexpr unsigned str_len() { return 16 * sizeof(decltype(addr)::value_type); };

		bool operator==(const endpoint& rhs) const
		{
			return addr == rhs.addr;
		}

		bool operator!=(const endpoint& rhs) const
		{
			return !(*this == rhs);
		}

		void clear_addr();
		void from_string(const std::string& str);
		void from_addr(const unsigned char* src_addr);
		void to_addr(unsigned char* dest_addr) const;
		void to_buffer(buffer_t& buf) const;
	} endpoint;
	typedef struct peer_connection {
		int status = CS_INACTIVE; // client_status
		int sock = -1;  // connected socket-id
		endpoint peer;
		std::deque<buffer_t*> send_frame_queue;
		frame_queue recv_queue;
	} peer_connection;

	peer_connection active_connections[MAX_PLRS] = { };

	protocol_zt();
	~protocol_zt();
	void poll(zt_client* client);
	void disconnect(int pnum);
	void send(int pnum, const buffer_t& data);
	bool send_oob(const endpoint& peer, const buffer_t& data) const;
	bool send_oob_mc(const buffer_t& data) const;
	void connect_ep(const unsigned char* addr, int pnum);
	void accept_ep(const unsigned char* addr, int pnum);
	void accept_self(int pnum);
	bool network_online();
	static void make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1]);
	void close();

private:
	static constexpr uint16_t DEFAULT_PORT = 6112;

	typedef struct pending_connection {
		int sock; // connected socket-id + 1
		endpoint peer;
		uint32_t timeout;
	} pending_connection;
	buffer_t recv_buffer = buffer_t(frame_queue::MAX_FRAME_SIZE);

	pending_connection pending_connections[MAX_PLRS] = { };
	int fd_tcp = -1;
	int fd_udp = -1;

	static void set_nonblock(int fd);
	static void set_nodelay(int fd);
	static void set_reuseaddr(int fd);

	void send_queued_peer(peer_connection& pc);
	void recv_peer(peer_connection& pc);
	void send_queued_all();
	void recv_from_peers(zt_client* client);
	void recv_from_udp(zt_client* client);
	void accept_all();
};

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
