#pragma once

#include "DiabloUI/art.h"

DEVILUTION_BEGIN_NAMESPACE

extern BYTE* FontTables[4];
extern Art ArtFonts[4][2];

void LoadArtFonts();
void UnloadArtFonts();

DEVILUTION_END_NAMESPACE
