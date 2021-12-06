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
/*#define GAMESTATE_PRIVATE 0x01
#define GAMESTATE_FULL    0x02
#define GAMESTATE_ACTIVE  0x04
#define GAMESTATE_STARTED 0x08
#define GAMESTATE_REPLAY  0x80*/

#if defined(__GNUC__) || defined(__cplusplus)
extern "C" {
#endif

#define MPQ_OPEN_READ_ONLY			0x00000100
#define MPQ_FILE_IMPLODE			0x00000100
#define MPQ_FILE_EXISTS				0x80000000
#define MPQ_HASH_TABLE_INDEX		0x000
#define MPQ_HASH_NAME_A				0x100
#define MPQ_HASH_NAME_B				0x200
#define MPQ_HASH_FILE_KEY			0x300
#define HASH_ENTRY_DELETED          0xFFFFFFFE  // Block index for deleted entry in the hash table
#define HASH_ENTRY_FREE             0xFFFFFFFF  // Block index for free entry in the hash table
#define SFILE_OPEN_FROM_MPQ			0
#define SFILE_OPEN_LOCAL_FILE		0xFFFFFFFF
#define SFILE_OPEN_CHECK_EXISTS		0xFFFFFFFC

void WINAPI SFileCloseArchive(HANDLE hArchive);
void WINAPI SFileCloseFile(HANDLE hFile);

DWORD WINAPI SFileGetFileSize(HANDLE hFile);
DWORD WINAPI SFileGetFilePointer(HANDLE hFile);
DWORD WINAPI SFileSetFilePointer(HANDLE hFile, long lFilePos, unsigned dwMoveMethod);
HANDLE WINAPI SFileOpenArchive(const char* szMpqName, DWORD dwFlags);

HANDLE SFileOpenFile(const char* filename);
bool WINAPI SFileOpenFileEx(HANDLE hMpq, const char *szFileName, DWORD dwSearchScope, HANDLE *phFile);

bool WINAPI SFileReadFile(HANDLE hFile, void *buffer, DWORD nNumberOfBytesToRead);

bool getIniBool(const char *sectionName, const char *keyName, bool defaultValue);
bool getIniValue(const char *sectionName, const char *keyName, char *string, int stringSize);
void setIniValue(const char *sectionName, const char *keyName, const char *value);
bool getIniInt(const char *sectionName, const char *keyName, int *value);
void setIniInt(const char *sectionName, const char *keyName, int value);

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
//#define STORM_ERROR_NO_MESSAGES_WAITING          0x8510006b
#define STORM_ERROR_NOT_IN_GAME                  0x85100070

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
