#include "diabloui.h"

#include "utils/utf8.h"
#include <string>

#include "controls/menu_controls.h"

#include "DiabloUI/scrollbar.h"
#include "DiabloUI/text_draw.h"
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

#define FOCUS_FRAME_COUNT	8

BYTE* gbBackCel;
static BYTE* gbLogoCel;
static BYTE* gbFocusCelSmall;
static BYTE* gbFocusCelMed;
static BYTE* gbFocusCelBig;
BYTE* gbHerosCel;
BYTE* gbSmlButtonCel;

void (*gfnSoundFunction)(int gfx, int rndCnt);
static void (*gfnListFocus)(unsigned index);
static void (*gfnListSelect)(unsigned index);
static void (*gfnListEsc)();
static bool (*gfnListYesNo)();
std::vector<UiListItem*> gUIListItems;
std::vector<UiItemBase*> gUiItems;
unsigned SelectedItem;
static unsigned SelectedItemMax;
static unsigned ListViewportSize;
unsigned ListOffset;
UiEdit* gUiEditField;

static Uint32 _gdwFadeTc;
static int _gnFadeValue = 0;

struct ScrollBarState {
	char upPressCounter;
	char downPressCounter;
};
ScrollBarState scrollBarState;

void UiInitList(unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)(), bool (*fnYesNo)())
{
	SelectedItem = 0;
	SelectedItemMax = listSize != 0 ? listSize - 1 : 0;
	ListViewportSize = listSize;
	ListOffset = 0;
	gfnListFocus = fnFocus;
	gfnListSelect = fnSelect;
	gfnListEsc = fnEsc;
	gfnListYesNo = fnYesNo;
	if (fnFocus != NULL)
		fnFocus(0);

	gUiEditField = NULL;
#ifndef __SWITCH__
	SDL_StopTextInput(); // input is enabled by default
#endif
	for (unsigned i = 0; i < gUiItems.size(); i++) {
		if (gUiItems[i]->m_type == UI_EDIT) {
			gUiEditField = (UiEdit*)gUiItems[i];
			SDL_SetTextInputRect(&gUiEditField->m_rect);
#ifdef __SWITCH__
			switch_start_text_input(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_max_length);
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

void UiInitScrollBar(UiScrollBar* uiSb, unsigned viewportSize)
{
	ListViewportSize = viewportSize;
	if (ListViewportSize > SelectedItemMax) {
		uiSb->m_iFlags |= UIS_HIDDEN;
	} else {
		uiSb->m_iFlags &= ~UIS_HIDDEN;
	}
	scrollBarState.upPressCounter = -1;
	scrollBarState.downPressCounter = -1;
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
}

static void UiFocusDown()
{
	if (SelectedItem < SelectedItemMax)
		UiFocus(SelectedItem + 1);
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
	/*switch (event->type) {
	case SDL_KEYUP:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
#if SDL_VERSION_ATLEAST(2, 0, 0)
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
	case SDL_SYSWMEVENT:*/
		mainmenu_restart_repintro();
	//	break;
	//}

	if (HandleMenuAction(GetMenuAction(*event)))
		return;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (event->type == SDL_MOUSEWHEEL) {
		if (event->wheel.y > 0) {
			UiFocusUp();
		} else if (event->wheel.y < 0) {
			UiFocusDown();
		}
		return;
	}
#else
	if (event->type == SDL_MOUSEBUTTONDOWN) {
		switch (event->button.button) {
		case SDL_BUTTON_WHEELUP:
			UiFocusUp();
			return;
		case SDL_BUTTON_WHEELDOWN:
			UiFocusDown();
			return;
		}
	}
#endif
	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
		UiItemMouseEvents(event);
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
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
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
	assert(gbLogoCel == NULL);
	gbLogoCel = LoadFileInMem("ui_art\\smlogo.CEL");
	assert(gbFocusCelSmall == NULL);
	gbFocusCelSmall = LoadFileInMem("ui_art\\focus16.CEL");
	assert(gbFocusCelMed == NULL);
	gbFocusCelMed = LoadFileInMem("ui_art\\focus.CEL");
	assert(gbFocusCelBig == NULL);
	gbFocusCelBig = LoadFileInMem("ui_art\\focus42.CEL");

	NewCursor(CURSOR_HAND);
}

static void UnloadUiGFX()
{
	MemFreeDbg(gbLogoCel);
	MemFreeDbg(gbFocusCelSmall);
	MemFreeDbg(gbFocusCelMed);
	MemFreeDbg(gbFocusCelBig);
}

void UiInitialize()
{
	LoadUiGFX();
	//if (pCursCels != NULL) {
		if (SDL_ShowCursor(SDL_DISABLE) < 0) {
			sdl_error(ERR_SDL_UI_CURSOR_DISABLE);
		}
	//}
}

void UiDestroy()
{
	UnloadUiGFX();
	//UiInitList_clear();
}

void LoadBackgroundArt(const char* pszFile, const char* palette)
{
	assert(gbBackCel == NULL);
	//if (gbBackCel != NULL)
	//	MemFreeDbg(gbBackCel);
	gbBackCel = LoadFileInMem(pszFile);

	LoadPalette(palette);
	PaletteFadeIn(true);

	// help the render loops by setting up an initial fade level
	_gdwFadeTc = 0;
	_gnFadeValue = 0;
	SetFadeLevel(0);
/* unnecessary, because the render loops are supposed to start with this.
	UiClearScreen();
//#ifdef USE_SDL1
//	if (DiabloUiSurface() == back_surface)
		BltFast();
//#endif
	RenderPresent();*/
}

void UiAddBackground(std::vector<UiItemBase*>* vecDialog)
{
	assert(gbBackCel != NULL);
	SDL_Rect rect = { PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT };
	vecDialog->push_back(new UiImage(gbBackCel, 0, rect, UIS_CENTER, false));
}

void UiAddLogo(std::vector<UiItemBase*>* vecDialog)
{
	assert(gbLogoCel != NULL);
	SDL_Rect rect = { PANEL_LEFT + (PANEL_WIDTH - 390) / 2, PANEL_TOP, 390, 154 };
	vecDialog->push_back(new UiImage(gbLogoCel, 15, rect, UIS_CENTER, true));
}

void UiFadeIn(bool draw_cursor)
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
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (sgbControllerActive)
		draw_cursor = false;
#endif
	scrollrt_draw_screen(draw_cursor);
}

int GetAnimationFrame(int frames, int animFrameLenMs)
{
	//assert(frames > 0 && fps > 0);
	return (SDL_GetTicks() / animFrameLenMs) % frames;
}

static void DrawSelector(const SDL_Rect &rect)
{
	int size, frame, x, y;
	BYTE* selCel;

	assert(gbFocusCelSmall != NULL);
	assert(gbFocusCelMed != NULL);
	assert(gbFocusCelBig != NULL);
	size = 20;
	selCel = gbFocusCelSmall;
	if (rect.h >= 42) {
		size = 42;
		selCel = gbFocusCelBig;
	} else if (rect.h >= 30) {
		size = 30;
		selCel = gbFocusCelMed;
	}
	frame = GetAnimationFrame(FOCUS_FRAME_COUNT) + 1;
	x = SCREEN_X + rect.x;
	y = SCREEN_Y + rect.y + (rect.h - size) / 2 + size; // TODO FOCUS_MED appears higher than the box

	CelDraw(x, y, selCel, frame, size);
	x += rect.w - size;
	CelDraw(x, y, selCel, frame, size);
}

void UiClearScreen()
{
	if (SCREEN_WIDTH > PANEL_WIDTH) // Background size
		SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
}

void UiPollAndRender()
{
	UiRenderItems(gUiItems);
	UiFadeIn(true);

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		UiFocusNavigation(&event);
		UiHandleEvents(&event);
	}
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	HandleMenuAction(GetMenuHeldUpDownAction());
#endif
#ifdef __3DS__
	// Keyboard blocks until input is finished
	// so defer until after render and fade-in
	ctr_vkbdFlush();
#endif
}

static void Render(const UiText* uiArtText)
{
	DrawArtStr(uiArtText->m_text, uiArtText->m_rect, uiArtText->m_iFlags);
}

static void Render(const UiImage* uiImage)
{
	int x = SCREEN_X + uiImage->m_rect.x;
	int y = SCREEN_Y + uiImage->m_rect.y + uiImage->m_rect.h;
	int frame = uiImage->m_animated ? GetAnimationFrame(uiImage->m_frame) : uiImage->m_frame;

	CelDraw(x, y, uiImage->m_cel_data, frame + 1, uiImage->m_rect.w);
}

static void Render(const UiTxtButton* uiButton)
{
	DrawArtStr(uiButton->m_text, uiButton->m_rect, uiButton->m_iFlags);
}

static void Render(const UiButton* button)
{
	int frame = button->m_pressed ? UiButton::PRESSED : UiButton::DEFAULT;

	CelDraw(SCREEN_X + button->m_rect.x, SCREEN_Y + button->m_rect.y + 28, gbSmlButtonCel, frame + 1, 110);	

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
		int bgYEnd = SCREEN_Y + DownArrowRect(uiSb).y;
		int bgX = SCREEN_X + uiSb->m_rect.x;
		int bgY = SCREEN_Y + uiSb->m_rect.y + SCROLLBAR_ARROW_HEIGHT;
		assert(scrollBarBackCel != NULL);
		while (bgY < bgYEnd) {
			bgY += SCROLLBAR_BG_HEIGHT;
			if (bgYEnd < bgY)
				bgY = bgYEnd;
			CelDraw(bgX, bgY, scrollBarBackCel, 1, SCROLLBAR_BG_WIDTH);
		}
	}
	// Arrows:
	assert(scrollBarArrowCel != NULL);
	{
		SDL_Rect rect = UpArrowRect(uiSb);
		int frame = scrollBarState.upPressCounter != -1 ? ScrollBarArrowFrame_UP_ACTIVE : ScrollBarArrowFrame_UP;
		CelDraw(SCREEN_X + rect.x, SCREEN_Y + rect.y, scrollBarArrowCel, frame + 1,SCROLLBAR_ARROW_WIDTH);
	}
	{
		SDL_Rect rect = DownArrowRect(uiSb);
		int frame = scrollBarState.downPressCounter != -1 ? ScrollBarArrowFrame_DOWN_ACTIVE : ScrollBarArrowFrame_DOWN;
		CelDraw(SCREEN_X + rect.x, SCREEN_Y + rect.y, scrollBarArrowCel, frame + 1,SCROLLBAR_ARROW_WIDTH);
	}
	// Thumb:
	assert(scrollBarThumbCel != NULL);
	if (SelectedItemMax > 0) {
		SDL_Rect rect = ThumbRect(uiSb, SelectedItem, SelectedItemMax);
		CelDraw(SCREEN_X + rect.x, SCREEN_Y + rect.y, scrollBarThumbCel, 1, SCROLLBAR_THUMB_WIDTH);
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
	case UI_TEXT:
		Render(static_cast<UiText *>(item));
		break;
	case UI_IMAGE:
		Render(static_cast<UiImage *>(item));
		break;
	case UI_TXT_BUTTON:
		Render(static_cast<UiTxtButton *>(item));
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

static bool HandleMouseEventArtTextButton(const SDL_Event &event, const UiTxtButton* uiButton)
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
	case UI_TXT_BUTTON:
		return HandleMouseEventArtTextButton(event, static_cast<UiTxtButton *>(item));
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

void UiClearListItems()
{
	for (size_t i = 0; i < gUIListItems.size(); i++) {
		UiListItem* pUIItem = gUIListItems[i];
		delete pUIItem;
	}
	gUIListItems.clear();
}

void UiItemMouseEvents(SDL_Event* event)
{
	if (event->button.button != SDL_BUTTON_LEFT)
		return; // false;

	// In SDL2 mouse events already use logical coordinates.
#ifdef USE_SDL1
	OutputToLogical(&event->button.x, &event->button.y);
#endif

	//bool handled = false;
	for (unsigned i = 0; i < gUiItems.size(); i++) {
		if (HandleMouseEvent(*event, gUiItems[i])) {
			//handled = true;
			break;
		}
	}

	if (event->type == SDL_MOUSEBUTTONUP) {
		scrollBarState.downPressCounter = scrollBarState.upPressCounter = -1;
		for (unsigned i = 0; i < gUiItems.size(); i++) {
			UiItemBase *item = gUiItems[i];
			if (item->m_type == UI_BUTTON)
				HandleGlobalMouseUpButton(static_cast<UiButton *>(item));
		}
	}

	//return handled;
}

DEVILUTION_END_NAMESPACE
