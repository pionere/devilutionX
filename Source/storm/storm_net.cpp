#include <memory>
#ifdef ZEROTIER
#include <mutex>
#include <thread>
#endif

#include "all.h"
#include "dvlnet/abstract_net.h"
#include "utils/stubs.h"

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

bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned int (&size)[MAX_PLRS],
    DWORD (&status)[MAX_PLRS])
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

int SNetGetProviderCaps(struct _SNETCAPS *caps)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetGetProviderCaps(caps);
}

BOOL SNetUnregisterEventHandler(int evtype, SEVTHANDLER func)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetUnregisterEventHandler(*(event_type *)&evtype, func);
}

BOOL SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetRegisterEventHandler(*(event_type *)&evtype, func);
}

bool SNetDestroy()
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return true;
}

bool SNetDropPlayer(int playerid, DWORD flags)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return dvlnet_inst->SNetDropPlayer(playerid, flags);
}

bool SNetGetGameInfo(int type, void *dst, unsigned int length)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
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

bool SNetLeaveGame(int type)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
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
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	dvlnet_inst = net::abstract_net::make_net(provider);
	return ui_info->selectnamecallback(client_info, user_info, ui_info, fileinfo, provider, NULL, 0, NULL, 0, NULL);
}

/**
 * @brief Called by engine for single, called by ui for multi
 */
bool SNetCreateGame(const char *pszGameName, const char *pszGamePassword, const char *pszGameStatString,
	DWORD dwGameType, char *GameTemplateData, int GameTemplateSize, int playerCount,
    const char *creatorName, const char *a11, int *playerID)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	if (GameTemplateSize != sizeof(_gamedata))
		ABORT();
	net::buffer_t game_init_info(GameTemplateData, GameTemplateData + GameTemplateSize);
	dvlnet_inst->setup_gameinfo(std::move(game_init_info));

	std::string default_name;
	if (!pszGameName) {
		default_name = dvlnet_inst->make_default_gamename();
		pszGameName = default_name.c_str();
	}

	SStrCopy(gpszGameName, pszGameName, sizeof(gpszGameName));
	if (pszGamePassword)
		SStrCopy(gpszGamePassword, pszGamePassword, sizeof(gpszGamePassword));
	*playerID = dvlnet_inst->create(pszGameName, pszGamePassword);
	return *playerID != -1;
}

bool SNetJoinGame(int id, char *pszGameName, char *pszGamePassword, char *playerName, char *userStats, int *playerID)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
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

/**
 * @brief engine calls this only once with argument 1
 */
bool SNetSetBasePlayer(int)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	return true;
}

/**
 * @brief since we never signal STORM_ERROR_REQUIRES_UPGRADE the engine will not call this function
 */
bool SNetPerformUpgrade(DWORD *upgradestatus)
{
#ifdef ZEROTIER
	std::lock_guard<std::mutex> lg(storm_net_mutex);
#endif
	UNIMPLEMENTED();
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
	dvlnet_inst->setup_password(pw);
}
#endif

DEVILUTION_END_NAMESPACE
