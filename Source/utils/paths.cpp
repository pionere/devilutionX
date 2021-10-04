#include "paths.h"

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

static std::string basePath;
static std::string prefPath;
static std::string configPath;

#ifdef _WIN32
#define SLASH '\\'
#else
#define SLASH '/'
#endif

static void AddTrailingSlash(std::string &path)
{
	if (path.empty())
		path = " "; // make sure the path is not set during InitPaths
	else if (path.back() != SLASH)
		path += SLASH;
}

static void FromSDL(std::string &path, bool base)
{
	char* s;

	if (!path.empty()) {
		if (path.back() != SLASH)
			path = ""; // reset path if it was set to empty
		return;
	}
	// the path was not set explicitly -> get it from SDL
	s = base ? SDL_GetBasePath() : SDL_GetPrefPath("diasurgical", "devilx");
	if (s != NULL) {
		path = s;
		SDL_free(s);
	} else {
		SDL_Log("%s", SDL_GetError());
		SDL_ClearError();
	}
}

void InitPaths()
{
	FromSDL(basePath, true);
	FromSDL(prefPath, false);
	FromSDL(configPath, false);
}

const char* GetBasePath()
{
	return basePath.c_str();
}

const char* GetPrefPath()
{
	return prefPath.c_str();
}

const char* GetConfigPath()
{
	return configPath.c_str();
}

const std::string* GetBasePathStr()
{
	return &basePath;
}

void SetBasePath(const char* path)
{
	basePath = path;
	AddTrailingSlash(basePath);
}

void SetPrefPath(const char* path)
{
	prefPath = path;
	AddTrailingSlash(prefPath);
}

void SetConfigPath(const char* path)
{
	configPath = path;
	AddTrailingSlash(configPath);
}

DEVILUTION_END_NAMESPACE
