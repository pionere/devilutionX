#include "paths.h"

#include <SDL.h>

#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

static std::string *basePath = NULL;
static std::string *prefPath = NULL;
static std::string *configPath = NULL;

static void AddTrailingSlash(std::string &path)
{
#ifdef _WIN32
	if (!path.empty() && path.back() != '\\')
		path += '\\';
#else
	if (!path.empty() && path.back() != '/')
		path += '/';
#endif
}

static std::string *FromSDL(char *s)
{
	std::string *result = new std::string(s != NULL ? s : "");
	if (s != NULL) {
		SDL_free(s);
	} else {
		SDL_Log("%s", SDL_GetError());
		SDL_ClearError();
	}
	return result;
}

const char* GetBasePath()
{
	if (basePath == NULL)
		basePath = FromSDL(SDL_GetBasePath());
	return basePath->c_str();
}

const char* GetPrefPath()
{
	if (prefPath == NULL)
		prefPath = FromSDL(SDL_GetPrefPath("diasurgical", "devilution"));
	return prefPath->c_str();
}

const char *GetConfigPath()
{
	if (configPath == NULL)
		configPath = FromSDL(SDL_GetPrefPath("diasurgical", "devilution"));
	return configPath->c_str();
}

void SetBasePath(const char *path)
{
	if (basePath == NULL)
		basePath = new std::string;
	*basePath = path;
	AddTrailingSlash(*basePath);
}

void SetPrefPath(const char *path)
{
	if (prefPath == NULL)
		prefPath = new std::string;
	*prefPath = path;
	AddTrailingSlash(*prefPath);
}

void SetConfigPath(const char *path)
{
	if (configPath == NULL)
		configPath = new std::string;
	*configPath = path;
	AddTrailingSlash(*configPath);
}

DEVILUTION_END_NAMESPACE
