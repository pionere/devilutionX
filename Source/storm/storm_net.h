#pragma once

#ifdef ZEROTIER
#include <vector>
#include <string>
#endif

DEVILUTION_BEGIN_NAMESPACE

#if defined(__GNUC__) || defined(__cplusplus)
extern "C" {
#endif

bool SNetCreateGame(const char *pszGamePassword, struct _SNETGAMEDATA *gameData);
bool SNetDestroy();

/*  SNetDropPlayer @ 106
 *
 *  Drops a player from the current game.
 *
 *  playerid:     The player ID for the player to be dropped.
 *
 */
void SNetDropPlayer(int playerid);

/*  SNetGetGameInfo @ 107
 *
 *  @param name:         The address and port of the game
 *  @param password:     The password of the game
 */
void SNetGetGameInfo(const char **name, const char **password);

bool SNetJoinGame(const char *gameName, unsigned port, const char *gamePassword);

/*  SNetLeaveGame @ 119
 *
 *  Notifies Storm that the player has left the game. Storm will
 *  notify all connected peers through the network provider.
 *
 *  reason: see leave_reason enum.
 *
 */
void SNetLeaveGame(int reason);

/*  SNetSendTurn @ 128
 *
 *  Sends a turn (data packet) to all players in the game. Network data
 *  is sent using class 02 and is retrieved by the other client using
 *  SNetReceiveTurns().
 *
 *  turn:       the data packet.
 *
 *  Returns TRUE if the function was called successfully and FALSE otherwise.
 */
void SNetSendTurn(uint32_t turn);
bool SNetReceiveTurns(uint32_t *(&turns)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);

/*  SNetGetTurnsInTransit @ 115
 *
 *  Retrieves the number of turns (buffers) that have been queued
 *  before sending them over the network.
 *
 *  @return the number of turns
 */
uint32_t SNetGetTurnsInTransit();
uint32_t SNetGetOwnerTurnsWaiting();

typedef void (*SEVTHANDLER)(struct _SNETEVENT *);

/*  SNetSendMessage @ 127
 *
 *  Sends a message to a player given their player ID. Network message
 *  is sent using class 01 and is retrieved by the other client using
 *  SNetReceiveMessage().
 *
 *  playerID:   The player index of the player to receive the data.
 *              Conversely, this field can be one of the following constants:
 *                  SNPLAYER_ALL      | Sends the message to all players, including oneself.
 *                  SNPLAYER_OTHERS   | Sends the message to all players, except for oneself.
 *  data:       A pointer to the data.
 *  databytes:  The amount of bytes that the data pointer contains.
 *
 */
void SNetSendMessage(int playerID, void *data, unsigned databytes);
bool SNetReceiveMessage(int *senderplayerid, char **data, unsigned *databytes);

void SNetUnregisterEventHandler(event_type, SEVTHANDLER);
void SNetRegisterEventHandler(event_type, SEVTHANDLER);
void SNetInitializeProvider(unsigned long provider);
#ifdef ZEROTIER
void SNetSendInfoRequest();
std::vector<std::string> SNetGetGamelist();
void SNetSetPassword(std::string pw);
#endif

// Macro values to target specific players
#define SNPLAYER_ALL    -1
#define SNPLAYER_OTHERS -2

#if defined(__GNUC__) || defined(__cplusplus)
}
#endif

DEVILUTION_END_NAMESPACE
