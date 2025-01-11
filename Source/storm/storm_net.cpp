#include "storm_net.h"

#include "dvlnet/abstract_net.h"
#include "dvlnet/zt_client.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static net::abstract_net* dvlnet_inst = NULL;
static char gpszGameName[NET_MAX_GAMENAME_LEN + 1] = {};
static char gpszGamePassword[NET_MAX_PASSWD_LEN + 1] = {};

void SNetInitializeProvider(unsigned provider)
{
	delete dvlnet_inst;
	dvlnet_inst = net::abstract_net::make_net(provider);
}

/*void SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	dvlnet_inst->SNetGetProviderCaps(caps);
}*/

void SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	dvlnet_inst->SNetRegisterEventHandler(evtype, func);
}

void SNetUnregisterEventHandler(int evtype)
{
	dvlnet_inst->SNetUnregisterEventHandler(evtype);
}

bool SNetCreateGame(unsigned port, const char* pszGamePassword, _uigamedata* gameData, char (&errorText)[256])
{
	bool result;
	// assert(gameData != NULL && pszGamePassword != NULL);
	dvlnet_inst->make_default_gamename(gpszGameName);
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	result = dvlnet_inst->setup_game(gameData, gpszGameName, port, pszGamePassword, errorText);
#ifdef ZEROTIER
	if (port != 0)
#endif
		snprintf(gpszGameName, sizeof(gpszGameName), "%s:%d", gpszGameName, port);
	return result;
}

bool SNetJoinGame(const char* pszGameName, unsigned port, const char* pszGamePassword, char (&errorText)[256])
{
	const char* format = "%s:%d";
	// assert(pszGameName != NULL && pszGamePassword != NULL);
#ifdef ZEROTIER
	if (port == 0)
		format = "%s";
#endif
	snprintf(gpszGameName, sizeof(gpszGameName), format, pszGameName, port);
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	return dvlnet_inst->setup_game(NULL, pszGameName, port, pszGamePassword, errorText);
}

void SNetGetGameInfo(const char** name, const char** password)
{
	*name = gpszGameName;
	*password = gpszGamePassword;
}

void SNetSendMessage(int receiver, const BYTE* data, unsigned databytes)
{
	dvlnet_inst->SNetSendMessage(receiver, data, databytes);
}

SNetMsgPkt* SNetReceiveMessage()
{
	return dvlnet_inst->SNetReceiveMessage();
}

void SNetSendTurn(turn_t turn, const BYTE* data, unsigned databytes)
{
	dvlnet_inst->SNetSendTurn(turn, data, databytes);
}

SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_inst->SNetReceiveTurn(status);
}

turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_inst->SNetPollTurns(status);
}

unsigned SNetGetTurnsInTransit()
{
	return dvlnet_inst->SNetGetTurnsInTransit();
}

#ifdef ZEROTIER
bool SNetReady()
{
	return (static_cast<net::zt_client*>(&*dvlnet_inst))->network_ready();
}

void SNetGetGamelist(std::vector<SNetZtGame>& games)
{
	return (static_cast<net::zt_client*>(&*dvlnet_inst))->get_gamelist(games);
}
#endif

void SNetDropPlayer(int playerid)
{
	dvlnet_inst->SNetDropPlayer(playerid);
}

void SNetLeaveGame()
{
	dvlnet_inst->SNetLeaveGame();
	memset(gpszGamePassword, 0, sizeof(gpszGamePassword));
}

void SNetDisconnect()
{
	dvlnet_inst->SNetDisconnect();
}

void SNetDestroy()
{
	delete dvlnet_inst;
	dvlnet_inst = NULL;
}

DEVILUTION_END_NAMESPACE
