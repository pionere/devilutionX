#pragma once

#include <cstddef>
#include <vector>
#include <SDL.h>

#include "DiabloUI/ui_item.h"
#include "utils/display.h"
#include "../diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern CelImageBuf* gbBackCel;
extern CelImageBuf* gbHerosCel;
extern CelImageBuf* gbSmlButtonCel;

extern void (*gfnSoundFunction)(int gfx, int rndCnt);
extern std::vector<UiListItem*> gUIListItems;
extern std::vector<UiItemBase*> gUiItems;
extern unsigned SelectedItem;
extern unsigned ListOffset;
extern UiEdit* gUiEditField;
extern bool gUiDrawCursor;

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

void UiFadeIn();
void UiHandleEvents(SDL_Event* event);
void LoadBackgroundArt(const char* pszFile, const char* palette);
void UiAddBackground(std::vector<UiItemBase*>* vecDialog);
void UiAddLogo(std::vector<UiItemBase*>* vecDialog);
void UiFocusNavigationSelect();
void UiFocusNavigationEsc();
void UiFocusNavigationYesNo();
void UiInitScreen(unsigned listSize, void (*fnFocus)(unsigned index) = NULL, void (*fnSelect)(unsigned index) = NULL, void (*fnEsc)() = NULL, bool (*fnYesNo)() = NULL);
void UiInitScrollBar(UiScrollBar* ui_sb, unsigned viewport_size);
void UiClearScreen();
void UiRenderAndPoll(std::vector<UiItemBase*>* addUiItems);
void UiRenderItems(const std::vector<UiItemBase*>& uiItems);
void UiClearItems(std::vector<UiItemBase*>& uiItems);
void UiClearListItems();
int GetAnimationFrame(int frames, int animFrameLenMs = 64);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
