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

bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned (&size)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
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

void SNetLeaveGame(int type)
{
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
	dvlnet_inst = net::abstract_net::make_net(provider);
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
bool SNetCreateGame(const char *pszGamePassword, _SNETGAMEDATA* gameData)
{
	// assert(gameData != NULL && pszGamePassword != NULL);

	char* gData = (char*)gameData;
	net::buffer_t game_init_info(gData, gData + sizeof(*gameData));
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	char addrstr[129] = "0.0.0.0";
	getIniValue("Network", "Bind Address", addrstr, 128);
	SStrCopy(gpszGameName, addrstr, sizeof(gpszGameName));
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	return dvlnet_inst->create(addrstr, pszGamePassword);
}

bool SNetJoinGame(char *pszGameName, char *pszGamePassword)
{
	// assert(pszGameName != NULL && pszGamePassword != NULL);

	SStrCopy(gpszGameName, pszGameName, sizeof(gpszGameName));
	SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	return dvlnet_inst->join(pszGameName, pszGamePassword);
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

DEVILUTION_END_NAMESPACE
