#include "storm_net.h"

#ifdef ZEROTIER
#include <mutex>
#include <thread>
#include <utility>
#endif

#include "dvlnet/abstract_net.h"
#include "all.h"

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

SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetReceiveTurn(status);
}

void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetSendTurn(turn, data, size);
}

turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetPollTurns(status);
}

uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_inst->SNetLastTurn(status);
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

void SNetGetGameInfo(const char** name, const char** password)
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
bool SNetCreateGame(const char* pszGamePassword, _uigamedata* gameData, char (&errorText)[256])
{
	bool result;

	// assert(gameData != NULL && pszGamePassword != NULL);
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->make_default_gamename(gpszGameName);
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	result = dvlnet_inst->create_game(gpszGameName, port, pszGamePassword, gameData, errorText);
	snprintf(gpszGameName, sizeof(gpszGameName), "%s:%d", gpszGameName, port);
	return result;
}

bool SNetJoinGame(const char* pszGameName, unsigned port, const char* pszGamePassword, char (&errorText)[256])
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	// assert(pszGameName != NULL && pszGamePassword != NULL);
	snprintf(gpszGameName, sizeof(gpszGameName), "%s:%d", pszGameName, port);
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	return dvlnet_inst->join_game(pszGameName, port, pszGamePassword, errorText);
}

unsigned SNetGetTurnsInTransit()
{
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
