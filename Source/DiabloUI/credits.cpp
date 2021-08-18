#include <memory>
#include <vector>

#include "controls/menu_controls.h"
#include "all.h"
#include "utils/display.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/credits_lines.h"
#include "DiabloUI/art.h"
#include "DiabloUI/art_draw.h"
#include "DiabloUI/fonts.h"

DEVILUTION_BEGIN_NAMESPACE

namespace {

const SDL_Rect VIEWPORT = { 0, 114, 640, 251 };
const int ShadowOffsetX = 2;
const int ShadowOffsetY = 2;
const int LINE_H = 22;

// The maximum number of visible lines is the number of whole lines
// (VIEWPORT.h / LINE_H) rounded up, plus one extra line for when
// a line is leaving the screen while another one is entering.
#define MAX_VISIBLE_LINES ((VIEWPORT.h - 1) / LINE_H + 2)

struct CachedLine {

	CachedLine()
	{
		m_index = 0;
		m_surface = NULL;
		palette_version = pal_surface_palette_version;
	}

	CachedLine(unsigned index, SDL_Surface *surface)
	{
		m_index = index;
		m_surface = surface;
		palette_version = pal_surface_palette_version;
	}

	unsigned m_index;
	SDL_Surface *m_surface;
	unsigned int palette_version;
};

SDL_Surface *RenderText(const char *text, SDL_Color color)
{
	if (text[0] == '\0')
		return NULL;
	SDL_Surface *result = TTF_RenderUTF8_Solid(font, text, color);
	if (result == NULL)
		SDL_Log("%s", TTF_GetError());
	return result;
}

CachedLine PrepareLine(unsigned index)
{
	const char *contents = CREDITS_LINES[index];
	while (contents[0] == '\t')
		++contents;

	const SDL_Color shadowColor = { 0, 0, 0, 0 };
	SDL_Surface *text = RenderText(contents, shadowColor);

	// Precompose shadow and text:
	SDL_Surface *surface = NULL;
	if (text != NULL) {
		// Set up the target surface to have 3 colors: mask, text, and shadow.
		surface = SDL_CreateRGBSurfaceWithFormat(0, text->w + ShadowOffsetX, text->h + ShadowOffsetY, 8, SDL_PIXELFORMAT_INDEX8);
		const SDL_Color maskColor = { 0, 255, 0, 0 }; // Any color different from both shadow and text
		const SDL_Color &textColor = palette->colors[224];
		SDL_Color colors[3] = { maskColor, textColor, shadowColor };
		if (SDLC_SetSurfaceColors(surface, colors, 0, 3) <= -1)
			SDL_Log("%s", SDL_GetError());
		SDLC_SetColorKey(surface, 0);

		// Blit the shadow first:
		SDL_Rect shadowRect = { ShadowOffsetX, ShadowOffsetY, 0, 0 };
		if (SDL_BlitSurface(text, NULL, surface, &shadowRect) <= -1)
			ErrSdl();

		// Change the text surface color and blit again:
		SDL_Color textColors[2] = { maskColor, textColor };
		if (SDLC_SetSurfaceColors(text, textColors, 0, 2) <= -1)
			ErrSdl();
		SDLC_SetColorKey(text, 0);

		if (SDL_BlitSurface(text, NULL, surface, NULL) <= -1)
			ErrSdl();

		SDL_Surface *surface_ptr = surface;
		ScaleSurfaceToOutput(&surface_ptr);
		surface = surface_ptr;
	}
	SDL_FreeSurface(text);
	return CachedLine(index, surface);
}

class CreditsRenderer {

public:
	CreditsRenderer()
	{
#ifndef NOWIDESCREEN
		LoadArt("ui_art\\creditsw.pcx", &ArtBackgroundWidescreen);
#endif
		LoadBackgroundArt("ui_art\\credits.pcx");
		LoadTtfFont();
		ticks_begin_ = SDL_GetTicks();
		prev_offset_y_ = 0;
		finished_ = false;
	}

	~CreditsRenderer()
	{
#ifndef NOWIDESCREEN
		ArtBackgroundWidescreen.Unload();
#endif
		ArtBackground.Unload();
		UnloadTtfFont();

		for (unsigned x = 0; x < lines_.size(); x++) {
			SDL_FreeSurface(lines_[x].m_surface);
		}
	}

	void Render();

	bool Finished() const
	{
		return finished_;
	}

private:
	std::vector<CachedLine> lines_;
	bool finished_;
	Uint32 ticks_begin_;
	int prev_offset_y_;
};

void CreditsRenderer::Render()
{
	const int offsetY = -VIEWPORT.h + (SDL_GetTicks() - ticks_begin_) / 40;
	if (offsetY == prev_offset_y_)
		return;
	prev_offset_y_ = offsetY;

	SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
#ifndef NOWIDESCREEN
	DrawArt(PANEL_LEFT - 320, UI_OFFSET_Y, &ArtBackgroundWidescreen);
#endif
	DrawArt(PANEL_LEFT, UI_OFFSET_Y, &ArtBackground);
	if (font == NULL)
		return;

	const unsigned linesBegin = std::max(offsetY / LINE_H, 0);
	const unsigned linesEnd = std::min(linesBegin + MAX_VISIBLE_LINES, CREDITS_LINES_SIZE);

	if (linesBegin >= linesEnd) {
		if (linesEnd == CREDITS_LINES_SIZE)
			finished_ = true;
		return;
	}

	while (linesEnd > lines_.size())
		lines_.push_back(PrepareLine(lines_.size()));

	SDL_Rect viewport = VIEWPORT;
	viewport.x += PANEL_LEFT;
	viewport.y += UI_OFFSET_Y;
	ScaleOutputRect(&viewport);
	SDL_SetClipRect(DiabloUiSurface(), &viewport);

	// We use unscaled coordinates for calculation throughout.
	int destY = UI_OFFSET_Y + VIEWPORT.y - (offsetY - linesBegin * LINE_H);
	for (unsigned i = linesBegin; i < linesEnd; ++i, destY += LINE_H) {
		CachedLine &line = lines_[i];
		if (line.m_surface == NULL)
			continue;

		// Still fading in: the cached line was drawn with a different fade level.
		if (line.palette_version != pal_surface_palette_version) {
			SDL_FreeSurface(line.m_surface);
			line = PrepareLine(line.m_index);
		}

		int destX = PANEL_LEFT + VIEWPORT.x + 31;
		int j = 0;
		while (CREDITS_LINES[line.m_index][j++] == '\t')
			destX += 40;

		SDL_Rect dstRect = { destX, destY, 0, 0 };
		ScaleOutputRect(&dstRect);
		dstRect.w = line.m_surface->w;
		dstRect.h = line.m_surface->h;
		if (SDL_BlitSurface(line.m_surface, NULL, DiabloUiSurface(), &dstRect) < 0)
			ErrSdl();
	}
	SDL_SetClipRect(DiabloUiSurface(), NULL);
}

} // namespace

void UiCreditsDialog()
{
	CreditsRenderer creditsRenderer;
	bool endMenu = false;

	SDL_Event event;
	do {
		creditsRenderer.Render();
		UiFadeIn();
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_MOUSEBUTTONDOWN:
				endMenu = true;
				break;
			default:
				switch (GetMenuAction(event)) {
				case MenuAction_BACK:
				case MenuAction_SELECT:
					endMenu = true;
					break;
				default:
					break;
				}
			}
			UiHandleEvents(&event);
		}
	} while (!endMenu && !creditsRenderer.Finished());
}

DEVILUTION_END_NAMESPACE
