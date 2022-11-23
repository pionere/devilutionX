#include "packet.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

const buffer_t& packet::encrypted_data()
{
	return encrypted_buffer;
}

void packet_in::create(buffer_t buf)
{
	encrypted_buffer = std::move(buf);
}

bool packet_in::decrypt()
{
#ifdef NETENCRYPT
	if (encrypted_buffer.size() < crypto_secretbox_NONCEBYTES
	     + crypto_secretbox_MACBYTES + sizeof(NetPktHdr))
		return false;
	auto pktlen = encrypted_buffer.size() - crypto_secretbox_NONCEBYTES -  crypto_secretbox_MACBYTES;
	decrypted_buffer.resize(pktlen);
	if (crypto_secretbox_open_easy(decrypted_buffer.data(),
	        encrypted_buffer.data() + crypto_secretbox_NONCEBYTES,
	        encrypted_buffer.size() - crypto_secretbox_NONCEBYTES,
	        encrypted_buffer.data(),
	        key.data))
		return false;
#else
	if (encrypted_buffer.size() < sizeof(NetPktHdr))
		return false;
	decrypted_buffer = encrypted_buffer;
#endif
	return true;
}

void packet_out::encrypt()
{
	encrypted_buffer = decrypted_buffer;
	assert(encrypted_buffer.size() >= sizeof(NetPktHdr));
#ifdef NETENCRYPT
	auto lenCleartext = encrypted_buffer.size();
	encrypted_buffer.insert(encrypted_buffer.begin(), crypto_secretbox_NONCEBYTES, 0);
	encrypted_buffer.insert(encrypted_buffer.end(), crypto_secretbox_MACBYTES, 0);
	randombytes_buf(encrypted_buffer.data(), crypto_secretbox_NONCEBYTES);
	if (crypto_secretbox_easy(encrypted_buffer.data() + crypto_secretbox_NONCEBYTES,
	        encrypted_buffer.data() + crypto_secretbox_NONCEBYTES,
	        lenCleartext,
	        encrypted_buffer.data(),
	        key.data))
		app_error(ERR_APP_PACKET_ENCRYPT);
#endif
}

void packet_factory::setup_password(const char* passwd)
{
#ifdef NETENCRYPT
	if (sodium_init() < 0)
		app_error(ERR_APP_PACKET_SETUP);
	std::string pw = std::string(passwd);
	pw.resize(std::min<size_t>(pw.size(), crypto_pwhash_argon2id_PASSWD_MAX));
	pw.resize(std::max<size_t>(pw.size(), crypto_pwhash_argon2id_PASSWD_MIN), 0);
	std::string salt("W9bE9dQgVaeybwr2");
	salt.resize(crypto_pwhash_argon2id_SALTBYTES, 0);
	if (crypto_pwhash(key.data, crypto_secretbox_KEYBYTES,
	        pw.data(), pw.size(),
	        reinterpret_cast<const unsigned char*>(salt.data()),
	        3 * crypto_pwhash_argon2id_OPSLIMIT_MIN,
	        2 * crypto_pwhash_argon2id_MEMLIMIT_MIN,
	        crypto_pwhash_ALG_ARGON2ID13))
		app_error(ERR_APP_PACKET_PASSWD);
#endif
}

} // namespace net
DEVILUTION_END_NAMESPACE
