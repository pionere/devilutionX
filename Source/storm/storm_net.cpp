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
static char gpszGameName[NET_MAX_GAMENAME_LEN + 1] = {};
static char gpszGamePassword[NET_MAX_PASSWD_LEN + 1] = {};

#ifdef ZEROTIER
static std::mutex storm_net_mutex;
#endif

void SNetInitializeProvider(unsigned provider)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst = net::abstract_net::make_net(provider);
}

/*void SNetGetProviderCaps(struct _SNETCAPS *caps)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetGetProviderCaps(caps);
}*/

void SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetRegisterEventHandler(evtype, func);
}

void SNetUnregisterEventHandler(int evtype)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetUnregisterEventHandler(evtype);
}

bool SNetCreateGame(unsigned port, const char* pszGamePassword, _uigamedata* gameData, char (&errorText)[256])
{
	bool result;

	// assert(gameData != NULL && pszGamePassword != NULL);
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->make_default_gamename(gpszGameName);
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	result = dvlnet_inst->setup_game(gameData, gpszGameName, port, pszGamePassword, errorText);
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
	return dvlnet_inst->setup_game(NULL, pszGameName, port, pszGamePassword, errorText);
}

void SNetGetGameInfo(const char** name, const char** password)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	*name = gpszGameName;
	*password = gpszGamePassword;
}

void SNetSendMessage(int receiver, const BYTE* data, unsigned databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetSendMessage(receiver, data, databytes);
}

bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetReceiveMessage(sender, data, databytes);
}

void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned databytes)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetSendTurn(turn, data, databytes);
}

SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetReceiveTurn(status);
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

void SNetDropPlayer(int playerid)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetDropPlayer(playerid);
}

void SNetLeaveGame()
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	if (dvlnet_inst == NULL)
		return;
	dvlnet_inst->SNetLeaveGame();
}

void SNetDisconnect()
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst->SNetDisconnect();
}

DEVILUTION_END_NAMESPACE
