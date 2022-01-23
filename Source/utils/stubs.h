#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "log.h"

#define UNIMPLEMENTED()                                                         \
	do {                                                                        \
		DoLog("UNIMPLEMENTED: %s @ %s:%d", __FUNCTION__, __FILE__, __LINE__); \
		abort();                                                                \
	} while (0)

#define ABORT()                                                         \
	do {                                                                \
		DoLog("ABORT: %s @ %s:%d", __FUNCTION__, __FILE__, __LINE__); \
		abort();                                                        \
	} while (0)

#define ASSERT(x)                                                         \
	if (!(x)) {                                                           \
		DoLog("Assertion failed in %s:%d: %s", __FILE__, __LINE__, #x); \
		abort();                                                          \
	}
