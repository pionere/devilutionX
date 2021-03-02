#include <memory>

#include "all.h"
#include "stubs.h"
#include "dvlnet/abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE

static std::unique_ptr<net::abstract_net> dvlnet_inst;
static char gpszGameName[128] = {};
static char gpszGamePassword[128] = {};

bool SNetReceiveMessage(int *senderplayerid, char **data, int *databytes)
{
	if (!dvlnet_inst->SNetReceiveMessage(senderplayerid, data, databytes)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

bool SNetSendMessage(int playerID, void *data, unsigned int databytes)
{
	return dvlnet_inst->SNetSendMessage(playerID, data, databytes);
}

bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned int (&size)[MAX_PLRS],
    DWORD (&status)[MAX_PLRS])
{
	if (!dvlnet_inst->SNetReceiveTurns(data, size, status)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

bool SNetSendTurn(char *data, unsigned int databytes)
{
	return dvlnet_inst->SNetSendTurn(data, databytes);
}

bool SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	return dvlnet_inst->SNetGetProviderCaps(caps);
}

bool SNetUnregisterEventHandler(int evtype, SEVTHANDLER func)
{
	return dvlnet_inst->SNetUnregisterEventHandler(*(event_type *)&evtype, func);
}

bool SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	return dvlnet_inst->SNetRegisterEventHandler(*(event_type *)&evtype, func);
}

bool SNetDestroy()
{
	return true;
}

bool SNetDropPlayer(int playerid, DWORD flags)
{
	return dvlnet_inst->SNetDropPlayer(playerid, flags);
}

void SNetGetGameInfo(int type, void *dst, unsigned int length)
{
	switch (type) {
	case GAMEINFO_NAME:
		SStrCopy((char *)dst, gpszGameName, length);
		break;
	case GAMEINFO_PASSWORD:
		SStrCopy((char *)dst, gpszGamePassword, length);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

bool SNetLeaveGame(int type)
{
	if (dvlnet_inst == NULL)
		return true;
	return dvlnet_inst->SNetLeaveGame(type);
}

/**
 * @brief Called by engine for single, called by ui for multi
 * @param provider BNET, IPXN, MODM, SCBL or UDPN
 */
bool SNetInitializeProvider(unsigned long provider,
	struct _SNETPROGRAMDATA *client_info, struct _SNETUIDATA *ui_info)
{
	dvlnet_inst = net::abstract_net::make_net(provider);
	return ui_info->selectnamecallback(client_info);
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
bool SNetCreateGame(const char *pszGamePassword, _SNETGAMEDATA* gameData, int *playerID)
{
	char* gData = (char*)gameData;
	net::buffer_t game_init_info(gData, gData + sizeof(gameData));
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	char addrstr[129] = "0.0.0.0";
	getIniValue("dvlnet", "bindaddr", addrstr, 128);
	SStrCopy(gpszGameName, addrstr, sizeof(gpszGameName));
	if (pszGamePassword != NULL)
		SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	*playerID = dvlnet_inst->create(addrstr, pszGamePassword);
	return *playerID != -1;
}

bool SNetJoinGame(int id, char *pszGameName, char *pszGamePassword, int *playerID)
{
	if (pszGameName)
		SStrCopy(gpszGameName, pszGameName, sizeof(gpszGameName));
	if (pszGamePassword)
		SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	*playerID = dvlnet_inst->join(pszGameName, pszGamePassword);
	return *playerID != -1;
}

/**
 * @brief Is this the mirror image of SNetGetTurnsInTransit?
 */
bool SNetGetOwnerTurnsWaiting(DWORD *turns)
{
	return dvlnet_inst->SNetGetOwnerTurnsWaiting(turns);
}

bool SNetGetTurnsInTransit(DWORD *turns)
{
	return dvlnet_inst->SNetGetTurnsInTransit(turns);
}

/**
 * @brief engine calls this only once with argument 1
 */
bool SNetSetBasePlayer(int)
{
	return true;
}

DEVILUTION_END_NAMESPACE
