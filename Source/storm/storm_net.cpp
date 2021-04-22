#include <memory>
#ifdef ZEROTIER
#include <mutex>
#include <thread>
#include <utility>
#endif

#include "all.h"
#include "utils/stubs.h"
#include "dvlnet/abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE

static std::unique_ptr<net::abstract_net> dvlnet_inst;
static char gpszGameName[128] = {};
static char gpszGamePassword[128] = {};

#ifdef ZEROTIER
static std::mutex storm_net_mutex;
#endif

bool SNetReceiveMessage(int *senderplayerid, char **data, int *databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	if (!dvlnet_inst->SNetReceiveMessage(senderplayerid, data, databytes)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

bool SNetSendMessage(int playerID, void *data, unsigned int databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetSendMessage(playerID, data, databytes);
}

bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned (&size)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	if (!dvlnet_inst->SNetReceiveTurns(data, size, status)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

bool SNetSendTurn(char *data, unsigned int databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetSendTurn(data, databytes);
}

bool SNetGetProviderCaps(struct _SNETCAPS *caps)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetGetProviderCaps(caps);
}

bool SNetUnregisterEventHandler(event_type evtype, SEVTHANDLER func)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetUnregisterEventHandler(evtype, func);
}

bool SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetRegisterEventHandler(evtype, func);
}

bool SNetDestroy()
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return true;
}

bool SNetDropPlayer(int playerid, unsigned flags)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetDropPlayer(playerid, flags);
}

void SNetGetGameInfo(const char** name, const char **password)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	*name = gpszGameName;
	*password = gpszGamePassword;
}

void SNetLeaveGame(int type)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	if (dvlnet_inst == NULL)
		return;
	dvlnet_inst->SNetLeaveGame(type);
}

/**
 * @brief Called by engine for single, called by ui for multi
 * @param provider BNET, IPXN, MODM, SCBL or UDPN
 */
void SNetInitializeProvider(unsigned long provider)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst = net::abstract_net::make_net(provider);
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
bool SNetCreateGame(const char *pszGamePassword, _SNETGAMEDATA* gameData)
{
	// assert(gameData != NULL && pszGamePassword != NULL);
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif

	char* gData = (char*)gameData;
	net::buffer_t game_init_info(gData, gData + sizeof(*gameData));
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	std::string default_name = dvlnet_inst->make_default_gamename();
	const char *pszGameName = default_name.c_str();
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	snprintf(gpszGameName, sizeof(gpszGameName), "%s:%d", pszGameName, port);
	snprintf(gpszGamePassword, sizeof(gpszGamePassword), "%s", pszGamePassword);
	return dvlnet_inst->create(pszGameName, port, pszGamePassword);
}

bool SNetJoinGame(const char *pszGameName, unsigned port, const char *pszGamePassword)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	// assert(pszGameName != NULL && pszGamePassword != NULL);
	snprintf(gpszGameName, sizeof(gpszGameName), "%s:%d", pszGameName, port);
	snprintf(gpszGamePassword, sizeof(gpszGamePassword), "%s", pszGamePassword);
	return dvlnet_inst->join(pszGameName, port, pszGamePassword);
}

/**
 * @brief Is this the mirror image of SNetGetTurnsInTransit?
 */
bool SNetGetOwnerTurnsWaiting(DWORD *turns)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetGetOwnerTurnsWaiting(turns);
}

bool SNetGetTurnsInTransit(DWORD *turns)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetGetTurnsInTransit(turns);
}

#ifdef ZEROTIER
void SNetSendInfoRequest()
{
	dvlnet_inst->send_info_request();
}

std::vector<std::string> SNetGetGamelist()
{
	return dvlnet_inst->get_gamelist();
}

void SNetSetPassword(std::string pw)
{
	dvlnet_inst->setup_password(std::move(pw));
}
#endif
DEVILUTION_END_NAMESPACE
