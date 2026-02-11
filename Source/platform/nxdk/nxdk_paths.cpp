#include "nxdk_paths.h"

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

char *NxdkGetPrefPath()
{
	const char *path = "E:\\UDATA\\devilx\\";
	if (CreateDirectoryA(path, nullptr) == FALSE && ::GetLastError() != ERROR_ALREADY_EXISTS) {
		return NULL;
	}
	return strdup(path);
}

#ifdef __cplusplus
}
#endif
