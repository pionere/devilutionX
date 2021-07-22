#pragma once

#include <exception>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "dvlnet/abstract_net.h"

DEVILUTION_BEGIN_NAMESPACE
namespace net {

template <class T>
class cdwrap : public abstract_net {
private:
	std::unique_ptr<abstract_net> dvlnet_wrap;
	SEVTHANDLER registered_handlers[NUM_EVT_TYPES] = { };
	buffer_t game_init_info;

	void reset();

public:
	virtual bool create(const std::string &addrstr, unsigned port, const std::string &passwd);
	virtual bool join(const std::string &addrstr, unsigned port, const std::string &passwd);
	virtual bool SNetReceiveMessage(int *sender, char **data, unsigned *size);
	virtual void SNetSendMessage(int dest, const void *data, unsigned int size);
	virtual bool SNetReceiveTurns(uint32_t *(&data)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn);
	virtual void SNetRegisterEventHandler(int evtype, SEVTHANDLER func);
	virtual void SNetUnregisterEventHandler(int evtype);
	virtual void SNetLeaveGame(int reason);
	virtual void SNetDropPlayer(int playerid);
	virtual uint32_t SNetGetOwnerTurnsWaiting();
	virtual uint32_t SNetGetTurnsInTransit();
	virtual void setup_gameinfo(buffer_t info);
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
	dvlnet_wrap->setup_gameinfo(game_init_info);

	for (i = 0; i < NUM_EVT_TYPES; i++)
		dvlnet_wrap->SNetRegisterEventHandler(i, registered_handlers[i]);
}

template <class T>
bool cdwrap<T>::create(const std::string &addrstr, unsigned port, const std::string &passwd)
{
	reset();
	return dvlnet_wrap->create(addrstr, port, passwd);
}

template <class T>
bool cdwrap<T>::join(const std::string &addrstr, unsigned port, const std::string &passwd)
{
	game_init_info = buffer_t();
	reset();
	return dvlnet_wrap->join(addrstr, port, passwd);
}

template <class T>
void cdwrap<T>::setup_gameinfo(buffer_t info)
{
	game_init_info = std::move(info);
	if (dvlnet_wrap)
		dvlnet_wrap->setup_gameinfo(game_init_info);
}

template <class T>
bool cdwrap<T>::SNetReceiveMessage(int *sender, char **data, unsigned *size)
{
	return dvlnet_wrap->SNetReceiveMessage(sender, data, size);
}

template <class T>
void cdwrap<T>::SNetSendMessage(int dest, const void *data, unsigned int size)
{
	dvlnet_wrap->SNetSendMessage(dest, data, size);
}

template <class T>
bool cdwrap<T>::SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS])
{
	return dvlnet_wrap->SNetReceiveTurns(turns, status);
}

template <class T>
void cdwrap<T>::SNetSendTurn(uint32_t turn)
{
	dvlnet_wrap->SNetSendTurn(turn);
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

template <class T>
uint32_t cdwrap<T>::SNetGetOwnerTurnsWaiting()
{
	return dvlnet_wrap->SNetGetOwnerTurnsWaiting();
}

template <class T>
uint32_t cdwrap<T>::SNetGetTurnsInTransit()
{
	return dvlnet_wrap->SNetGetTurnsInTransit();
}

template <class T>
void cdwrap<T>::make_default_gamename(char (&gamename)[128])
{
	dvlnet_wrap->make_default_gamename(gamename);
}

} // namespace net
DEVILUTION_END_NAMESPACE
