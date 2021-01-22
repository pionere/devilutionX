#include <memory>

#include "all.h"
#include "stubs.h"
#include "dvlnet/abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE

static std::unique_ptr<net::abstract_net> dvlnet_inst;
static char gpszGameName[128] = {};
static char gpszGamePassword[128] = {};

BOOL SNetReceiveMessage(int *senderplayerid, char **data, int *databytes)
{
	if (!dvlnet_inst->SNetReceiveMessage(senderplayerid, data, databytes)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

BOOL SNetSendMessage(int playerID, void *data, unsigned int databytes)
{
	return dvlnet_inst->SNetSendMessage(playerID, data, databytes);
}

BOOL SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned int (&size)[MAX_PLRS],
    DWORD (&status)[MAX_PLRS])
{
	if (!dvlnet_inst->SNetReceiveTurns(data, size, status)) {
		SErrSetLastError(STORM_ERROR_NO_MESSAGES_WAITING);
		return false;
	}
	return true;
}

BOOL SNetSendTurn(char *data, unsigned int databytes)
{
	return dvlnet_inst->SNetSendTurn(data, databytes);
}

int SNetGetProviderCaps(struct _SNETCAPS *caps)
{
	return dvlnet_inst->SNetGetProviderCaps(caps);
}

BOOL SNetUnregisterEventHandler(int evtype, SEVTHANDLER func)
{
	return dvlnet_inst->SNetUnregisterEventHandler(*(event_type *)&evtype, func);
}

BOOL SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	return dvlnet_inst->SNetRegisterEventHandler(*(event_type *)&evtype, func);
}

BOOL SNetDestroy()
{
	return true;
}

BOOL SNetDropPlayer(int playerid, DWORD flags)
{
	return dvlnet_inst->SNetDropPlayer(playerid, flags);
}

BOOL SNetGetGameInfo(int type, void *dst, unsigned int length)
{
	switch (type) {
	case GAMEINFO_NAME:
		SStrCopy((char *)dst, gpszGameName, length);
		break;
	case GAMEINFO_PASSWORD:
		SStrCopy((char *)dst, gpszGamePassword, length);
		break;
	}

	return true;
}

BOOL SNetLeaveGame(int type)
{
	if (dvlnet_inst == NULL)
		return true;
	return dvlnet_inst->SNetLeaveGame(type);
}

/**
 * @brief Called by engine for single, called by ui for multi
 * @param provider BNET, IPXN, MODM, SCBL or UDPN
 * @param fileinfo Ignore
 */
int SNetInitializeProvider(unsigned long provider, struct _SNETPROGRAMDATA *client_info,
    struct _SNETPLAYERDATA *user_info, struct _SNETUIDATA *ui_info,
    struct _SNETVERSIONDATA *fileinfo)
{
	dvlnet_inst = net::abstract_net::make_net(provider);
	return ui_info->selectnamecallback(client_info, user_info, ui_info, fileinfo, provider, NULL, 0, NULL, 0, NULL);
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
BOOL SNetCreateGame(const char *pszGameName, const char *pszGamePassword, const char *pszGameStatString,
	DWORD dwGameType, char *GameTemplateData, int GameTemplateSize, int playerCount,
    const char *creatorName, const char *a11, int *playerID)
{
	if (GameTemplateSize != sizeof(_gamedata))
		ABORT();
	net::buffer_t game_init_info(GameTemplateData, GameTemplateData + GameTemplateSize);
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	char addrstr[129] = "0.0.0.0";
	getIniValue("dvlnet", "bindaddr", addrstr, 128);
	SStrCopy(gpszGameName, addrstr, sizeof(gpszGameName));
	if (pszGamePassword)
		SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	*playerID = dvlnet_inst->create(addrstr, pszGamePassword);
	return *playerID != -1;
}

BOOL SNetJoinGame(int id, char *pszGameName, char *pszGamePassword, char *playerName, char *userStats, int *playerID)
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
BOOL SNetGetOwnerTurnsWaiting(DWORD *turns)
{
	return dvlnet_inst->SNetGetOwnerTurnsWaiting(turns);
}

BOOL SNetGetTurnsInTransit(DWORD *turns)
{
	return dvlnet_inst->SNetGetTurnsInTransit(turns);
}

/**
 * @brief engine calls this only once with argument 1
 */
BOOLEAN SNetSetBasePlayer(int)
{
	return true;
}

/**
 * @brief since we never signal STORM_ERROR_REQUIRES_UPGRADE the engine will not call this function
 */
BOOL SNetPerformUpgrade(DWORD *upgradestatus)
{
	UNIMPLEMENTED();
}

DEVILUTION_END_NAMESPACE
