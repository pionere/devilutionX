#pragma once

#include <algorithm>
#include <string>
#include <cstdint>

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

#include "log.h"

#if defined(_WIN64) || defined(_WIN32)
// Suppress definitions of `min` and `max` macros by <windows.h>:
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if _POSIX_C_SOURCE >= 200112L || defined(_BSD_SOURCE) || defined(__APPLE__)
#include <unistd.h>
#include <sys/stat.h>
#else
#include <cstdio>
#endif

#include "../defs.h"

DEVILUTION_BEGIN_NAMESPACE

inline FILE* FileOpen(const char* path, const char* mode)
{
	FILE* file;
#if (defined(_MSC_VER) && (_MSC_VER >= 1400))
	fopen_s(&file, path, mode);
#else
	file = std::fopen(path, mode);
#endif
	return file;
}

inline bool FileExists(const char* path)
{
#if (_POSIX_C_SOURCE >= 200112L || defined(_BSD_SOURCE) || defined(__APPLE__)) && !defined(__ANDROID__)
	return ::access(path, F_OK) == 0;
#else
	FILE* file = FileOpen(path, "rb");
	if (file == NULL)
		return false;
	std::fclose(file);
	return true;
#endif
}

inline bool GetFileSize(const char* path, std::uintmax_t* size)
{
#if defined(_WIN64) || defined(_WIN32)
	WIN32_FILE_ATTRIBUTE_DATA attr;
	/*int path_utf16_size = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
	wchar_t *path_utf16 = new wchar_t[path_utf16_size];
	if (MultiByteToWideChar(CP_UTF8, 0, path, -1, path_utf16, path_utf16_size) != path_utf16_size) {
		delete[] path_utf16;
		return false;
	}
	if (!GetFileAttributesExW(path_utf16, GetFileExInfoStandard, &attr)) {
		delete[] path_utf16;
		return false;
	}
	delete[] path_utf16;*/
	if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attr)) {
		return false;
	}
	*size = (std::uintmax_t)(attr.nFileSizeHigh) << (sizeof(attr.nFileSizeLow) * 8) | attr.nFileSizeLow;
	return true;
#else
	struct ::stat statResult;
	if (::stat(path, &statResult) == -1)
		return false;
	*size = static_cast<uintmax_t>(statResult.st_size);
	return true;
#endif
}

inline bool ReadFile(void* out, size_t size, FILE* f)
{
	return fread(out, size, 1, f) == 1;
}

inline bool WriteFile(const void* data, size_t size, FILE* f)
{
	return fwrite(data, size, 1, f) == 1;
}

inline bool ResizeFile(const char* path, std::uintmax_t size)
{
#if defined(_WIN64) || defined(_WIN32)
	LARGE_INTEGER lisize;
	lisize.QuadPart = static_cast<LONGLONG>(size);
	if (lisize.QuadPart < 0) {
		return false;
	}
	/*int path_utf16_size = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
	wchar_t *path_utf16 = new wchar_t[path_utf16_size];
	if (MultiByteToWideChar(CP_UTF8, 0, path, -1, path_utf16, path_utf16_size) != path_utf16_size) {
		delete[] path_utf16;
		return false;
	}
	HANDLE file = ::CreateFileW(path_utf16, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	delete[] path_utf16;*/
	HANDLE file = ::CreateFileA(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		return false;
	} else if (::SetFilePointerEx(file, lisize, NULL, FILE_BEGIN) == 0 || ::SetEndOfFile(file) == 0) {
		::CloseHandle(file);
		return false;
	}
	::CloseHandle(file);
	return true;
#else
	return ::truncate(path, static_cast<off_t>(size)) == 0;
#endif
}

inline void RemoveFile(const char* lpFileName)
{
	/*std::string name = lpFileName;
	std::replace(name.begin(), name.end(), '\\', '/');
	if (remove(name.c_str()) == 0) {
		DoLog("Removed file: %s", name.c_str());
	} else {
		DoLog("Failed to remove file: %s", name.c_str());
	}*/
	if (remove(lpFileName) == 0) {
#if DEBUG_MODE
		DoLog("Removed file: %s", lpFileName);
#endif
	} else {
		DoLog("Failed to remove file: %s", lpFileName);
	}
}

DEVILUTION_END_NAMESPACE
