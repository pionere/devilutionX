#pragma once

#include <cerrno>
#include <limits>
#include <string>
#include <SDL.h>
#include "appfat.h"
#include "miniwin/miniwin.h"

namespace dvl {

// Note to self: Linker error => forgot a return value in cpp

// We declare the StormLib methods we use here.
// StormLib uses the Windows calling convention on Windows for these methods.
#ifdef _WIN32
#define WINAPI __stdcall
#else
#define WINAPI
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

bool SNetCreateGame(const char *pszGameName, const char *pszGamePassword, const char *pszGameStatString, DWORD dwGameType, char *GameTemplateData, int GameTemplateSize, int playerCount, const char *creatorName, const char *a11, int *playerID);
bool SNetDestroy();

/*  SNetDropPlayer @ 106
 *
 *  Drops a player from the current game.
 *
 *  playerid:     The player ID for the player to be dropped.
 *  flags:
 *
 *  Returns true if the function was called successfully and false otherwise.
 */
bool SNetDropPlayer(int playerid, DWORD flags);

/*  SNetGetGameInfo @ 107
 *
 *  Retrieves specific game information from Storm, such as name, password,
 *  stats, mode, game template, and players.
 *
 *  type:         The type of data to retrieve. See GAMEINFO_ flags.
 *  dst:          The destination buffer for the data.
 *  length:       The maximum size of the destination buffer.
 *
 *  Returns true if the function was called successfully and false otherwise.
 */
bool SNetGetGameInfo(int type, void *dst, unsigned int length);

/*  SNetGetTurnsInTransit @ 115
 *
 *  Retrieves the number of turns (buffers) that have been queued
 *  before sending them over the network.
 *
 *  turns: A pointer to an integer that will receive the value.
 *
 *  Returns true if the function was called successfully and false otherwise.
 */
bool
    SNetGetTurnsInTransit(
        DWORD *turns);

// Network provider structures
typedef struct _client_info {
	DWORD dwSize; // 60
	char *pszName;
	char *pszVersion;
	DWORD dwProduct;
	DWORD dwVerbyte;
	DWORD dwUnk5;
	DWORD dwMaxPlayers;
	DWORD dwUnk7;
	DWORD dwUnk8;
	DWORD dwUnk9;
	DWORD dwUnk10; // 0xFF
	char *pszCdKey;
	char *pszCdOwner;
	DWORD dwIsShareware;
	DWORD dwLangId;
} client_info;

typedef struct _user_info {
	DWORD dwSize; // 16
	char *pszPlayerName;
	char *pszUnknown;
	DWORD dwUnknown;
} user_info;

bool SNetJoinGame(int id, char *gameName, char *gamePassword, char *playerName, char *userStats, int *playerid);

/*  SNetLeaveGame @ 119
 *
 *  Notifies Storm that the player has left the game. Storm will
 *  notify all connected peers through the network provider.
 *
 *  type: The leave type. It doesn't appear to be important, no documentation available.
 *
 *  Returns true if the function was called successfully and false otherwise.
 */
bool SNetLeaveGame(int type);

bool SNetPerformUpgrade(DWORD *upgradestatus);
bool SNetReceiveMessage(int *senderplayerid, char **data, int *databytes);
bool SNetReceiveTurns(char *(&data)[MAX_PLRS], unsigned int (&size)[MAX_PLRS], DWORD (&status)[MAX_PLRS]);

typedef void(*SEVTHANDLER)(struct _SNETEVENT *);

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
 *  Returns true if the function was called successfully and false otherwise.
 */
bool SNetSendMessage(int playerID, void *data, unsigned int databytes);

// Macro values to target specific players
#define SNPLAYER_ALL    -1
#define SNPLAYER_OTHERS -2

#define MPQ_FLAG_READ_ONLY          1
#define MPQ_FILE_IMPLODE			0x00000100
#define MPQ_FILE_EXISTS				0x80000000
#define MPQ_HASH_TABLE_INDEX		0x000
#define MPQ_HASH_NAME_A				0x100
#define MPQ_HASH_NAME_B				0x200
#define MPQ_HASH_FILE_KEY			0x300
#define HASH_ENTRY_DELETED          0xFFFFFFFE  // Block index for deleted entry in the hash table
#define HASH_ENTRY_FREE             0xFFFFFFFF  // Block index for free entry in the hash table
#define SFILE_OPEN_FROM_MPQ         0
#define SFILE_OPEN_LOCAL_FILE       0xFFFFFFFF
#define SFILE_OPEN_CHECK_EXISTS     0xFFFFFFFC

bool WINAPI SFileCloseArchive(HANDLE hArchive);
bool WINAPI SFileCloseFile(HANDLE hFile);

LONG WINAPI SFileGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
bool WINAPI SFileOpenArchive(const char *szMpqName, DWORD dwFlags, HANDLE *phMpq);

bool SFileOpenFile(const char *filename, HANDLE *phFile);
BOOL WINAPI SFileOpenFileEx(HANDLE hMpq, const char *szFileName, DWORD dwSearchScope, HANDLE *phFile);

BOOL WINAPI SFileReadFile(HANDLE hFile, void *buffer, DWORD nNumberOfBytesToRead, DWORD *read, LONG *lpDistanceToMoveHigh);
DWORD WINAPI SFileSetFilePointer(HANDLE, LONG, LONG *, DWORD);

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
 *  Returns true if the image was supported and loaded correctly, false otherwise.
 */
bool
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
 *  logfilename:  The name of the file or object that this call belongs to.
 *  logline:      The line in the file or one of the SLOG_ macros.
 *  defaultValue: The default value of a byte in the allocated memory.
 *
 *  Returns a pointer to the allocated memory. This pointer does NOT include
 *  the additional storm header.
 */
void *
    SMemAlloc(
        unsigned int amount,
        const char *logfilename,
        int logline,
        int defaultValue);

/*  SMemFree @ 403
 *
 *  Frees a block of memory that was created using SMemAlloc,
 *  includes the log file and line for debugging purposes.
 *
 *  location:     The memory location to be freed.
 *  logfilename:  The name of the file or object that this call belongs to.
 *  logline:      The line in the file or one of the SLOG_ macros.
 *  defaultValue:
 *
 *  Returns TRUE if the call was successful and FALSE otherwise.
 */
BOOL
    SMemFree(
        void *location,
        const char *logfilename,
        int logline,
        char defaultValue);

bool getIniBool(const char *sectionName, const char *keyName, bool defaultValue = false);
bool getIniValue(const char *sectionName, const char *keyName, char *string, int stringSize, int *dataSize = NULL);
void setIniValue(const char *sectionName, const char *keyName, const char *value, int len = 0);
BOOL SRegLoadValue(const char *keyname, const char *valuename, BYTE flags, int *value);
BOOL SRegSaveValue(const char *keyname, const char *valuename, BYTE flags, DWORD result);

void SVidPlayBegin(const char *filename, int flags, HANDLE *video);
void SVidPlayEnd(HANDLE video);

// These error codes are used and returned by StormLib.
// See StormLib/src/StormPort.h
#if defined(_WIN32)
// https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
#define STORM_ERROR_FILE_NOT_FOUND 2
#define STORM_ERROR_HANDLE_EOF 38
#else // !defined(_WIN32)
#define STORM_ERROR_FILE_NOT_FOUND ENOENT
#define STORM_ERROR_HANDLE_EOF 1002
#endif

/*  SErrGetLastError @ 463
 *
 *  Retrieves the last error that was specifically
 *  set for the Storm library.
 *
 *  Returns the last error set within the Storm library.
 */
DWORD SErrGetLastError();

/*  SErrSetLastError @ 465
 *
 *  Sets the last error for the Storm library and the Kernel32 library.
 *
 *  dwErrCode:  The error code that will be set.
 */
void SErrSetLastError(DWORD dwErrCode);

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
void SStrCopy(char *dest, const char *src, int max_length);

bool SFileSetBasePath(const char *);
bool SVidPlayContinue(void);

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
bool SNetSendTurn(char *data, unsigned int databytes);
bool SNetGetOwnerTurnsWaiting(DWORD *);
BOOL SNetUnregisterEventHandler(int, SEVTHANDLER);
BOOL SNetRegisterEventHandler(int, SEVTHANDLER);
bool SNetSetBasePlayer(int);
int SNetInitializeProvider(unsigned long, struct _SNETPROGRAMDATA *, struct _SNETPLAYERDATA *, struct _SNETUIDATA *, struct _SNETVERSIONDATA *);
int SNetGetProviderCaps(struct _SNETCAPS *);
#ifdef ZEROTIER
void SNetSendInfoRequest();
std::vector<std::string> SNetGetGamelist();
void SNetSetPassword(std::string pw);
#endif

void  InitializeMpqCryptography();
void  EncryptMpqBlock(void * pvDataBlock, DWORD dwLength, DWORD dwKey);
void  DecryptMpqBlock(void * pvDataBlock, DWORD dwLength, DWORD dwKey);
DWORD HashStringSlash(const char * szFileName, DWORD dwHashType);
bool SFileEnableDirectAccess(bool enable);
void SLoadKeyMap(BYTE (&map)[256]);

#if defined(__GNUC__) || defined(__cplusplus)
}
#endif

} // namespace dvl
