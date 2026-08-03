#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

#define PKT_META_LEN (crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES + sizeof(NetPktHdr))

const buffer_t& packet::encrypted_data() const
{
	return encrypted_buffer;
}

#if DEV_MODE || INET_MODE || DEBUG_MODE
bool packet::validate()
{
	size_t size = decrypted_buffer.size();
	plr_t pkt_plr = pktSrc();
	// assert(size >= sizeof(NetPktHdr));
	switch (reinterpret_cast<const NetPktHdr*>(decrypted_buffer.data())->m_type) {
	case PT_MESSAGE:
		if (pkt_plr >= MAX_PLRS && pkt_plr != PLR_MASTER)
			return false;
		break;
	case PT_TURN:
		if (pkt_plr >= MAX_PLRS || size < sizeof(NetPktTurn))
			return false;
		break;
	case PT_JOIN_REQUEST:
		if (size != sizeof(NetPktJoinRequest))
			return false;
		break;
	case PT_JOIN_ACCEPT:
		if (size < sizeof(NetPktJoinAccept))
			return false;
		if (pktJoinAccPlr() >= MAX_PLRS)
			return false;
		break;
	case PT_CONNECT:		// tcp, zt-only
		if (size < sizeof(NetPktConnect))
			return false;
		// if (pkt_plr >= MAX_PLRS && pkt_plr != SNPLAYER_MASTER)
		//	return false;
		break;
	case PT_DISCONNECT:
		if (size != sizeof(NetPktDisconnect))
			return false;
		break;
#ifdef ZEROTIER
	case PT_INFO_REQUEST:
		return true;
	case PT_INFO_REPLY:
		if (size != sizeof(NetPktInfoReply))
			return false;
		break;
#endif
	default:
		return false;
	}

	return true;
}
#endif

void packet_in::create(buffer_t buf)
{
	encrypted_buffer = std::move(buf);
}

bool packet_in::decrypt()
{
	size_t insize = encrypted_buffer.size();
#ifdef NETENCRYPT
	if (insize < PKT_META_LEN)
		return false;
	size_t pktlen = insize - crypto_secretbox_NONCEBYTES - crypto_secretbox_MACBYTES;
	decrypted_buffer.resize(pktlen);
	BYTE* indata = encrypted_buffer.data();
	if (crypto_secretbox_open_easy(decrypted_buffer.data(),
	        &indata[crypto_secretbox_NONCEBYTES],
	        insize - crypto_secretbox_NONCEBYTES,
	        &indata[0],
	        key.data))
		return false;
#else
	if (insize < sizeof(NetPktHdr))
		return false;
	decrypted_buffer = encrypted_buffer;
#endif

#if INET_MODE
	return this->validate();
#else
	return true;
#endif
}

void packet_out::encrypt()
{
	size_t insize = decrypted_buffer.size();

	assert(insize >= sizeof(NetPktHdr));
#ifdef NETENCRYPT
	encrypted_buffer.resize(crypto_secretbox_NONCEBYTES + insize + crypto_secretbox_MACBYTES);
	BYTE* outdata = encrypted_buffer.data();
	randombytes_buf(outdata, crypto_secretbox_NONCEBYTES);
	//memcpy(&outdata[crypto_secretbox_NONCEBYTES], decrypted_buffer.data(), insize);
	//memset(&outdata[crypto_secretbox_NONCEBYTES + insize], 0, crypto_secretbox_MACBYTES);
	if (crypto_secretbox_easy(&outdata[crypto_secretbox_NONCEBYTES],
	        decrypted_buffer.data(), // &outdata[crypto_secretbox_NONCEBYTES]
	        insize,
	        &outdata[0],
	        key.data))
		app_error(ERR_APP_PACKET_ENCRYPT);
#else
	encrypted_buffer = decrypted_buffer;
#endif
}

void packet_factory::setup_password(const char* passwd)
{
#ifdef NETENCRYPT
	if (sodium_init() < 0)
		app_error(ERR_APP_PACKET_SETUP);
	std::string pw = std::string(passwd);
	size_t pwsize = pw.size();
	if (pwsize < crypto_pwhash_argon2id_PASSWD_MIN)
		pwsize = crypto_pwhash_argon2id_PASSWD_MIN;
	if (pwsize > crypto_pwhash_argon2id_PASSWD_MAX)
		pwsize = crypto_pwhash_argon2id_PASSWD_MAX;
	pw.resize(pwsize, '\000');
	const BYTE salt[crypto_pwhash_argon2id_SALTBYTES] = { 'W', '9', 'b', 'E', '9', 'd', 'Q', 'g', 'V', 'a', 'e', 'y', 'b', 'w', 'r', '2' };
	if (crypto_pwhash(key.data, crypto_secretbox_KEYBYTES,
	        pw.data(), pwsize,
	        salt,
	        3 * crypto_pwhash_argon2id_OPSLIMIT_MIN,
	        2 * crypto_pwhash_argon2id_MEMLIMIT_MIN,
	        crypto_pwhash_ALG_ARGON2ID13))
		app_error(ERR_APP_PACKET_PASSWD);
#endif
}

void packet_factory::clear_password()
{
	setup_password("");
}

} // namespace net
DEVILUTION_END_NAMESPACE
