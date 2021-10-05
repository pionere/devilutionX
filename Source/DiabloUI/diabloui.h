#pragma once

#include <cstddef>
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
#ifndef NOWIDESCREEN
extern Art ArtBackgroundWidescreen;
#endif
extern Art ArtCursor;
extern Art ArtHero;
extern Art SmlButton;

extern void (*gfnSoundFunction)(int gfx, int rndCnt);
extern unsigned SelectedItem;
extern unsigned ListOffset;
extern UiEdit* gUiEditField;

inline SDL_Surface* DiabloUiSurface()
{
	auto *output_surface = GetOutputSurface();

#ifdef USE_SDL1
	// When using a non double-buffered hardware surface, render the UI
	// to an off-screen surface first to avoid flickering / tearing.
	if ((output_surface->flags & SDL_HWSURFACE) != 0
	    && (output_surface->flags & SDL_DOUBLEBUF) == 0) {
		return back_surface;
	}
#endif

	return output_surface;
}

void UiFadeIn();
void UiHandleEvents(SDL_Event* event);
void UiItemMouseEvents(SDL_Event* event, const std::vector<UiItemBase*> &uiItems);
void DrawMouse();
void LoadBackgroundArt(const char* pszFile, int frames = 1);
void UiAddBackground(std::vector<UiItemBase*>* vecDialog);
void UiAddLogo(std::vector<UiItemBase*>* vecDialog);
void UiFocusNavigationSelect();
void UiFocusNavigationEsc();
void UiFocusNavigationYesNo();
void UiInitList(std::vector<UiItemBase*>* uiItems, unsigned listSize, void (*fnFocus)(unsigned index) = NULL, void (*fnSelect)(unsigned index) = NULL, void (*fnEsc)() = NULL, bool (*fnYesNo)() = NULL, bool wraps = false);
void UiInitScrollBar(UiScrollBar* ui_sb, unsigned viewport_size);
void UiClearScreen();
void UiPollAndRender();
void UiRenderItems(const std::vector<UiItemBase*> &uiItems);
void UiClearItems(std::vector<UiItemBase*> &uiItems);
void UiClearListItems(std::vector<UiListItem*> &uiItems);
//void UiInitList_clear();

void mainmenu_restart_repintro();

DEVILUTION_END_NAMESPACE
