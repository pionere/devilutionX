#include "storm.h"

#include <cstddef>
#include <string>

#include "storm_cfg.h"
#include "utils/paths.h"

/*// Include Windows headers for Get/SetLastError.
#if defined(_WIN32)
// Suppress definitions of `min` and `max` macros by <windows.h>:
#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else // !defined(_WIN32)
// On non-Windows, these are defined in 3rdParty/StormLib.
//extern "C" void SetLastError(std::uint32_t dwErrCode);
//extern "C" std::uint32_t GetLastError();
#endif*/

DEVILUTION_BEGIN_NAMESPACE

static bool directFileAccess = false;

DWORD SFileReadFileEx(const char* filename, BYTE** dest)
{
	unsigned i;
	DWORD result = 0;

	if (directFileAccess) {
		const std::string* basePath = GetBasePathStr();
#ifdef _WIN32
		std::string path = *basePath + filename;
#else
		std::string path = filename;
		for (i = 0; i < path.size(); ++i)
			if (path[i] == '\\')
				path[i] = '/';
		path = *basePath + path;
#endif
		result = SFileReadLocalFile(path.c_str(), dest);
	}
#if USE_MPQONE
	if (result == 0 && diabdat_mpq != NULL)
		SFileReadArchive(diabdat_mpq, filename, dest);
#else
	for (i = 0; i < (unsigned)lengthof(diabdat_mpqs) && result == 0; i++) {
		if (diabdat_mpqs[i] == NULL)
			continue;
		result = SFileReadArchive(diabdat_mpqs[i], filename, dest);
	}
#endif
	if (result == 0) {
		DoLog("File '%s' not found.", filename);
	}
	return result;
}

//DWORD SErrGetLastError()
//{
//	return ::GetLastError();
//}

//void SErrSetLastError(DWORD dwErrCode)
//{
//	::SetLastError(dwErrCode);
//}

int SStrCopy(char* dest, const char* src, int max_length)
{
	int result = snprintf(dest, max_length, "%s", src);
	if (result >= max_length)
		result = max_length - 1;
	return result;
}

void SFileEnableDirectAccess(bool enable)
{
	directFileAccess = enable;
}

DEVILUTION_END_NAMESPACE
