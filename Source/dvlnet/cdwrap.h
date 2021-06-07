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
	std::map<event_type, SEVTHANDLER> registered_handlers;
	buffer_t game_init_info;

	void reset();

public:
	virtual bool create(const std::string &addrstr, unsigned port, const std::string &passwd);
	virtual bool join(const std::string &addrstr, unsigned port, const std::string &passwd);
	virtual bool SNetReceiveMessage(int *sender, char **data, unsigned *size);
	virtual void SNetSendMessage(int dest, void *data, unsigned int size);
	virtual bool SNetReceiveTurns(uint32_t *(&data)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);
	virtual void SNetSendTurn(uint32_t turn);
	virtual void SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func);
	virtual void SNetUnregisterEventHandler(event_type evtype, SEVTHANDLER func);
	virtual void SNetLeaveGame(int type);
	virtual void SNetDropPlayer(int playerid);
	virtual uint32_t SNetGetOwnerTurnsWaiting();
	virtual uint32_t SNetGetTurnsInTransit();
	virtual void setup_gameinfo(buffer_t info);
	virtual std::string make_default_gamename();

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
	dvlnet_wrap.reset(new T);
	dvlnet_wrap->setup_gameinfo(game_init_info);

	for (const auto &pair : registered_handlers)
		dvlnet_wrap->SNetRegisterEventHandler(pair.first, pair.second);
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
void cdwrap<T>::SNetSendMessage(int playerID, void *data, unsigned int size)
{
	dvlnet_wrap->SNetSendMessage(playerID, data, size);
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
void cdwrap<T>::SNetUnregisterEventHandler(event_type evtype, SEVTHANDLER func)
{
	registered_handlers.erase(evtype);
	if (dvlnet_wrap)
		dvlnet_wrap->SNetUnregisterEventHandler(evtype, func);
}

template <class T>
void cdwrap<T>::SNetRegisterEventHandler(event_type evtype, SEVTHANDLER func)
{
	registered_handlers[evtype] = func;
	if (dvlnet_wrap)
		dvlnet_wrap->SNetRegisterEventHandler(evtype, func);
}

template <class T>
void cdwrap<T>::SNetLeaveGame(int type)
{
	dvlnet_wrap->SNetLeaveGame(type);
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
std::string cdwrap<T>::make_default_gamename()
{
	return dvlnet_wrap->make_default_gamename();
}

} // namespace net
DEVILUTION_END_NAMESPACE
