#pragma once

#ifdef ZEROTIER
#include <vector>
#include <string>
#endif

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#if defined(__GNUC__) || defined(__cplusplus)
extern "C" {
#endif

// Macro values to target specific players
#define SNPLAYER_ALL    0xFF
#define SNPLAYER_MASTER 0xFE
//#define SNPLAYER_OTHERS -2

typedef void (*SEVTHANDLER)(SNetEvent*);

void SNetInitializeProvider(unsigned provider);
void SNetRegisterEventHandler(int eventType, SEVTHANDLER func);
void SNetUnregisterEventHandler(int eventType);

bool SNetCreateGame(unsigned port, const char* pszGamePassword, _uigamedata* gameData, char (&errorText)[256]);

bool SNetJoinGame(const char* gameName, unsigned port, const char* gamePassword, char (&errorText)[256]);

/*  SNetGetGameInfo @ 107
 *
 *  @param name:         The address and port of the game
 *  @param password:     The password of the game
 */
void SNetGetGameInfo(const char** name, const char** password);

/*  SNetSendMessage @ 127
 *
 *  Sends a message to a player given their player ID. Network message
 *  is sent using class 01 and is retrieved by the other client using
 *  SNetReceiveMessage().
 *
 *  receiver:   The player index of the player to receive the data.
 *              Conversely, this field can be one of the following constants:
 *                  SNPLAYER_ALL      | Sends the message to all players, including oneself.
 *                  SNPLAYER_OTHERS   | Sends the message to all players, except for oneself.
 *  data:       A pointer to the data.
 *  databytes:  The amount of bytes that the data pointer contains.
 *
 */
void SNetSendMessage(int receiver, const BYTE* data, unsigned databytes);
bool SNetReceiveMessage(int* sender, BYTE** data, unsigned* databytes);

/*  SNetSendTurn @ 128
 *
 *  Sends a turn (data packet) to all players in the game. Network data
 *  is sent using class 02 and is retrieved by the other client using
 *  SNetReceiveTurn().
 *
 *  turn:       the data packet.
 *
 *  Returns TRUE if the function was called successfully and FALSE otherwise.
 */
void SNetSendTurn(uint32_t turn, const BYTE* data, unsigned size);
SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]);
turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]);
uint32_t SNetLastTurn(unsigned (&status)[MAX_PLRS]);
/*  SNetGetTurnsInTransit @ 115
 *
 *  Retrieves the number of turns that have been sent over the network but not processed yet.
 *
 *  @return the number of turns
 */
unsigned SNetGetTurnsInTransit();
#ifdef ZEROTIER
void SNetSendInfoRequest();
std::vector<std::string> SNetGetGamelist();
void SNetSetPassword(std::string pw);
#endif

/*  SNetDropPlayer @ 106
 *
 *  Drops a player from the current game.
 *
 *  playerid:     The player ID for the player to be dropped.
 *
 */
void SNetDropPlayer(int playerid);

/*  SNetLeaveGame @ 119
 *
 *  Notifies Storm that the player has left the game. Storm will
 *  notify all connected peers through the network provider.
 *
 */
void SNetLeaveGame();

void SNetDisconnect();

#if defined(__GNUC__) || defined(__cplusplus)
}
#endif

DEVILUTION_END_NAMESPACE
