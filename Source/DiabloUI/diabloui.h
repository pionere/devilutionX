#pragma once

#include <cstddef>
#include <SDL.h>

#include "DiabloUI/art.h"
#include "DiabloUI/ui_item.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

extern unsigned SelectedItem;
extern unsigned ListOffset;
extern bool textInputActive;

typedef enum _artFocus {
	FOCUS_SMALL,
	FOCUS_MED,
	FOCUS_BIG,
} _artFocus;

typedef enum _artLogo {
	LOGO_SMALL,
	LOGO_MED,
	LOGO_BIG,
} _artLogo;

extern Art ArtLogos[3];
extern Art ArtFocus[3];
extern Art ArtBackground;
#ifdef WIDESCREEN
extern Art ArtBackgroundWidescreen;
#endif
extern Art ArtCursor;
extern Art ArtHero;

extern void (*gfnSoundFunction)(const char *file);

inline SDL_Surface *DiabloUiSurface()
{
	auto *output_surface = GetOutputSurface();

#ifdef USE_SDL1
	// When using a non double-buffered hardware surface, render the UI
	// to an off-screen surface first to avoid flickering / tearing.
	if ((output_surface->flags & SDL_HWSURFACE) != 0
	    && (output_surface->flags & SDL_DOUBLEBUF) == 0) {
		return pal_surface;
	}
#endif

	return output_surface;
}

void UiFadeIn();
void UiHandleEvents(SDL_Event *event);
bool UiItemMouseEvents(SDL_Event *event, const std::vector<UiItemBase *> &uiItems);
int GetCenterOffset(int w, int bw = 0);
void DrawMouse();
void LoadBackgroundArt(const char *pszFile, int frames = 1);
void UiAddBackground(std::vector<UiItemBase *> *vecDialog);
void UiAddLogo(std::vector<UiItemBase *> *vecDialog, int size = LOGO_MED, int y = 0);
void UiFocusNavigationSelect();
void UiFocusNavigationEsc();
void UiFocusNavigationYesNo();
void UiInitList(const std::vector<UiItemBase *> &uiItems, unsigned listSize, void (*fnFocus)(unsigned index) = NULL, void (*fnSelect)(unsigned index) = NULL, void (*fnEsc)() = NULL, bool (*fnYesNo)() = NULL, bool wraps = false);
void UiInitScrollBar(UiScrollBar *ui_sb, unsigned viewport_size);
void UiClearScreen();
void UiPollAndRender();
void UiRenderItems(const std::vector<UiItemBase *> &uiItems);
void UiInitList_clear();

void mainmenu_restart_repintro();
bool UiValidPlayerName(const char *name);

DEVILUTION_END_NAMESPACE
