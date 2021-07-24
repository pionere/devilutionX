#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <array>
#include <cstring>
#ifndef NONET
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
	PT_CONNECT,			// zt-only
	PT_DISCONNECT,
	PT_INFO_REQUEST,	// zt-only
	PT_INFO_REPLY,		// zt-only
	// clang-format on
};

// Returns NULL for an invalid packet type.
const char *packet_type_to_string(uint8_t packet_type);

typedef uint8_t plr_t;
typedef DWORD cookie_t;
typedef DWORD turn_t;
typedef uint8_t leaveinfo_t;
#ifndef NONET
typedef std::array<unsigned char, crypto_secretbox_KEYBYTES> key_t;
#else
// Stub out the key_t definition as we're not doing any encryption.
using key_t = uint8_t;
#endif

class packet_exception : public dvlnet_exception {
public:
	const char *what() const throw() override
	{
		return "Incorrect package size";
	}
};

class wrong_packet_type_exception : public packet_exception {
public:
	wrong_packet_type_exception(std::initializer_list<packet_type> expected_types, std::uint8_t actual);

	const char *what() const throw() override
	{
		return message_.c_str();
	}

private:
	std::string message_;
};

class packet {
protected:
	packet_type m_type;
	plr_t m_src;
	plr_t m_dest;
	buffer_t m_message;
	turn_t m_turn;
	cookie_t m_cookie;
	plr_t m_newplr;
	buffer_t m_info;
	leaveinfo_t m_leaveinfo;

	const key_t &key;
	buffer_t encrypted_buffer;
	buffer_t decrypted_buffer;

public:
	packet(const key_t &k)
	    : key(k) {};

	const buffer_t &encrypted_data();

	packet_type type();
	plr_t src();
	plr_t dest();
	const buffer_t &message();
	turn_t turn();
	cookie_t cookie();
	plr_t newplr();
	const buffer_t &info();
	leaveinfo_t leaveinfo();
};

template <class P>
class packet_proc : public packet {
public:
	using packet::packet;
	void process_data();
};

class packet_in : public packet_proc<packet_in> {
public:
	using packet_proc<packet_in>::packet_proc;
	void create(buffer_t buf);
	void process_element(buffer_t &x);
	template <class T>
	void process_element(T &x);
	void decrypt();
};

class packet_out : public packet_proc<packet_out> {
public:
	using packet_proc<packet_out>::packet_proc;

	template <packet_type t, typename... Args>
	void create(Args... args);

	void process_element(buffer_t &x);
	template <class T>
	void process_element(T &x);
	void encrypt();
};

template <class P>
void packet_proc<P>::process_data()
{
	P &self = static_cast<P &>(*this);
	self.process_element(m_type);
	self.process_element(m_src);
	self.process_element(m_dest);
	switch (m_type) {
	case PT_MESSAGE:
		self.process_element(m_message);
		break;
	case PT_TURN:
		self.process_element(m_turn);
		break;
	case PT_JOIN_REQUEST:
		self.process_element(m_cookie);
		break;
	case PT_JOIN_ACCEPT:
		self.process_element(m_cookie);
		self.process_element(m_newplr);
		self.process_element(m_info);
		break;
	case PT_CONNECT:
		self.process_element(m_newplr);
		self.process_element(m_info);
		break;
	case PT_DISCONNECT:
		self.process_element(m_newplr);
		self.process_element(m_leaveinfo);
		break;
	case PT_INFO_REQUEST:
		break;
	case PT_INFO_REPLY:
		self.process_element(m_info);
		break;
	}
}

inline void packet_in::process_element(buffer_t &x)
{
	x.insert(x.begin(), decrypted_buffer.begin(), decrypted_buffer.end());
	decrypted_buffer.resize(0);
}

template <class T>
void packet_in::process_element(T &x)
{
	if (decrypted_buffer.size() < sizeof(T))
		throw packet_exception();
	std::memcpy(&x, decrypted_buffer.data(), sizeof(T));
	decrypted_buffer.erase(decrypted_buffer.begin(),
	    decrypted_buffer.begin() + sizeof(T));
}

template <>
inline void packet_out::create<PT_INFO_REQUEST>(plr_t s, plr_t d)
{
	m_type = PT_INFO_REQUEST;
	m_src = s;
	m_dest = d;
}

template <>
inline void packet_out::create<PT_INFO_REPLY>(plr_t s, plr_t d, buffer_t i)
{
	m_type = PT_INFO_REPLY;
	m_src = s;
	m_dest = d;
	m_info = std::move(i);
}

template <>
inline void packet_out::create<PT_MESSAGE>(plr_t s, plr_t d, buffer_t m)
{
	m_type = PT_MESSAGE;
	m_src = s;
	m_dest = d;
	m_message = std::move(m);
}

template <>
inline void packet_out::create<PT_TURN>(plr_t s, plr_t d, turn_t u)
{
	m_type = PT_TURN;
	m_src = s;
	m_dest = d;
	m_turn = u;
}

template <>
inline void packet_out::create<PT_JOIN_REQUEST>(plr_t s, plr_t d, cookie_t c)
{
	m_type = PT_JOIN_REQUEST;
	m_src = s;
	m_dest = d;
	m_cookie = c;
}

template <>
inline void packet_out::create<PT_JOIN_ACCEPT>(plr_t s, plr_t d, cookie_t c,
    plr_t n, buffer_t i)
{
	m_type = PT_JOIN_ACCEPT;
	m_src = s;
	m_dest = d;
	m_cookie = c;
	m_newplr = n;
	m_info = i;
}

template <>
inline void packet_out::create<PT_CONNECT>(plr_t s, plr_t d, plr_t n, buffer_t i)
{
	m_type = PT_CONNECT;
	m_src = s;
	m_dest = d;
	m_newplr = n;
	m_info = i;
}

template <>
inline void packet_out::create<PT_CONNECT>(plr_t s, plr_t d, plr_t n)
{
	m_type = PT_CONNECT;
	m_src = s;
	m_dest = d;
	m_newplr = n;
}

template <>
inline void packet_out::create<PT_DISCONNECT>(plr_t s, plr_t d, plr_t n,
    leaveinfo_t l)
{
	m_type = PT_DISCONNECT;
	m_src = s;
	m_dest = d;
	m_newplr = n;
	m_leaveinfo = l;
}

class packet_factory {
	key_t key = {};

public:
	void setup_password(const char* passwd);
	std::unique_ptr<packet> make_in_packet(buffer_t buf);
	template <packet_type t, typename... Args>
	std::unique_ptr<packet> make_out_packet(Args... args);
	template <packet_type t, typename... Args>
	std::unique_ptr<packet> make_fake_out_packet(Args... args);

	template <class T>
	static const BYTE* begin(const T &x);
	template <class T>
	static const BYTE* end(const T &x);
};

inline std::unique_ptr<packet> packet_factory::make_in_packet(buffer_t buf)
{
	auto ret = std::make_unique<packet_in>(key);
	ret->create(std::move(buf));
	ret->decrypt();
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

template <packet_type t, typename... Args>
std::unique_ptr<packet> packet_factory::make_fake_out_packet(Args... args)
{
	std::unique_ptr<packet_out> ret(new packet_out(key));
	ret->create<t>(args...);
	return std::unique_ptr<packet>(std::move(ret));
}

template <class T>
const BYTE* packet_factory::begin(const T &x)
{
	return reinterpret_cast<const BYTE*>(&x);
}

template <class T>
const BYTE* packet_factory::end(const T &x)
{
	return reinterpret_cast<const BYTE*>(&x) + sizeof(T);
}

inline void packet_out::process_element(buffer_t &x)
{
	encrypted_buffer.insert(encrypted_buffer.end(), x.begin(), x.end());
}

template <class T>
void packet_out::process_element(T &x)
{
	encrypted_buffer.insert(encrypted_buffer.end(), packet_factory::begin(x), packet_factory::end(x));
}

} // namespace net
DEVILUTION_END_NAMESPACE
