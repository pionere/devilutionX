#pragma once

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

typedef std::vector<unsigned char> buffer_t;
typedef unsigned long provider_t;
typedef size_t net_size_t;

class dvlnet_exception : public std::exception {
public:
	const char *what() const throw() override
	{
		return "Network error";
	}
};

class abstract_net {
public:
	virtual bool create(const std::string &addrstr, unsigned port, const std::string &passwd) = 0;
	virtual bool join(const std::string &addrstr, unsigned port, const std::string &passwd) = 0;
	virtual bool SNetReceiveMessage(int *sender, char **data,
	    int *size)
	    = 0;
	virtual bool SNetSendMessage(int dest, void *data,
	    unsigned int size)
	    = 0;
	virtual bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned (&size)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
	    = 0;
	virtual bool SNetSendTurn(char *data, unsigned int size) = 0;
	virtual bool SNetGetProviderCaps(struct _SNETCAPS *caps) = 0;
	virtual bool SNetRegisterEventHandler(event_type evtype,
	    SEVTHANDLER func)
	    = 0;
	virtual bool SNetUnregisterEventHandler(event_type evtype,
	    SEVTHANDLER func)
	    = 0;
	virtual void SNetLeaveGame(int type) = 0;
	virtual bool SNetDropPlayer(int playerid, unsigned flags) = 0;
	virtual bool SNetGetOwnerTurnsWaiting(DWORD *turns) = 0;
	virtual bool SNetGetTurnsInTransit(DWORD *turns) = 0;
	virtual void setup_gameinfo(buffer_t info) = 0;
	virtual ~abstract_net() = default;

	virtual std::string make_default_gamename() = 0;
#ifdef ZEROTIER
	virtual void setup_password(std::string passwd)
	{
	}

	virtual void send_info_request()
	{
	}

	virtual std::vector<std::string> get_gamelist()
	{
		return std::vector<std::string>();
	}
#endif
	static std::unique_ptr<abstract_net> make_net(provider_t provider);
};

} // namespace net
DEVILUTION_END_NAMESPACE
