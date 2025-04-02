#pragma once

#include <cstddef>
#include <vector>
#include <SDL.h>

#include "ui_item.h"
#include "../diabloui.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern CelImageBuf* gbHerosCel;

extern std::vector<UiListItem*> gUIListItems;
extern std::vector<UiItemBase*> gUiItems;
extern unsigned SelectedItem;
extern unsigned ListOffset;
extern bool gUiDrawCursor;

/*inline SDL_Surface* DiabloUiSurface()
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
}*/

bool UiPeekAndHandleEvents(Dvl_Event* event);
void LoadBackgroundArt(const char* pszFile, const char* palette);
void FreeBackgroundArt();
void UiAddBackground();
void UiAddLogo();
void UiFocusNavigationSelect();
void UiFocusNavigationEsc();
void UiFocusNavigationDelete();
void UiInitScreen(unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)());
void UiInitScrollBar(UiScrollBar* ui_sb, unsigned viewport_size, void (*fnDelete)());
void UiInitEdit(UiEdit* ui_edit);
void UiRender();
void UiRenderAndPoll();
void UiClearItems();
void UiClearListItems();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
