#pragma once

#include <limits>
#include <string>
namespace dvl {

// Note to self: Linker error => forgot a return value in cpp

// Storm API definition
#ifndef STORMAPI
#define STORMAPI
#endif

#ifdef __cplusplus
struct CCritSect {
	SDL_mutex *m_critsect;

	CCritSect()
	{
		m_critsect = SDL_CreateMutex();
		if (m_critsect == NULL) {
			app_fatal("Failed to create mutex. (%s)", SDL_GetError());
		}
	}
	~CCritSect()
	{
		SDL_DestroyMutex(m_critsect);
	}
	void Enter()
	{
		if (SDL_LockMutex(m_critsect) < 0) {
			app_fatal("Failed to lock mutex. (%s)", SDL_GetError());
		}
	}
	void Leave()
	{
		if (SDL_UnlockMutex(m_critsect) < 0) {
			app_fatal("Failed to unlock mutex. (%s)", SDL_GetError());
		}
	}
};
#endif

// Game states
#define GAMESTATE_PRIVATE 0x01
#define GAMESTATE_FULL    0x02
#define GAMESTATE_ACTIVE  0x04
#define GAMESTATE_STARTED 0x08
#define GAMESTATE_REPLAY  0x80

#define PS_CONNECTED 0x10000
#define PS_TURN_ARRIVED 0x20000
#define PS_ACTIVE 0x40000

#define LEAVE_ENDING 0x40000004
#define LEAVE_DROP 0x40000006

#if defined(__GNUC__) || defined(__cplusplus)
extern "C" {
#endif

bool STORMAPI SNetCreateGame(const char *pszGamePassword, struct _SNETGAMEDATA *gameData);
bool STORMAPI SNetDestroy();

/*  SNetDropPlayer @ 106
 *
 *  Drops a player from the current game.
 *
 *  playerid:     The player ID for the player to be dropped.
 *  flags:
 *
 *  Returns TRUE if the function was called successfully and FALSE otherwise.
 */
bool
    STORMAPI
    SNetDropPlayer(
        int playerid,
        unsigned flags);

/*  SNetGetGameInfo @ 107
 *
 *  @param name:         The address and port of the game
 *  @param password:     The password of the game
 */
void
    STORMAPI
    SNetGetGameInfo(const char **name, const char **password);

/*  SNetGetTurnsInTransit @ 115
 *
 *  Retrieves the number of turns (buffers) that have been queued
 *  before sending them over the network.
 *
 *  turns: A pointer to an integer that will receive the value.
 *
 *  Returns TRUE if the function was called successfully and FALSE otherwise.
 */
bool
    STORMAPI
    SNetGetTurnsInTransit(
        DWORD *turns);

bool STORMAPI SNetJoinGame(const char *gameName, unsigned port, const char *gamePassword);

/*  SNetLeaveGame @ 119
 *
 *  Notifies Storm that the player has left the game. Storm will
 *  notify all connected peers through the network provider.
 *
 *  type: The leave type. It doesn't appear to be important, no documentation available.
 *
 */
void
    STORMAPI
    SNetLeaveGame(
        int type);

bool STORMAPI SNetReceiveMessage(int *senderplayerid, char **data, int *databytes);
bool STORMAPI SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned (&size)[MAX_PLRS], unsigned (&status)[MAX_PLRS]);

typedef void(STORMAPI *SEVTHANDLER)(struct _SNETEVENT *);

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
 *  Returns TRUE if the function was called successfully and FALSE otherwise.
 */
bool
    STORMAPI
    SNetSendMessage(
        int playerID,
        void *data,
        unsigned int databytes);

// Macro values to target specific players
#define SNPLAYER_ALL    -1
#define SNPLAYER_OTHERS -2

#define MPQ_FLAG_READ_ONLY			1
#define MPQ_FILE_IMPLODE			0x00000100
#define MPQ_FILE_EXISTS				0x80000000
#define MPQ_HASH_TABLE_INDEX		0x000
#define MPQ_HASH_NAME_A				0x100
#define MPQ_HASH_NAME_B				0x200
#define MPQ_HASH_FILE_KEY			0x300
#define SFILE_OPEN_FROM_MPQ			0
#define SFILE_OPEN_LOCAL_FILE		0xFFFFFFFF
#define SFILE_OPEN_CHECK_EXISTS		0xFFFFFFFC

/*  SNetSendTurn @ 128
 *
 *  Sends a turn (data packet) to all players in the game. Network data
 *  is sent using class 02 and is retrieved by the other client using
 *  SNetReceiveTurns().
 *
 *  data:       A pointer to the data.
 *  databytes:  The amount of bytes that the data pointer contains.
 *
 *  Returns TRUE if the function was called successfully and FALSE otherwise.
 */
bool
    STORMAPI
    SNetSendTurn(
        char *data,
        unsigned int databytes);

bool STORMAPI SFileCloseArchive(HANDLE hArchive);
void STORMAPI SFileCloseFile(HANDLE hFile);

DWORD STORMAPI SFileGetFileSize(HANDLE hFile);
DWORD STORMAPI SFileGetFilePointer(HANDLE hFile);
DWORD STORMAPI SFileSetFilePointer(HANDLE hFile, long lFilePos, unsigned dwMoveMethod);
bool STORMAPI SFileOpenArchive(const char *szMpqName, DWORD dwFlags, HANDLE *phMpq);

bool STORMAPI SFileOpenFile(const char *filename, HANDLE *phFile);
bool STORMAPI SFileOpenFileEx(HANDLE hMpq, const char *szFileName, DWORD dwSearchScope, HANDLE *phFile);

bool STORMAPI SFileReadFile(HANDLE hFile, void *buffer, DWORD nNumberOfBytesToRead, DWORD *read);

/*  SBmpLoadImage @ 323
 *
 *  Load an image from an available archive into a buffer.
 *
 *  pszFileName:  The name of the graphic in an active archive.
 *  pPalette:     An optional buffer that receives the image palette.
 *  pBuffer:      A buffer that receives the image data.
 *  dwBuffersize: The size of the specified image buffer.
 *  pdwWidth:     An optional variable that receives the image width.
 *  pdwHeight:    An optional variable that receives the image height.
 *  pdwBpp:       An optional variable that receives the image bits per pixel.
 *
 *  Returns TRUE if the image was supported and loaded correctly, FALSE otherwise.
 */
bool
    STORMAPI
    SBmpLoadImage(
        const char *pszFileName,
        SDL_Color *pPalette,
        BYTE *pBuffer,
        DWORD dwBuffersize,
        DWORD *pdwWidth,
        DWORD *pdwHeight,
        DWORD *pdwBpp);

/*  SMemAlloc @ 401
 *
 *  Allocates a block of memory. This block is different
 *  from the standard malloc by including a header containing
 *  information about the block.
 *
 *  amount:       The amount of memory to allocate, in bytes.
 *
 *  Returns a pointer to the allocated memory. This pointer does NOT include
 *  the additional storm header.
 */
void *STORMAPI SMemAlloc(size_t amount);

/*  SMemFree @ 403
 *
 *  Frees a block of memory that was created using SMemAlloc,
 *  includes the log file and line for debugging purposes.
 *
 *  location:     The memory location to be freed.
 *
 */
void STORMAPI SMemFree(void *location);

bool getIniBool(const char *sectionName, const char *keyName, bool defaultValue);
bool getIniValue(const char *sectionName, const char *keyName, char *string, int stringSize);
void setIniValue(const char *sectionName, const char *keyName, const char *value);
bool getIniInt(const char *sectionName, const char *keyName, int *value);
void setIniInt(const char *sectionName, const char *keyName, int value);

HANDLE SVidPlayBegin(const char *filename, int flags);
void SVidPlayEnd();

/*  SErrGetLastError @ 463
 *
 *  Retrieves the last error that was specifically
 *  set for the Storm library.
 *
 *  Returns the last error set within the Storm library.
 */
DWORD
STORMAPI
SErrGetLastError();

/*  SErrSetLastError @ 465
 *
 *  Sets the last error for the Storm library and the Kernel32 library.
 *
 *  dwErrCode:  The error code that will be set.
 */
void
    STORMAPI
    SErrSetLastError(
        DWORD dwErrCode);

// Values for dwErrCode
#define STORM_ERROR_GAME_TERMINATED              0x85100069
#define STORM_ERROR_INVALID_PLAYER               0x8510006a
#define STORM_ERROR_NO_MESSAGES_WAITING          0x8510006b
#define STORM_ERROR_NOT_IN_GAME                  0x85100070
#define STORM_ERROR_REQUIRES_UPGRADE             0x85100077

/*  SStrCopy @ 501
 *
 *  Copies a string from src to dest (including NULL terminator)
 *  until the max_length is reached.
 *
 *  dest:         The destination array.
 *  src:          The source array.
 *  max_length:   The maximum length of dest.
 *
 */
void
    STORMAPI
    SStrCopy(
        char *dest,
        const char *src,
        int max_length);

void SFileSetBasePath(const char *);
bool SVidPlayContinue(void);
bool SNetGetOwnerTurnsWaiting(DWORD *);
bool SNetUnregisterEventHandler(int, SEVTHANDLER);
bool SNetRegisterEventHandler(int, SEVTHANDLER);
void SNetInitializeProvider(unsigned long provider);
bool SNetGetProviderCaps(struct _SNETCAPS *);

void  InitializeMpqCryptography();
void  EncryptMpqBlock(void * pvDataBlock, DWORD dwLength, DWORD dwKey);
void  DecryptMpqBlock(void * pvDataBlock, DWORD dwLength, DWORD dwKey);
DWORD HashStringSlash(const char * szFileName, unsigned dwHashType);
void SFileEnableDirectAccess(bool enable);
void SLoadKeyMap(BYTE (&map)[256]);

#if defined(__GNUC__) || defined(__cplusplus)
}
#endif

} // namespace dvl
