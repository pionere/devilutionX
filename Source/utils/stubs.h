#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>

#define UNIMPLEMENTED()                                                         \
	do {                                                                        \
		SDL_Log("UNIMPLEMENTED: %s @ %s:%d", __FUNCTION__, __FILE__, __LINE__); \
		abort();                                                                \
	} while (0)

#define ABORT()                                                         \
	do {                                                                \
		SDL_Log("ABORT: %s @ %s:%d", __FUNCTION__, __FILE__, __LINE__); \
		abort();                                                        \
	} while (0)

#define ASSERT(x)                                                         \
	if (!(x)) {                                                           \
		SDL_Log("Assertion failed in %s:%d: %s", __FILE__, __LINE__, #x); \
		abort();                                                          \
	}
