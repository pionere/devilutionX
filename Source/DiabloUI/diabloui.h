#pragma once

#include <cstddef>
#include <vector>
#include <SDL.h>

#include "DiabloUI/art.h"
#include "DiabloUI/ui_item.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

#define SML_BUTTON_WIDTH	110
#define SML_BUTTON_HEIGHT	28

typedef enum _artFocus {
	FOCUS_SMALL,
	FOCUS_MED,
	FOCUS_BIG,
	NUM_FOCUS
} _artFocus;

extern Art ArtFocus[NUM_FOCUS];
extern Art ArtBackground;
extern Art ArtCursor;
extern Art ArtHero;
extern Art ArtSmlButton;

extern void (*gfnSoundFunction)(int gfx, int rndCnt);
extern std::vector<UiListItem*> gUIListItems;
extern std::vector<UiItemBase*> gUiItems;
extern unsigned SelectedItem;
extern unsigned ListOffset;
extern UiEdit* gUiEditField;

inline SDL_Surface* DiabloUiSurface()
{
// commented out, because:
//		1. no need to optimize drawing in the menu
//		2. makes DrawArt unusable in-game
//
//	auto *output_surface = GetOutputSurface();

//#ifdef USE_SDL1
//	// When using a non double-buffered hardware surface, render the UI
//	// to an off-screen surface first to avoid flickering / tearing.
//	if ((output_surface->flags & SDL_HWSURFACE) != 0
//	    && (output_surface->flags & SDL_DOUBLEBUF) == 0) {
		return back_surface;
//	}
//#endif

//	return output_surface;
}

void UiFadeIn(bool draw_cursor);
void UiHandleEvents(SDL_Event* event);
void UiItemMouseEvents(SDL_Event* event);
void LoadBackgroundArt(const char* pszFile, int frames = 1);
void UiAddBackground(std::vector<UiItemBase*>* vecDialog);
void UiAddLogo(std::vector<UiItemBase*>* vecDialog);
void UiFocusNavigationSelect();
void UiFocusNavigationEsc();
void UiFocusNavigationYesNo();
void UiInitList(unsigned listSize, void (*fnFocus)(unsigned index) = NULL, void (*fnSelect)(unsigned index) = NULL, void (*fnEsc)() = NULL, bool (*fnYesNo)() = NULL);
void UiInitScrollBar(UiScrollBar* ui_sb, unsigned viewport_size);
void UiClearScreen();
void UiPollAndRender();
void UiRenderItems(const std::vector<UiItemBase*> &uiItems);
void UiClearItems(std::vector<UiItemBase*> &uiItems);
void UiClearListItems();
//void UiInitList_clear();

void mainmenu_restart_repintro();

DEVILUTION_END_NAMESPACE
