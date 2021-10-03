#include "fonts.h"

#include "utils/file_util.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE* FontTables[4];
Art ArtFonts[4][2];

void LoadArtFonts()
{
	FontTables[AFT_SMALL] = LoadFileInMem("ui_art\\font16.bin");
	FontTables[AFT_MED] = LoadFileInMem("ui_art\\font24.bin");
	FontTables[AFT_BIG] = LoadFileInMem("ui_art\\font30.bin");
	FontTables[AFT_HUGE] = LoadFileInMem("ui_art\\font42.bin");
	LoadMaskedArt("ui_art\\font16s.pcx", &ArtFonts[AFT_SMALL][AFC_SILVER], 256, 32);
	LoadMaskedArt("ui_art\\font16g.pcx", &ArtFonts[AFT_SMALL][AFC_GOLD], 256, 32);
	LoadMaskedArt("ui_art\\font24s.pcx", &ArtFonts[AFT_MED][AFC_SILVER], 256, 32);
	LoadMaskedArt("ui_art\\font24g.pcx", &ArtFonts[AFT_MED][AFC_GOLD], 256, 32);
	LoadMaskedArt("ui_art\\font30s.pcx", &ArtFonts[AFT_BIG][AFC_SILVER], 256, 32);
	LoadMaskedArt("ui_art\\font30g.pcx", &ArtFonts[AFT_BIG][AFC_GOLD], 256, 32);
	LoadMaskedArt("ui_art\\font42g.pcx", &ArtFonts[AFT_HUGE][AFC_GOLD], 256, 32);
}

void UnloadArtFonts()
{
	ArtFonts[AFT_SMALL][AFC_SILVER].Unload();
	ArtFonts[AFT_SMALL][AFC_GOLD].Unload();
	ArtFonts[AFT_MED][AFC_SILVER].Unload();
	ArtFonts[AFT_MED][AFC_GOLD].Unload();
	ArtFonts[AFT_BIG][AFC_SILVER].Unload();
	ArtFonts[AFT_BIG][AFC_GOLD].Unload();
	ArtFonts[AFT_HUGE][AFC_GOLD].Unload();
	MemFreeDbg(FontTables[AFT_SMALL]);
	MemFreeDbg(FontTables[AFT_MED]);
	MemFreeDbg(FontTables[AFT_BIG]);
	MemFreeDbg(FontTables[AFT_HUGE]);
}

DEVILUTION_END_NAMESPACE
