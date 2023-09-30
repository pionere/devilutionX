#include "cdwrap.h"
#ifdef ZEROTIER

DEVILUTION_BEGIN_NAMESPACE
namespace net {

cdwrap::cdwrap()
{
	reset();
}

void cdwrap::reset()
{
	int i;

	dvlnet_wrap.reset();

	for (i = 0; i < NUM_EVT_TYPES; i++)
		dvlnet_wrap->SNetRegisterEventHandler(i, registered_handlers[i]);
}

bool cdwrap::create_game(const char* addrstr, unsigned port, const char* passwd, _uigamedata* gameData, char (&errorText)[256])
{
	reset();
	return dvlnet_wrap->create_game(addrstr, port, passwd, gameData, errorText);
}

bool cdwrap::join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	reset();
	return dvlnet_wrap->join_game(addrstr, port, passwd, errorText);
}

bool cdwrap::SNetReceiveMessage(int* sender, BYTE** data, unsigned* size)
{
	return dvlnet_wrap->SNetReceiveMessage(sender, data, size);
}

void cdwrap::SNetSendMessage(int receiver, const BYTE* data, unsigned int size)
{
	dvlnet_wrap->SNetSendMessage(receiver, data, size);
}

SNetTurnPkt* cdwrap::SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetReceiveTurn(status);
}

void cdwrap::SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size)
{
	dvlnet_wrap->SNetSendTurn(turn, data, size);
}

turn_status cdwrap::SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetPollTurns(status);
}

uint32_t cdwrap::SNetLastTurn(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetLastTurn(status);
}

void cdwrap::SNetUnregisterEventHandler(int evtype)
{
	registered_handlers[evtype] = NULL;
	if (dvlnet_wrap)
		dvlnet_wrap->SNetUnregisterEventHandler(evtype);
}

void cdwrap::SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	registered_handlers[evtype] = func;
	if (dvlnet_wrap)
		dvlnet_wrap->SNetRegisterEventHandler(evtype, func);
}

void cdwrap::SNetLeaveGame(int reason)
{
	dvlnet_wrap->SNetLeaveGame(reason);
}

void cdwrap::SNetDropPlayer(int playerid)
{
	dvlnet_wrap->SNetDropPlayer(playerid);
}

unsigned cdwrap::SNetGetTurnsInTransit()
{
	return dvlnet_wrap->SNetGetTurnsInTransit();
}

void cdwrap::make_default_gamename(char (&gamename)[NET_MAX_GAMENAME_LEN + 1])
{
	dvlnet_wrap->make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER
