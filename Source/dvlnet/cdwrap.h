#pragma once
#ifdef ZEROTIER
//#include <exception>
//#include <map>
#include <memory>
//#include <string>
//#include <vector>

#include "abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

template <class T>
class cdwrap : public abstract_net {
private:
	std::unique_ptr<abstract_net> dvlnet_wrap;
	SEVTHANDLER registered_handlers[NUM_EVT_TYPES] = { };

	void reset();

public:
	virtual bool create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info, char (&errorText)[256]);
	virtual bool join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256]);
	virtual bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* size);
	virtual void SNetSendMessage(int receiver, const BYTE* data, unsigned int size);
	virtual SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn, const BYTE *data, unsigned size);
	virtual turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]);
	virtual uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]);
//#ifdef ADAPTIVE_NETUPDATE
	unsigned SNetGetTurnsInTransit();
//#endif
	virtual void SNetRegisterEventHandler(int evtype, SEVTHANDLER func);
	virtual void SNetUnregisterEventHandler(int evtype);
	virtual void SNetLeaveGame(int reason);
	virtual void SNetDropPlayer(int playerid);
	virtual void make_default_gamename(char (&gamename)[128]);

	cdwrap();
	virtual ~cdwrap() = default;
};

template <class T>
cdwrap<T>::cdwrap()
{
	reset();
}

template <class T>
void cdwrap<T>::reset()
{
	int i;

	dvlnet_wrap.reset(new T);

	for (i = 0; i < NUM_EVT_TYPES; i++)
		dvlnet_wrap->SNetRegisterEventHandler(i, registered_handlers[i]);
}

template <class T>
bool cdwrap<T>::create_game(const char* addrstr, unsigned port, const char* passwd, buffer_t info, char (&errorText)[256])
{
	reset();
	return dvlnet_wrap->create_game(addrstr, port, passwd, std::move(info), errorText);
}

template <class T>
bool cdwrap<T>::join_game(const char* addrstr, unsigned port, const char* passwd, char (&errorText)[256])
{
	reset();
	return dvlnet_wrap->join_game(addrstr, port, passwd, errorText);
}

template <class T>
bool cdwrap<T>::SNetReceiveMessage(int* sender, BYTE** data, unsigned* size)
{
	return dvlnet_wrap->SNetReceiveMessage(sender, data, size);
}

template <class T>
void cdwrap<T>::SNetSendMessage(int receiver, const BYTE* data, unsigned int size)
{
	dvlnet_wrap->SNetSendMessage(receiver, data, size);
}

template <class T>
SNetTurnPkt* cdwrap<T>::SNetReceiveTurn(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetReceiveTurn(status);
}

template <class T>
void cdwrap<T>::SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size)
{
	dvlnet_wrap->SNetSendTurn(turn, data, size);
}

template <class T>
turn_status cdwrap<T>::SNetPollTurns(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetPollTurns(status);
}

template <class T>
uint32_t cdwrap<T>::SNetLastTurn(unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetLastTurn(status);
}

template <class T>
void cdwrap<T>::SNetUnregisterEventHandler(int evtype)
{
	registered_handlers[evtype] = NULL;
	if (dvlnet_wrap)
		dvlnet_wrap->SNetUnregisterEventHandler(evtype);
}

template <class T>
void cdwrap<T>::SNetRegisterEventHandler(int evtype, SEVTHANDLER func)
{
	registered_handlers[evtype] = func;
	if (dvlnet_wrap)
		dvlnet_wrap->SNetRegisterEventHandler(evtype, func);
}

template <class T>
void cdwrap<T>::SNetLeaveGame(int reason)
{
	dvlnet_wrap->SNetLeaveGame(reason);
}

template <class T>
void cdwrap<T>::SNetDropPlayer(int playerid)
{
	dvlnet_wrap->SNetDropPlayer(playerid);
}

//#ifdef ADAPTIVE_NETUPDATE
template <class T>
unsigned cdwrap<T>::SNetGetTurnsInTransit()
{
	return dvlnet_wrap->SNetGetTurnsInTransit();
}
//#endif

template <class T>
void cdwrap<T>::make_default_gamename(char (&gamename)[128])
{
	dvlnet_wrap->make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
#endif // ZEROTIER