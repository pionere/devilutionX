#include "fonts.h"

#include "utils/file_util.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

TTF_Font* font = NULL;
BYTE* FontTables[4];
Art ArtFonts[4][2];
/** This is so we know ttf has been init when we get to the diablo_deinit() function */
bool gbWasFontsInit = false;

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

void LoadTtfFont()
{
	if (TTF_WasInit() == 0) {
		if (TTF_Init() == -1) {
			ttf_fatal(ERR_SDL_TTF_INIT);
		}
		gbWasFontsInit = true;
	}

	const char* ttfFontPath = TTF_FONT_NAME;
	if (!FileExists(ttfFontPath))
	{
		ttfFontPath = TTF_FONT_DIR TTF_FONT_NAME;
	}
#if defined(__linux__) && !defined(__ANDROID__)
	if (!FileExists(ttfFontPath)) {
		ttfFontPath = "/usr/share/fonts/truetype/" TTF_FONT_NAME;
	}
#endif
	font = TTF_OpenFont(ttfFontPath, 17);
	if (font == NULL) {
		ttf_fatal(ERR_SDL_TTF_FONT);
	}

	TTF_SetFontKerning(font, 0);
	TTF_SetFontHinting(font, TTF_HINTING_MONO);
}

void UnloadTtfFont()
{
	if (font != NULL /*&& TTF_WasInit() != 0*/)
		TTF_CloseFont(font);
	font = NULL;
}

void FontsCleanup()
{
	TTF_Quit();
}

DEVILUTION_END_NAMESPACE
