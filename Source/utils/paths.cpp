#include "paths.h"

#include <SDL.h>
#include "utils/sdl2_to_1_2_backports.h"

#ifdef __IPHONEOS__
#include "platform/ios/ios_paths.h"
#endif

#ifdef NXDK
#include "platform/nxdk/nxdk_paths.h"
#endif

#include "log.h"

DEVILUTION_BEGIN_NAMESPACE

static std::string basePath;
static std::string prefPath;

static void FromSDL(std::string& path, bool base)
{
	char* s;

	if (!path.empty()) {
		return;
	}
	// the path was not set explicitly -> get it from SDL
#ifdef __IPHONEOS__
	s = base ? SDL_GetBasePath() : IOSGetPrefPath();
#elif defined(NXDK)
	s = base ? SDL_GetBasePath() : NxdkGetPrefPath();
#else
	s = base ? SDL_GetBasePath() : SDL_GetPrefPath("diasurgical", "devilx");
#endif
	if (s != NULL) {
		path = s;
		SDL_free(s);
	} else {
		DoLog(SDL_GetError());
	}
}

void InitPaths()
{
	FromSDL(basePath, true);
	FromSDL(prefPath, false);
}

const char* GetBasePath()
{
	return basePath.c_str();
}

const char* GetPrefPath()
{
	return prefPath.c_str();
}

const std::string* GetBasePathStr()
{
	return &basePath;
}

void SetBasePath(const char* path)
{
	basePath = path;
}

void SetPrefPath(const char* path)
{
	prefPath = path;
}

DEVILUTION_END_NAMESPACE
