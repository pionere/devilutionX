/**
 * @file restrict.cpp
 *
 * Implementation of functionality for checking if the game will be able run on the system.
 */
#include "all.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

void ReadOnlyTest()
{
	const std::string path = GetPrefPath() + "Diablo1ReadOnlyTest.foo";
	FILE *f = FileOpen(path.c_str(), "wt");
	if (f == NULL) {
		DirErrorDlg(GetPrefPath().c_str());
	}

	fclose(f);
	remove(path.c_str());
}

DEVILUTION_END_NAMESPACE
