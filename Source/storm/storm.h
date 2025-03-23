#pragma once

#include <cerrno>
#include <limits>
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

// Note to self: Linker error => forgot a return value in cpp

// We declare the StormLib methods we use here.
// StormLib uses the Windows calling convention on Windows for these methods.
#ifdef _WIN32
#define WINAPI __stdcall
#else
#define WINAPI
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

#define MPQ_OPEN_READ_ONLY       0x00000001

HANDLE WINAPI SFileOpenArchive(const char* szMpqName, DWORD dwFlags);
bool   WINAPI SFileReopenArchive(HANDLE hMpq, const char* szMpqName);
HANDLE WINAPI SFileCreateArchive(const char* szMpqName, DWORD dwHashCount, DWORD dwBlockCount);
DWORD  WINAPI SFileReadArchive(HANDLE hMpq, const char* szFileName, BYTE** dest);
DWORD  WINAPI SFileReadLocalFile(const char* szFileName, BYTE** dest);
bool   WINAPI SFileWriteFile(HANDLE hMpq, const char* szFileName, void* pvData, DWORD dwSize);
bool   WINAPI SFileRemoveFile(HANDLE hMpq, const char* szFileName);
void   WINAPI SFileRenameFile(HANDLE hMpq, const char* szOldFileName, const char* szNewFileName);
void   WINAPI SFileFlushArchive(HANDLE hMpq);
void   WINAPI SFileFlushAndCloseArchive(HANDLE hMpq);
void   WINAPI SFileReleaseArchive(HANDLE hMpq);
void   WINAPI SFileCloseArchive(HANDLE hMpq);

DWORD SFileReadFileEx(const char* szFileName, BYTE** dest);

// These error codes are used and returned by StormLib.
// See StormLib/src/StormPort.h
//#if defined(_WIN32)
//// https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
//#define STORM_ERROR_FILE_NOT_FOUND 2
//#define STORM_ERROR_HANDLE_EOF     38
//#else // !defined(_WIN32)
//#define STORM_ERROR_FILE_NOT_FOUND ENOENT
//#define STORM_ERROR_HANDLE_EOF     1002
//#endif

/*  SErrGetLastError @ 463
 *
 *  Retrieves the last error that was specifically
 *  set for the Storm library.
 *
 *  Returns the last error set within the Storm library.
 */
//DWORD SErrGetLastError();

/*  SErrSetLastError @ 465
 *
 *  Sets the last error for the Storm library and the Kernel32 library.
 *
 *  dwErrCode:  The error code that will be set.
 */
//void SErrSetLastError(DWORD dwErrCode);

// Values for dwErrCode
//#define STORM_ERROR_GAME_TERMINATED 0x85100069
//#define STORM_ERROR_INVALID_PLAYER  0x8510006a
//#define STORM_ERROR_NO_MESSAGES_WAITING 0x8510006b
//#define STORM_ERROR_NOT_IN_GAME     0x85100070

/*  SStrCopy @ 501
 *
 *  Copies a string from src to dest (including NULL terminator)
 *  until the max_length is reached.
 *
 *  dest:         The destination array.
 *  src:          The source array.
 *  max_length:   The maximum length of dest.
 *
 *  Returns the length of the string (without the trailing nul)
 */
int SStrCopy(char* dest, const char* src, int max_length);

void InitializeMpqCryptography();
void EncryptMpqBlock(void* pvDataBlock, DWORD dwLength, DWORD dwKey);
void DecryptMpqBlock(void* pvDataBlock, DWORD dwLength, DWORD dwKey);
DWORD HashStringSlash(const char* szFileName, unsigned dwHashType);
void SFileEnableDirectAccess(bool enable);

#if defined(__GNUC__) || defined(__cplusplus)
}
#endif

DEVILUTION_END_NAMESPACE
