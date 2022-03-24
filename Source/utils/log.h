#pragma once

#if DEBUG_MODE
#define DoLog            SDL_Log
#else
#define DoLog(x, ...)   do { } while(0);
#endif
