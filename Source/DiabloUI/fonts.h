#pragma once

#include "DiabloUI/art.h"

DEVILUTION_BEGIN_NAMESPACE

enum _artFontTables {
	AFT_SMALL,
	AFT_MED,
	AFT_BIG,
	AFT_HUGE,
};

enum _artFontColors {
	AFC_SILVER,
	AFC_GOLD,
};

extern BYTE* FontTables[4];
extern Art ArtFonts[4][2];

void LoadArtFonts();
void UnloadArtFonts();

DEVILUTION_END_NAMESPACE
