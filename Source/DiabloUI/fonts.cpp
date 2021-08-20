#include "fonts.h"

#include "utils/file_util.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

TTF_Font *font = NULL;
BYTE *FontTables[4];
Art ArtFonts[4][2];
/** This is so we know ttf has been init when we get to the diablo_deinit() function */
bool gbWasFontsInit = false;

static void LoadArtFont(const char *pszFile, int size, int color)
{
	LoadMaskedArt(pszFile, &ArtFonts[size][color], 256, 32);
}

void LoadArtFonts()
{
	FontTables[AFT_SMALL] = LoadFileInMem("ui_art\\font16.bin");
	FontTables[AFT_MED] = LoadFileInMem("ui_art\\font24.bin");
	FontTables[AFT_BIG] = LoadFileInMem("ui_art\\font30.bin");
	FontTables[AFT_HUGE] = LoadFileInMem("ui_art\\font42.bin");
	LoadArtFont("ui_art\\font16s.pcx", AFT_SMALL, AFC_SILVER);
	LoadArtFont("ui_art\\font16g.pcx", AFT_SMALL, AFC_GOLD);
	LoadArtFont("ui_art\\font24s.pcx", AFT_MED, AFC_SILVER);
	LoadArtFont("ui_art\\font24g.pcx", AFT_MED, AFC_GOLD);
	LoadArtFont("ui_art\\font30s.pcx", AFT_BIG, AFC_SILVER);
	LoadArtFont("ui_art\\font30g.pcx", AFT_BIG, AFC_GOLD);
	LoadArtFont("ui_art\\font42g.pcx", AFT_HUGE, AFC_GOLD);
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
	mem_free_dbg(FontTables[AFT_SMALL]);
	FontTables[AFT_SMALL] = NULL;
	mem_free_dbg(FontTables[AFT_MED]);
	FontTables[AFT_MED] = NULL;
	mem_free_dbg(FontTables[AFT_BIG]);
	FontTables[AFT_BIG] = NULL;
	mem_free_dbg(FontTables[AFT_HUGE]);
	FontTables[AFT_HUGE] = NULL;
}

void LoadTtfFont()
{
	if (TTF_WasInit() == 0) {
		if (TTF_Init() == -1) {
			SDL_Log("TTF_Init: %s", TTF_GetError());
			diablo_quit(1);
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
		SDL_Log("TTF_OpenFont: %s", TTF_GetError());
		return;
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
