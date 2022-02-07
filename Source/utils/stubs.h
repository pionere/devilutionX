#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "log.h"

#define ABORT()                                                         \
	do {                                                                \
		DoLog("ABORT: %s @ %s:%d", __FUNCTION__, __FILE__, __LINE__); \
		abort();                                                        \
	} while (0)
