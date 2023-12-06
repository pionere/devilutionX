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

typedef void (*SEVTHANDLER)(SNetEventHdr*);

/*
 *  @param provider:      the type of the connection (conn_type)
 */
void SNetInitializeProvider(unsigned provider);

/*  SNetRegisterEventHandler @ 123
 *
 *  @param eventType:     The type of the event (event_type)
 *  @param func:          The function to be called
 */
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
 *  @param receiver:  The player index of the player to receive the data.
 *              Conversely, this field can be one of the following constants:
 *                  SNPLAYER_ALL      | Sends the message to all players, including oneself.
 *                  SNPLAYER_OTHERS   | Sends the message to all players, except for oneself.
 *  @param data:      A pointer to the data containing the message(s).
 *  @param databytes: The amount of bytes that the data pointer contains.
 *
 */
void SNetSendMessage(int receiver, const BYTE* data, unsigned databytes);
SNetMsgPkt* SNetReceiveMessage();

/*  SNetSendTurn @ 122
 *
 *  Sends a turn (data packet) to all players in the game. Network data
 *  is sent using class 02 and is retrieved by the other client using
 *  SNetReceiveTurn().
 *
 *  @param turn:       The turn index.
 *  @param data:       A pointer to the data containing the commands in the turn.
 *  @param databytes:  The amount of bytes that the data pointer contains.
 */
void SNetSendTurn(turn_t turn, const BYTE* data, unsigned databytes);
SNetTurnPkt* SNetReceiveTurn(unsigned (&status)[MAX_PLRS]);
/*
 *  Checks the status of the turn packets from the connected players.
 *
 *  @param status:     The array to hold the status of the connected players.
 * 
 *  @return the status of the next turn (turn_status)
 */
turn_status SNetPollTurns(unsigned (&status)[MAX_PLRS]);

/*  SNetGetTurnsInTransit @ 115
 *
 *  Retrieves the number of turns that have been sent over the network but not processed yet.
 *
 *  @return the number of turns
 */
unsigned SNetGetTurnsInTransit();
#ifdef ZEROTIER
bool SNetReady();
void SNetGetGamelist(std::vector<SNetZtGame>& games);
#endif

/*
 *  Drops a player from the current game.
 *
 *  @param playerid:     The player ID for the player to be dropped.
 */
void SNetDropPlayer(int playerid);

/*  SNetLeaveGame @ 119
 *
 *  Notifies Storm that the player has left the game. Storm will
 *  notify all connected peers through the network provider.
 *
 */
void SNetLeaveGame();

/*  
 *  Disconnect everyone but the local player from the current game.
 */
void SNetDisconnect();

/*
 *  Destroy the net-instance.
 */
void SNetDestroy();

#if defined(__GNUC__) || defined(__cplusplus)
}
#endif

DEVILUTION_END_NAMESPACE
