#include "diabloui.h"

#include "utils/stubs.h"
#include "utils/utf8.h"
#include <string>

#include "controls/menu_controls.h"

#include "DiabloUI/scrollbar.h"
#include "DiabloUI/art_draw.h"
#include "DiabloUI/text_draw.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/dialogs.h"
#include "controls/plrctrls.h"

#ifdef __SWITCH__
// for virtual keyboard on Switch
#include "platform/switch/keyboard.h"
#endif
#ifdef __vita__
// for virtual keyboard on Vita
#include "platform/vita/keyboard.h"
#endif
#ifdef __3DS__
// for virtual keyboard on 3DS
#include "platform/ctr/keyboard.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

unsigned SelectedItem = 0;
unsigned SelectedItemMax;
unsigned ListViewportSize = 1;
unsigned ListOffset = 0;

#define FOCUS_FRAME_COUNT	8

Art ArtLogoMed;
Art ArtFocus[NUM_FOCUS];
#ifndef NOWIDESCREEN
Art ArtBackgroundWidescreen;
#endif
Art ArtBackground;
Art ArtCursor;
Art SmlButton;
Art ArtHero;

void (*gfnSoundFunction)(int gfx, int rndCnt);
void (*gfnListFocus)(unsigned index);
void (*gfnListSelect)(unsigned index);
void (*gfnListEsc)();
bool (*gfnListYesNo)();
std::vector<UiItemBase*>* gUiItems;
bool UiItemsWraps;
UiEdit* gUiEditField;

static Uint32 _gdwFadeTc;
static int _gnFadeValue = 0;

struct ScrollBarState {
	char upPressCounter;
	char downPressCounter;

	ScrollBarState()
	{
		upPressCounter = -1;
		downPressCounter = -1;
	}
} scrollBarState;

void UiInitList(std::vector<UiItemBase*>* uiItems, unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)(), bool (*fnYesNo)(), bool itemsWraps)
{
	SelectedItem = 0;
	SelectedItemMax = listSize != 0 ? listSize - 1 : 0;
	ListViewportSize = listSize;
	ListOffset = 0;
	gfnListFocus = fnFocus;
	gfnListSelect = fnSelect;
	gfnListEsc = fnEsc;
	gfnListYesNo = fnYesNo;
	gUiItems = uiItems;
	UiItemsWraps = itemsWraps;
	if (fnFocus != NULL)
		fnFocus(0);

	gUiEditField = NULL;
#ifndef __SWITCH__
	SDL_StopTextInput(); // input is enabled by default
#endif
	for (unsigned i = 0; i < uiItems->size(); i++) {
		if ((*uiItems)[i]->m_type == UI_EDIT) {
			gUiEditField = (UiEdit*)(*uiItems)[i];
			SDL_SetTextInputRect(&gUiEditField->m_rect);
#ifdef __SWITCH__
			switch_start_text_input(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_max_length, /*multiline=*/0);
#elif defined(__vita__)
			vita_start_text_input(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_max_length);
#elif defined(__3DS__)
			ctr_vkbdInput(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_value, gUiEditField->m_max_length);
#else
			SDL_StartTextInput();
#endif
		}
	}
}

void UiInitScrollBar(UiScrollBar *uiSb, unsigned viewportSize)
{
	ListViewportSize = viewportSize;
	if (ListViewportSize > SelectedItemMax) {
		uiSb->m_iFlags |= UIS_HIDDEN;
	} else {
		uiSb->m_iFlags &= ~UIS_HIDDEN;
	}
}

/*void UiInitList_clear()
{
	SelectedItem = 0;
	SelectedItemMax = 0;
	ListViewportSize = 1;
	gfnListFocus = NULL;
	gfnListSelect = NULL;
	gfnListEsc = NULL;
	gfnListYesNo = NULL;
	gUiItems->clear();
	UiItemsWraps = false;
}*/

static void UiPlayMoveSound()
{
	assert(gfnSoundFunction != NULL);
	gfnSoundFunction(IS_TITLEMOV, 1);
}

static void UiPlaySelectSound()
{
	assert(gfnSoundFunction != NULL);
	gfnSoundFunction(IS_TITLSLCT, 1);
}

static void UiScrollIntoView()
{
	unsigned newOffset = ListOffset, index = SelectedItem;

	if (index < ListOffset)
		newOffset = index;
	else if (index >= ListOffset + ListViewportSize)
		newOffset = index - (ListViewportSize - 1);
	//if (newOffset != ListOffset) {
		ListOffset = newOffset;
	//}
}

static void UiFocus(unsigned itemIndex)
{
	if (SelectedItem == itemIndex)
		return;

	SelectedItem = itemIndex;

	UiScrollIntoView();

	UiPlayMoveSound();

	if (gfnListFocus != NULL)
		gfnListFocus(itemIndex);
}

static void UiFocusUp()
{
	if (SelectedItem > 0)
		UiFocus(SelectedItem - 1);
	else if (UiItemsWraps)
		UiFocus(SelectedItemMax);
}

static void UiFocusDown()
{
	if (SelectedItem < SelectedItemMax)
		UiFocus(SelectedItem + 1);
	else if (UiItemsWraps)
		UiFocus(0);
}

static void UiFocusPageUp()
{
	unsigned newpos;

	if (ListOffset > ListViewportSize)
		ListOffset -= ListViewportSize;
	else
		ListOffset = 0;

	newpos = SelectedItem;
	if (newpos > ListViewportSize)
		newpos -= ListViewportSize;
	else
		newpos = 0;
	UiFocus(newpos);
}

static void UiFocusPageDown()
{
	unsigned newpos, lastPageStart;

	lastPageStart = SelectedItemMax - (ListViewportSize - 1);
	if ((int)lastPageStart >= 0) {
		ListOffset += ListViewportSize;
		if (ListOffset > lastPageStart)
			ListOffset = lastPageStart;
	}

	newpos = SelectedItem;
	newpos += ListViewportSize;
	if (newpos > SelectedItemMax)
		newpos = SelectedItemMax;
	UiFocus(newpos);
}

static void UiCatToName(char* inBuf)
{
	std::string output = utf8_to_latin1(inBuf);
	int pos = strlen(gUiEditField->m_value);
	SStrCopy(&gUiEditField->m_value[pos], output.c_str(), gUiEditField->m_max_length - pos);
}

#ifdef __vita__
static void UiSetName(char* inBuf)
{
	std::string output = utf8_to_latin1(inBuf);
	strncpy(gUiEditField->m_value, output.c_str(), gUiEditField->m_max_length);
}
#endif

static bool HandleMenuAction(MenuAction menuAction)
{
	switch (menuAction) {
	case MenuAction_SELECT:
		UiFocusNavigationSelect();
		return true;
	case MenuAction_UP:
		UiFocusUp();
		return true;
	case MenuAction_DOWN:
		UiFocusDown();
		return true;
	case MenuAction_PAGE_UP:
		UiFocusPageUp();
		return true;
	case MenuAction_PAGE_DOWN:
		UiFocusPageDown();
		return true;
	case MenuAction_DELETE:
		UiFocusNavigationYesNo();
		return true;
	case MenuAction_BACK:
		if (gfnListEsc == NULL)
			return false;
		UiFocusNavigationEsc();
		return true;
	default:
		return false;
	}
}

static void UiFocusNavigation(SDL_Event* event)
{
	switch (event->type) {
	case SDL_KEYUP:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
#ifndef USE_SDL1
	case SDL_MOUSEWHEEL:
#endif
	case SDL_JOYBUTTONUP:
	case SDL_JOYAXISMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYHATMOTION:
#ifndef USE_SDL1
	case SDL_FINGERUP:
	case SDL_FINGERMOTION:
	case SDL_CONTROLLERBUTTONUP:
	case SDL_CONTROLLERAXISMOTION:
	case SDL_WINDOWEVENT:
#endif
	case SDL_SYSWMEVENT:
		mainmenu_restart_repintro();
		break;
	}

	if (HandleMenuAction(GetMenuAction(*event)))
		return;

#ifndef USE_SDL1
	if (event->type == SDL_MOUSEWHEEL) {
		if (event->wheel.y > 0) {
			UiFocusUp();
		} else if (event->wheel.y < 0) {
			UiFocusDown();
		}
		return;
	}
#endif
	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
		UiItemMouseEvents(event, *gUiItems);
		return;
	}

	if (gUiEditField != NULL) {
		switch (event->type) {
		case SDL_KEYDOWN: {
			switch (event->key.keysym.sym) {
#ifndef USE_SDL1
			case SDLK_v:
				if (SDL_GetModState() & KMOD_CTRL) {
					char *clipboard = SDL_GetClipboardText();
					if (clipboard != NULL) {
						UiCatToName(clipboard);
						SDL_free(clipboard);
					}
				}
				return;
#endif
			case SDLK_BACKSPACE:
			case SDLK_LEFT: {
				int nameLen = strlen(gUiEditField->m_value);
				if (nameLen > 0) {
					gUiEditField->m_value[nameLen - 1] = '\0';
				}
				return;
			}
			default:
				break;
			}
#ifdef USE_SDL1
			if ((event->key.keysym.mod & KMOD_CTRL) == 0) {
				Uint16 unicode = event->key.keysym.unicode;
				if (unicode && (unicode & 0xFF80) == 0) {
					char utf8[SDL_TEXTINPUTEVENT_TEXT_SIZE];
					utf8[0] = (char)unicode;
					utf8[1] = '\0';
					UiCatToName(utf8);
				}
			}
#endif
			break;
		}
#ifndef USE_SDL1
		case SDL_TEXTINPUT:
#ifdef __vita__
			UiSetName(event->text.text);
#else
			UiCatToName(event->text.text);
#endif
			return;
#endif
		default:
			break;
		}
	}
}

void UiHandleEvents(SDL_Event *event)
{
	if (event->type == SDL_MOUSEMOTION) {
#ifdef USE_SDL1
		OutputToLogical(&event->motion.x, &event->motion.y);
#endif
		MouseX = event->motion.x;
		MouseY = event->motion.y;
		return;
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
		if (GetAsyncKeyState(DVL_VK_MENU)) {
			ToggleFullscreen();
			return;
		}
	}

	if (event->type == SDL_QUIT)
		diablo_quit(0);

#ifndef USE_SDL1
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	HandleControllerAddedOrRemovedEvent(*event);
#endif

	if (event->type == SDL_WINDOWEVENT) {
		if (event->window.event == SDL_WINDOWEVENT_SHOWN)
			gbWndActive = true;
		else if (event->window.event == SDL_WINDOWEVENT_HIDDEN)
			gbWndActive = false;
	}
#endif
}

void UiFocusNavigationSelect()
{
	UiPlaySelectSound();
	if (gUiEditField != NULL) {
		if (gUiEditField->m_value[0] == '\0') {
			return;
		}
		gUiEditField = NULL;
#ifndef __SWITCH__
		SDL_StopTextInput();
#endif
	}
	if (gfnListSelect != NULL)
		gfnListSelect(SelectedItem);
}

void UiFocusNavigationEsc()
{
	UiPlaySelectSound();
	if (gUiEditField != NULL) {
		gUiEditField = NULL;
#ifndef __SWITCH__
		SDL_StopTextInput();
#endif
	}
	if (gfnListEsc != NULL)
		gfnListEsc();
}

void UiFocusNavigationYesNo()
{
	if (gfnListYesNo == NULL)
		return;

	if (gfnListYesNo())
		UiPlaySelectSound();
}

static SDL_bool IsInsideRect(const SDL_Event &event, const SDL_Rect &rect)
{
	const SDL_Point point = { event.button.x, event.button.y };
	return SDL_PointInRect(&point, &rect);
}

static void LoadUiGFX()
{
#ifdef HELLFIRE
	LoadMaskedArt("ui_art\\hf_logo2.pcx", &ArtLogoMed, 16, 250);
#else
	LoadMaskedArt("ui_art\\smlogo.pcx", &ArtLogoMed, 15, 250);
#endif
	LoadMaskedArt("ui_art\\focus16.pcx", &ArtFocus[FOCUS_SMALL], FOCUS_FRAME_COUNT, 250);
	LoadMaskedArt("ui_art\\focus.pcx", &ArtFocus[FOCUS_MED], FOCUS_FRAME_COUNT, 250);
	LoadMaskedArt("ui_art\\focus42.pcx", &ArtFocus[FOCUS_BIG], FOCUS_FRAME_COUNT, 250);
	LoadMaskedArt("ui_art\\cursor.pcx", &ArtCursor, 1, 0);
#ifdef HELLFIRE
	LoadArt("ui_art\\heros.pcx", &ArtHero, 6);
#else
	LoadArt("ui_art\\heros.pcx", &ArtHero, 4);
#endif
}

static void UnloadUiGFX()
{
	ArtLogoMed.Unload();
	ArtFocus[FOCUS_SMALL].Unload();
	ArtFocus[FOCUS_MED].Unload();
	ArtFocus[FOCUS_BIG].Unload();
	ArtCursor.Unload();
	ArtHero.Unload();
}

void UiInitialize()
{
	LoadUiGFX();
	LoadArtFonts();
	if (ArtCursor.surface != NULL) {
		if (SDL_ShowCursor(SDL_DISABLE) < 0) {
			sdl_fatal(ERR_SDL_UI_CURSOR_DISABLE);
		}
	}
}

void UiDestroy()
{
	UnloadUiGFX();
	UnloadArtFonts();
	//UiInitList_clear();
}

static int GetCenterOffset(int w, int bw)
{
	if (bw == 0) {
		bw = SCREEN_WIDTH;
	}

	return (bw - w) / 2;
}

void LoadBackgroundArt(const char* pszFile, int frames)
{
	assert(ArtBackground.surface == NULL);

	if (!LoadArt(pszFile, &ArtBackground, frames, orig_palette))
		return;

	ApplyGamma(logical_palette, orig_palette);

	_gdwFadeTc = 0;
	_gnFadeValue = 0;
	BlackPalette();
	UiClearScreen();
#ifdef USE_SDL1
	if (DiabloUiSurface() == back_surface)
		BltFast(NULL, NULL);
#endif
	RenderPresent();
}

void UiAddBackground(std::vector<UiItemBase *> *vecDialog)
{
#ifndef NOWIDESCREEN
	if (ArtBackgroundWidescreen.surface != NULL) {
		SDL_Rect rectw = { 0, UI_OFFSET_Y, 0, 0 };
		vecDialog->push_back(new UiImage(&ArtBackgroundWidescreen, 0, rectw, UIS_CENTER));
	}
#endif

	SDL_Rect rect = { 0, UI_OFFSET_Y, 0, 0 };
	vecDialog->push_back(new UiImage(&ArtBackground, 0, rect, UIS_CENTER));
}

void UiAddLogo(std::vector<UiItemBase*>* vecDialog)
{
	SDL_Rect rect = { 0, UI_OFFSET_Y, 0, 0 };
	vecDialog->push_back(new UiImage(&ArtLogoMed, rect));
}

void UiFadeIn()
{
	Uint32 currTc;

	if (_gnFadeValue < 256) {
		currTc = SDL_GetTicks();
		if (_gnFadeValue == 0 && _gdwFadeTc == 0)
			_gdwFadeTc = currTc;
		_gnFadeValue = (currTc - _gdwFadeTc) >> 0; // instead of >> 0 it was / 2.083 ... 32 frames @ 60hz
		if (_gnFadeValue > 256) {
			_gnFadeValue = 256;
			//_gdwFadeTc = 0;
		}
		SetFadeLevel(_gnFadeValue);
	}
#ifdef USE_SDL1
	if (DiabloUiSurface() == back_surface)
		BltFast(NULL, NULL);
#endif
	RenderPresent();
}

static void DrawSelector(const SDL_Rect &rect)
{
	int size = FOCUS_SMALL;
	if (rect.h >= 42)
		size = FOCUS_BIG;
	else if (rect.h >= 30)
		size = FOCUS_MED;
	Art *art = &ArtFocus[size];

	int frame = GetAnimationFrame(FOCUS_FRAME_COUNT);
	int y = rect.y + (rect.h - art->frame_height) / 2; // TODO FOCUS_MED appears higher than the box

	DrawArt(rect.x, y, art, frame);
	DrawArt(rect.x + rect.w - art->logical_width, y, art, frame);
}

void UiClearScreen()
{
	if (SCREEN_WIDTH > 640) // Background size
		SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
}

void UiPollAndRender()
{
	UiRenderItems(*gUiItems);
	DrawMouse();
	UiFadeIn();

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		UiFocusNavigation(&event);
		UiHandleEvents(&event);
	}
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	HandleMenuAction(GetMenuHeldUpDownAction());
#endif
#ifdef __3DS__
	// Keyboard blocks until input is finished
	// so defer until after render and fade-in
	ctr_vkbdFlush();
#endif
}

static void Render(const UiArtText* uiArtText)
{
	DrawArtStr(uiArtText->m_text, uiArtText->m_rect, uiArtText->m_iFlags);
}

static void Render(const UiImage* uiImage)
{
	Art* mArt = uiImage->m_art;
	int x = uiImage->m_rect.x;

	if ((uiImage->m_iFlags & UIS_CENTER) && mArt != NULL) {
		const int xOffset = GetCenterOffset(mArt->logical_width, uiImage->m_rect.w);
		x += xOffset;
	}
	int frame = uiImage->m_animated ? GetAnimationFrame(mArt->frames) : uiImage->m_frame;
	DrawArt(x, uiImage->m_rect.y, mArt, frame, uiImage->m_rect.w, uiImage->m_rect.h);
}

static void Render(const UiArtTextButton* uiButton)
{
	DrawArtStr(uiButton->m_text, uiButton->m_rect, uiButton->m_iFlags);
}

static void Render(UiButton* button)
{
	int frame = button->m_pressed ? UiButton::PRESSED : UiButton::DEFAULT;

	DrawArt(button->m_rect.x, button->m_rect.y, &SmlButton, frame, button->m_rect.w, button->m_rect.h);

	SDL_Rect textRect = button->m_rect;
	if (button->m_pressed)
		textRect.y++;
	DrawArtStr(button->m_text, textRect, UIS_CENTER | UIS_VCENTER | UIS_SMALL | UIS_GOLD);
}

static void Render(const UiList* uiList)
{
	for (unsigned i = 0; i < uiList->m_vecItems->size(); i++) {
		SDL_Rect rect = uiList->itemRect(i);
		if (i + ListOffset == SelectedItem)
			DrawSelector(rect);
		UiListItem* item = (*uiList->m_vecItems)[i];
		DrawArtStr(item->m_text, rect, uiList->m_iFlags);
	}
}

static void Render(const UiScrollBar* uiSb)
{
	// Bar background (tiled):
	{
		int bgYEnd = DownArrowRect(uiSb).y;
		int bgX = uiSb->m_rect.x;
		int bgY = uiSb->m_rect.y + SCROLLBAR_ARROW_HEIGHT;
		while (bgY < bgYEnd) {
			int drawH = std::min(bgY + SCROLLBAR_BG_HEIGHT, bgYEnd) - bgY;
			DrawArt(bgX, bgY, &ArtScrollBarBackground, 0, SCROLLBAR_BG_WIDTH, drawH);
			bgY += SCROLLBAR_BG_HEIGHT;
		}
	}

	// Arrows:
	{
		SDL_Rect rect = UpArrowRect(uiSb);
		int frame = scrollBarState.upPressCounter != -1 ? ScrollBarArrowFrame_UP_ACTIVE : ScrollBarArrowFrame_UP;
		DrawArt(rect.x, rect.y, &ArtScrollBarArrow, frame);
	}
	{
		SDL_Rect rect = DownArrowRect(uiSb);
		int frame = scrollBarState.downPressCounter != -1 ? ScrollBarArrowFrame_DOWN_ACTIVE : ScrollBarArrowFrame_DOWN;
		DrawArt(rect.x, rect.y, &ArtScrollBarArrow, frame);
	}

	// Thumb:
	if (SelectedItemMax > 0) {
		SDL_Rect rect = ThumbRect(uiSb, SelectedItem, SelectedItemMax);
		DrawArt(rect.x, rect.y, &ArtScrollBarThumb);
	}
}

static void Render(const UiEdit* uiEdit)
{
	DrawSelector(uiEdit->m_rect);
	SDL_Rect rect = uiEdit->m_rect;
	rect.x += 43;
	rect.y += 1;
	rect.w -= 86;
	DrawArtStr(uiEdit->m_value, rect, UIS_LEFT | UIS_MED | UIS_GOLD, /*drawTextCursor=*/true);
}

static void RenderItem(UiItemBase* item)
{
	switch (item->m_type) {
	case UI_ART_TEXT:
		Render(static_cast<UiArtText *>(item));
		break;
	case UI_IMAGE:
		Render(static_cast<UiImage *>(item));
		break;
	case UI_ART_TEXT_BUTTON:
		Render(static_cast<UiArtTextButton *>(item));
		break;
	case UI_BUTTON:
		Render(static_cast<UiButton *>(item));
		break;
	case UI_LIST:
		Render(static_cast<UiList *>(item));
		break;
	case UI_SCROLLBAR:
		Render(static_cast<UiScrollBar *>(item));
		break;
	case UI_EDIT:
		Render(static_cast<UiEdit *>(item));
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static bool HandleMouseEventArtTextButton(const SDL_Event &event, const UiArtTextButton* uiButton)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return false;
	uiButton->m_action();
	return true;
}

static bool HandleMouseEventButton(const SDL_Event &event, UiButton* button)
{
	if (event.type == SDL_MOUSEBUTTONDOWN)
		button->m_pressed = true;
	else if (event.type == SDL_MOUSEBUTTONUP)
		button->m_action();
	else
		return false;
	return true;
}

#ifdef USE_SDL1
Uint32 dbClickTimer;
#endif

static bool HandleMouseEventList(const SDL_Event &event, UiList* uiList)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return false;

	const unsigned index = uiList->indexAt(event.button.y) + ListOffset;

	if (gfnListFocus != NULL && SelectedItem != index) {
		UiFocus(index);
#ifdef USE_SDL1
		dbClickTimer = SDL_GetTicks();
	} else if (gfnListFocus == NULL || dbClickTimer + 500 >= SDL_GetTicks()) {
#else
	} else if (gfnListFocus == NULL || event.button.clicks >= 2) {
#endif
		SelectedItem = index;
		UiFocusNavigationSelect();
#ifdef USE_SDL1
	} else {
		dbClickTimer = SDL_GetTicks();
#endif
	}

	return true;
}

static bool HandleMouseEventScrollBar(const SDL_Event &event, const UiScrollBar* uiSb)
{
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		int y = event.button.y - uiSb->m_rect.y;
		if (y >= uiSb->m_rect.h - SCROLLBAR_ARROW_HEIGHT) {
			// down arrow
			//scrollBarState.downArrowPressed = true;
			scrollBarState.downPressCounter--;
			if (scrollBarState.downPressCounter < 0) {
				scrollBarState.downPressCounter = 2;
				UiFocusDown();
			}
		} else if (y < SCROLLBAR_ARROW_HEIGHT) {
			// up arrow
			//scrollBarState.upArrowPressed = true;
			scrollBarState.upPressCounter--;
			if (scrollBarState.upPressCounter < 0) {
				scrollBarState.upPressCounter = 2;
				UiFocusUp();
			}
		} else {
			// Scroll up or down based on thumb position.
			const SDL_Rect thumbRect = ThumbRect(uiSb, SelectedItem, SelectedItemMax);
			if (event.button.y < thumbRect.y) {
				UiFocusPageUp();
			} else if (event.button.y > thumbRect.y + thumbRect.h) {
				UiFocusPageDown();
			}
		}
		return true;
	}
	return false;
}

static bool HandleMouseEvent(const SDL_Event &event, UiItemBase* item)
{
	if ((item->m_iFlags & (UIS_HIDDEN | UIS_DISABLED)) || !IsInsideRect(event, item->m_rect))
		return false;
	switch (item->m_type) {
	case UI_ART_TEXT_BUTTON:
		return HandleMouseEventArtTextButton(event, static_cast<UiArtTextButton *>(item));
	case UI_BUTTON:
		return HandleMouseEventButton(event, static_cast<UiButton *>(item));
	case UI_LIST:
		return HandleMouseEventList(event, static_cast<UiList *>(item));
	case UI_SCROLLBAR:
		return HandleMouseEventScrollBar(event, static_cast<UiScrollBar *>(item));
	default:
		return false;
	}
}

static void HandleGlobalMouseUpButton(UiButton* button)
{
	button->m_pressed = false;
}

void UiRenderItems(const std::vector<UiItemBase *> &uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++)
		if (!(uiItems[i]->m_iFlags & UIS_HIDDEN))
			RenderItem(uiItems[i]);
}

void UiClearItems(std::vector<UiItemBase *> &uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++) {
		UiItemBase* pUIItem = uiItems[i];
		delete pUIItem;
	}
	uiItems.clear();
}

void UiClearListItems(std::vector<UiListItem *> &uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++) {
		UiListItem* pUIItem = uiItems[i];
		delete pUIItem;
	}
	uiItems.clear();
}

void UiItemMouseEvents(SDL_Event* event, const std::vector<UiItemBase*> &uiItems)
{
	if (event->button.button != SDL_BUTTON_LEFT)
		return; // false;

	// In SDL2 mouse events already use logical coordinates.
#ifdef USE_SDL1
	OutputToLogical(&event->button.x, &event->button.y);
#endif

	//bool handled = false;
	for (unsigned i = 0; i < uiItems.size(); i++) {
		if (HandleMouseEvent(*event, uiItems[i])) {
			//handled = true;
			break;
		}
	}

	if (event->type == SDL_MOUSEBUTTONUP) {
		scrollBarState.downPressCounter = scrollBarState.upPressCounter = -1;
		for (unsigned i = 0; i < uiItems.size(); i++) {
			UiItemBase *item = uiItems[i];
			if (item->m_type == UI_BUTTON)
				HandleGlobalMouseUpButton(static_cast<UiButton *>(item));
		}
	}

	//return handled;
}

void DrawMouse()
{
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (sgbControllerActive)
		return;
#endif

	DrawArt(MouseX, MouseY, &ArtCursor);
}

DEVILUTION_END_NAMESPACE
