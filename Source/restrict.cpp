/**
 * @file restrict.cpp
 *
 * Implementation of functionality for checking if the game will be able run on the system.
 */
#include "all.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Check that we have write access to the game install folder

 */
void ReadOnlyTest()
{
	std::string path = GetPrefPath();
	path += "Diablo1ReadOnlyTest.foo";
	FILE *f = FileOpen(path.c_str(), "wt");
	if (f != NULL) {
		fclose(f);
		remove(path.c_str());
	} else {
		DirErrorDlg(GetPrefPath());
	}
}

DEVILUTION_END_NAMESPACE
