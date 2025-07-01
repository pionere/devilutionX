#pragma once

#include <string>
#include <fstream>
#include "../../defs.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/*std::istream&*/bool safeGetline(std::istream& is, std::string& t);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
