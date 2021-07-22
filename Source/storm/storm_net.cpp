#include "storm_net.h"

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

bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetReceiveMessage(sender, data, databytes);
}

void SNetSendMessage(int receiver, const BYTE* data, unsigned databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetSendMessage(receiver, data, databytes);
}

bool SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetReceiveTurns(turns, status);
}

void SNetSendTurn(uint32_t turn)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetSendTurn(turn);
}

/*void SNetGetProviderCaps(struct _SNETCAPS *caps)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetGetProviderCaps(caps);
}*/

void SNetUnregisterEventHandler(int evtype)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetUnregisterEventHandler(evtype);
}

void SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetRegisterEventHandler(evtype, func);
}

void SNetDropPlayer(int playerid)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetDropPlayer(playerid);
}

void SNetGetGameInfo(const char** name, const char **password)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	*name = gpszGameName;
	*password = gpszGamePassword;
}

void SNetLeaveGame(int reason)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	if (dvlnet_inst == NULL)
		return;
	dvlnet_inst->SNetLeaveGame(reason);
}

/**
 * @brief Called by engine for single, called by ui for multi
 * @param provider BNET, IPXN, MODM, SCBL or UDPN
 */
void SNetInitializeProvider(unsigned provider)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst = net::abstract_net::make_net(provider);
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
bool SNetCreateGame(const char* pszGamePassword, SNetGameData* gameData)
{
	bool result;

	// assert(gameData != NULL && pszGamePassword != NULL);
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif

	char* gData = (char*)gameData;
	net::buffer_t game_init_info(gData, gData + sizeof(*gameData));
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	dvlnet_inst->make_default_gamename(gpszGameName);
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	snprintf(gpszGamePassword, sizeof(gpszGamePassword), "%s", pszGamePassword);
	result = dvlnet_inst->create(gpszGameName, port, pszGamePassword);
	snprintf(gpszGameName, sizeof(gpszGameName), "%s:%d", gpszGameName, port);
	return result;
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
uint32_t SNetGetOwnerTurnsWaiting()
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetGetOwnerTurnsWaiting();
}

uint32_t SNetGetTurnsInTransit()
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetGetTurnsInTransit();
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
