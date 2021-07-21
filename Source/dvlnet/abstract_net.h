#pragma once

#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "all.h"
#include "storm/storm_net.h"

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
	virtual bool SNetReceiveMessage(int *sender, char **data, unsigned *size) = 0;
	virtual void SNetSendMessage(int dest, const void *data, unsigned size) = 0;
	virtual bool SNetReceiveTurns(uint32_t *(&data)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
	    = 0;
	virtual void SNetSendTurn(uint32_t turn) = 0;
	virtual void SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func) = 0;
	virtual void SNetUnregisterEventHandler(event_type evtype, SEVTHANDLER func) = 0;
	virtual void SNetLeaveGame(int reason) = 0;
	virtual void SNetDropPlayer(int playerid) = 0;
	virtual uint32_t SNetGetOwnerTurnsWaiting() = 0;
	virtual uint32_t SNetGetTurnsInTransit() = 0;
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
