#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <cstring>
#ifdef NETENCRYPT
#include <sodium.h>
#endif

#include "abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

enum packet_type : uint8_t {
	// clang-format off
	PT_MESSAGE,
	PT_TURN,
	PT_JOIN_REQUEST,
	PT_JOIN_ACCEPT,
	PT_CONNECT,			// tcp, zt-only
	PT_DISCONNECT,
	PT_INFO_REQUEST,	// zt-only
	PT_INFO_REPLY,		// zt-only
	// clang-format on
};

#ifdef NETENCRYPT
typedef struct key_t {
	BYTE data[crypto_secretbox_KEYBYTES];
} key_t;
#else
// Stub out the key_t definition as we're not doing any encryption.
using key_t = uint8_t;
#endif

#pragma pack(push, 1)
typedef struct NetPktHdr {
	BYTE m_type; // packet_type
	BYTE m_src;  // plr_t
	BYTE m_dest; // plr_t
} NetPktHdr;

typedef struct NetPktMessage {
	NetPktHdr npHdr;
	//BYTE m_message[0];
} NetPktMessage;

typedef struct NetPktTurn {
	NetPktHdr npHdr;
	LE_UINT32 m_turn;
	//BYTE m_message[0];
} NetPktTurn;

typedef struct NetPktJoinRequest {
	NetPktHdr npHdr;
	LE_UINT32 m_cookie;
} NetPktJoinRequest;

typedef struct NetPktJoinAccept {
	NetPktHdr npHdr;
	LE_UINT32 m_cookie;
	BYTE m_newplr; // plr_t
	SNetGameData m_info;
	BYTE m_plrmask;
	LE_UINT32 m_turn;
	// BYTE m_addrs[MAX_PLRS][0]; // 16 in case of zt, host:port text in case of direct-tcp
} NetPktJoinAccept;

typedef struct NetPktConnect {
	NetPktHdr npHdr;
	BYTE m_newplr; // plr_t
	LE_UINT32 m_turn;
	//BYTE m_addr[0]; // 16 in case of zt
} NetPktConnect;

typedef struct NetPktDisconnect {
	NetPktHdr npHdr;
	BYTE m_plr; // plr_t
} NetPktDisconnect;

typedef struct NetPktInfoRequest {
	NetPktHdr npHdr;
} NetPktInfoRequest;

typedef struct NetPktInfoReply {
	NetPktHdr npHdr;
	SNetZtGame m_info;
} NetPktInfoReply;
#pragma pack(pop)

class packet {
protected:
	const key_t& key;
	buffer_t encrypted_buffer;
	buffer_t decrypted_buffer;

public:
	packet(const key_t& k)
	    : key(k) {};

	const buffer_t& encrypted_data() const;

#if DEV_MODE || INET_MODE || DEBUG_MODE
	bool validate();
#endif

	packet_type pktType() const
	{
		return (packet_type)reinterpret_cast<const NetPktHdr*>(decrypted_buffer.data())->m_type;
	}
	plr_t pktSrc() const
	{
		return reinterpret_cast<const NetPktHdr*>(decrypted_buffer.data())->m_src;
	}
	plr_t pktDest() const
	{
		return reinterpret_cast<const NetPktHdr*>(decrypted_buffer.data())->m_dest;
	}
	// PT_MESSAGE
	buffer_t::const_iterator pktMessageBegin() const
	{
		return decrypted_buffer.begin() + sizeof(NetPktHdr);
	}
	buffer_t::const_iterator pktMessageEnd() const
	{
		return decrypted_buffer.end();
	}
	// PT_TURN
	turn_t pktTurn() const
	{
		return reinterpret_cast<const NetPktTurn*>(decrypted_buffer.data())->m_turn;
	}
	buffer_t::const_iterator pktTurnBegin() const
	{
		return decrypted_buffer.begin() + sizeof(NetPktTurn);
	}
	buffer_t::const_iterator pktTurnEnd() const
	{
		return decrypted_buffer.end();
	}
	// PT_JOIN_REQUEST
	cookie_t pktJoinReqCookie() const
	{
		return reinterpret_cast<const NetPktJoinRequest*>(decrypted_buffer.data())->m_cookie;
	}
	// PT_JOIN_ACCEPT
	plr_t pktJoinAccPlr() const
	{
		return reinterpret_cast<const NetPktJoinAccept*>(decrypted_buffer.data())->m_newplr;
	}
	cookie_t pktJoinAccCookie() const
	{
		return reinterpret_cast<const NetPktJoinAccept*>(decrypted_buffer.data())->m_cookie;
	}
	const SNetGameData& pktJoinAccInfo() const
	{
		return reinterpret_cast<const NetPktJoinAccept*>(decrypted_buffer.data())->m_info;
	}
	plr_t pktJoinAccMsk() const
	{
		return reinterpret_cast<const NetPktJoinAccept*>(decrypted_buffer.data())->m_plrmask;
	}
	turn_t pktJoinAccTurn() const
	{
		return reinterpret_cast<const NetPktJoinAccept*>(decrypted_buffer.data())->m_turn;
	}
	buffer_t::const_iterator pktJoinAccAddrsBegin() const
	{
		return decrypted_buffer.begin() + sizeof(NetPktJoinAccept);
	}
	buffer_t::const_iterator pktJoinAccAddrsEnd() const
	{
		return decrypted_buffer.end();
	}
	// PT_INFO_REPLY
	const SNetZtGame& pktGameInfo() const
	{
		return reinterpret_cast<const NetPktInfoReply*>(decrypted_buffer.data())->m_info;
	}
	// PT_CONNECT
	plr_t pktConnectPlr() const
	{
		return reinterpret_cast<const NetPktConnect*>(decrypted_buffer.data())->m_newplr;
	}
	turn_t pktConnectTurn() const
	{
		return reinterpret_cast<const NetPktConnect*>(decrypted_buffer.data())->m_turn;
	}
	buffer_t::const_iterator pktConnectAddrBegin() const
	{
		return decrypted_buffer.begin() + sizeof(NetPktConnect);
	}
	buffer_t::const_iterator pktConnectAddrEnd() const
	{
		return decrypted_buffer.end();
	}
	// PT_DISCONNECT
	plr_t pktDisconnectPlr() const
	{
		return reinterpret_cast<const NetPktDisconnect*>(decrypted_buffer.data())->m_plr;
	}
};

class packet_in : public packet {
	friend class packet_factory;
	using packet::packet;

private:
	void create(buffer_t buf);
	bool decrypt();
};

class packet_out : public packet {
	friend class packet_factory;
	using packet::packet;

private:
	template <packet_type t, typename... Args>
	void create(Args... args);
	void encrypt();
};

template <>
inline void packet_out::create<PT_INFO_REQUEST>(plr_t s, plr_t d)
{
	decrypted_buffer.resize(sizeof(NetPktInfoRequest));
	NetPktInfoRequest* data = (NetPktInfoRequest*)decrypted_buffer.data();
	data->npHdr.m_type = PT_INFO_REQUEST;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
}

template <>
inline void packet_out::create<PT_INFO_REPLY>(plr_t s, plr_t d, const BYTE* gameinfo)
{
	decrypted_buffer.resize(sizeof(NetPktInfoReply));
	NetPktInfoReply* data = (NetPktInfoReply*)decrypted_buffer.data();
	data->npHdr.m_type = PT_INFO_REPLY;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	memcpy(&data->m_info, gameinfo, sizeof(SNetZtGame));
}

template <>
inline void packet_out::create<PT_MESSAGE>(plr_t s, plr_t d, const BYTE* msg, unsigned size)
{
	decrypted_buffer.resize(sizeof(NetPktHdr) + size);
	NetPktMessage* data = (NetPktMessage*)decrypted_buffer.data();
	data->npHdr.m_type = PT_MESSAGE;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	memcpy((BYTE*)data + sizeof(NetPktHdr), msg, size);
}

template <>
inline void packet_out::create<PT_TURN>(plr_t s, plr_t d, turn_t u, const BYTE* msg, unsigned size)
{
	decrypted_buffer.resize(sizeof(NetPktTurn) + size);
	NetPktTurn* data = (NetPktTurn*)decrypted_buffer.data();
	data->npHdr.m_type = PT_TURN;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_turn = u;
	memcpy((BYTE*)data + sizeof(NetPktTurn), msg, size);
}

template <>
inline void packet_out::create<PT_JOIN_REQUEST>(plr_t s, plr_t d, cookie_t c)
{
	decrypted_buffer.resize(sizeof(NetPktJoinRequest));
	NetPktJoinRequest* data = (NetPktJoinRequest*)decrypted_buffer.data();
	data->npHdr.m_type = PT_JOIN_REQUEST;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_cookie = c;
}

template <>
inline void packet_out::create<PT_JOIN_ACCEPT>(plr_t s, plr_t d, cookie_t c,
    plr_t n, const BYTE* gamedata, plr_t p, turn_t t, const BYTE* addrs, unsigned size)
{
	decrypted_buffer.resize(sizeof(NetPktJoinAccept) + size);
	NetPktJoinAccept* data = (NetPktJoinAccept*)decrypted_buffer.data();
	data->npHdr.m_type = PT_JOIN_ACCEPT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_cookie = c;
	data->m_newplr = n;
	data->m_plrmask = p;
	data->m_turn = t;
	memcpy(&data->m_info, gamedata, sizeof(SNetGameData));
	memcpy((BYTE*)data + sizeof(NetPktJoinAccept), addrs, size);
}

template <>
inline void packet_out::create<PT_CONNECT>(plr_t s, plr_t d, plr_t n, turn_t t, const BYTE* addr, unsigned size)
{
	decrypted_buffer.resize(sizeof(NetPktConnect) + size);
	NetPktConnect* data = (NetPktConnect*)decrypted_buffer.data();
	data->npHdr.m_type = PT_CONNECT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_newplr = n;
	data->m_turn = t;
	memcpy((BYTE*)data + sizeof(NetPktConnect), addr, size);
}

template <>
inline void packet_out::create<PT_DISCONNECT>(plr_t s, plr_t d, plr_t n)
{
	decrypted_buffer.resize(sizeof(NetPktDisconnect));
	NetPktDisconnect* data = (NetPktDisconnect*)decrypted_buffer.data();
	data->npHdr.m_type = PT_DISCONNECT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_plr = n;
}

class packet_factory {
	key_t key;

public:
	packet_factory() { clear_password(); };
	void setup_password(const char* passwd);
	void clear_password();
	packet* make_in_packet(buffer_t buf);
	template <packet_type t, typename... Args>
	packet* make_out_packet(Args... args);
	template <packet_type t, typename... Args>
	packet* make_fake_packet(Args... args);
};

inline packet* packet_factory::make_in_packet(buffer_t buf)
{
	packet_in* ret = new packet_in(key);
	ret->create(std::move(buf));
	if (ret->decrypt()) {
		return ret;
	}
	delete ret;
	return NULL;
}

template <packet_type t, typename... Args>
packet* packet_factory::make_out_packet(Args... args)
{
	packet_out* ret = new packet_out(key);
	ret->create<t>(args...);
	ret->encrypt();
	return ret;
}

template <packet_type t, typename... Args>
packet* packet_factory::make_fake_packet(Args... args)
{
	packet_out* ret = new packet_out(key);
	ret->create<t>(args...);
	return ret;
}

} // namespace net
DEVILUTION_END_NAMESPACE
