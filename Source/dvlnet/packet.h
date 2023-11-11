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
	PT_CONNECT,			// tcpd, zt-only
	PT_DISCONNECT,
	PT_INFO_REQUEST,	// zt-only
	PT_INFO_REPLY,		// zt-only
	// clang-format on
};

typedef uint8_t plr_t;
typedef DWORD cookie_t;
typedef uint32_t turn_t;
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
	packet_type m_type;
	plr_t m_src;
	plr_t m_dest;
} NetPktHdr;

typedef struct NetPktMessage {
	NetPktHdr npHdr;
	//BYTE m_message[0];
} NetPktMessage;

typedef struct NetPktTurn {
	NetPktHdr npHdr;
	turn_t m_turn;
	//BYTE m_message[0];
} NetPktTurn;

typedef struct NetPktJoinRequest {
	NetPktHdr npHdr;
	cookie_t m_cookie;
	//SNetGameData m_info;
} NetPktJoinRequest;

typedef struct NetPktJoinAccept {
	NetPktHdr npHdr;
	cookie_t m_cookie;
	plr_t m_newplr;
	SNetGameData m_info;
} NetPktJoinAccept;

typedef struct NetPktConnect {
	NetPktHdr npHdr;
	plr_t m_newplr;
	//BYTE m_addr[0]; // 16 in case of zt
} NetPktConnect;

typedef struct NetPktDisconnect {
	NetPktHdr npHdr;
	plr_t m_newplr;
} NetPktDisconnect;

typedef struct NetPktInfoRequest {
	NetPktHdr npHdr;
} NetPktInfoRequest;

typedef struct NetPktInfoReply {
	NetPktHdr npHdr;
	//BYTE m_gamename[0];
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

	const buffer_t& encrypted_data();

	packet_type pktType() const
	{
		return reinterpret_cast<const NetPktHdr*>(decrypted_buffer.data())->m_type;
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
	SNetGameData& pktJoinAccInfo()
	{
		return reinterpret_cast<NetPktJoinAccept*>(decrypted_buffer.data())->m_info;
	}
	// PT_INFO_REPLY
	buffer_t::const_iterator pktInfoReplyNameBegin() const
	{
		return decrypted_buffer.begin() + sizeof(NetPktHdr);
	}
	buffer_t::const_iterator pktInfoReplyNameEnd() const
	{
		return decrypted_buffer.end();
	}
	// PT_CONNECT
	plr_t pktConnectPlr() const
	{
		return reinterpret_cast<const NetPktConnect*>(decrypted_buffer.data())->m_newplr;
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
		return reinterpret_cast<const NetPktDisconnect*>(decrypted_buffer.data())->m_newplr;
	}
};

class packet_in : public packet {
	friend class packet_factory;
	using packet::packet;

protected:
	void create(buffer_t buf);
	bool decrypt();
};

class packet_out : public packet {
	friend class packet_factory;
	using packet::packet;

protected:
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
inline void packet_out::create<PT_INFO_REPLY>(plr_t s, plr_t d, buffer_t i)
{
	decrypted_buffer.resize(sizeof(NetPktHdr) + i.size());
	NetPktInfoReply* data = (NetPktInfoReply*)decrypted_buffer.data();
	data->npHdr.m_type = PT_INFO_REPLY;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	memcpy((BYTE*)data + sizeof(NetPktHdr), i.data(), i.size());
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
    plr_t n, buffer_t i)
{
	decrypted_buffer.resize(sizeof(NetPktJoinAccept));
	NetPktJoinAccept* data = (NetPktJoinAccept*)decrypted_buffer.data();
	data->npHdr.m_type = PT_JOIN_ACCEPT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_cookie = c;
	data->m_newplr = n;
	memcpy(&data->m_info, i.data(), sizeof(SNetGameData));
}

template <>
inline void packet_out::create<PT_CONNECT>(plr_t s, plr_t d, plr_t n, buffer_t i)
{
	decrypted_buffer.resize(sizeof(NetPktConnect) + i.size());
	NetPktConnect* data = (NetPktConnect*)decrypted_buffer.data();
	data->npHdr.m_type = PT_CONNECT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_newplr = n;
	memcpy((BYTE*)data + sizeof(NetPktConnect), i.data(), i.size());
}

/*template <>
inline void packet_out::create<PT_CONNECT>(plr_t s, plr_t d, plr_t n)
{
	decrypted_buffer.resize(sizeof(NetPktConnect));
	NetPktConnect* data = (NetPktConnect*)decrypted_buffer.data();
	data->npHdr.m_type = PT_CONNECT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_newplr = n;
}*/

template <>
inline void packet_out::create<PT_DISCONNECT>(plr_t s, plr_t d, plr_t n)
{
	decrypted_buffer.resize(sizeof(NetPktDisconnect));
	NetPktDisconnect* data = (NetPktDisconnect*)decrypted_buffer.data();
	data->npHdr.m_type = PT_DISCONNECT;
	data->npHdr.m_src = s;
	data->npHdr.m_dest = d;
	data->m_newplr = n;
}

class packet_factory {
	key_t key = {};

public:
	void setup_password(const char* passwd);
	std::unique_ptr<packet> make_in_packet(buffer_t buf);
	template <packet_type t, typename... Args>
	std::unique_ptr<packet> make_out_packet(Args... args);
};

inline std::unique_ptr<packet> packet_factory::make_in_packet(buffer_t buf)
{
	auto ret = std::make_unique<packet_in>(key);
	ret->create(std::move(buf));
	if (!ret->decrypt())
		ret = NULL;
	return ret;
}

template <packet_type t, typename... Args>
std::unique_ptr<packet> packet_factory::make_out_packet(Args... args)
{
	auto ret = std::make_unique<packet_out>(key);
	ret->create<t>(args...);
	ret->encrypt();
	return ret;
}

} // namespace net
DEVILUTION_END_NAMESPACE
