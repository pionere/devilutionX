#include "diabloui.h"

#include "utils/utf8.h"

#include "controls/menu_controls.h"

#include "DiabloUI/scrollbar.h"
#include "DiabloUI/text_draw.h"
#include "DiabloUI/dialogs.h"
#include "controls/plrctrls.h"
#include "controls/touch.h"
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"
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

#define FOCUS_FRAME_COUNT 8

CelImageBuf* gbBackCel;
static CelImageBuf* gbLogoCelSmall;
static CelImageBuf* gbFocusCelSmall;
static CelImageBuf* gbFocusCelMed;
static CelImageBuf* gbFocusCelBig;
CelImageBuf* gbHerosCel;
CelImageBuf* gbSmlButtonCel;

void (*gfnSoundFunction)(int gfx, int rndCnt);
static void (*gfnListFocus)(unsigned index);
static void (*gfnListSelect)(unsigned index);
static void (*gfnListEsc)();
static bool (*gfnListDelete)();
std::vector<UiListItem*> gUIListItems;
std::vector<UiItemBase*> gUiItems;
unsigned SelectedItem;
static unsigned SelectedItemMax;
static unsigned ListViewportSize;
unsigned ListOffset;
/** The edit field on the current screen (if exists) */
UiEdit* gUiEditField;
/** Specifies whether the cursor should be shown on the current screen + controlls key/mouse-press events if set to false. TODO: better solution? */
bool gUiDrawCursor;

static Uint32 _gdwFadeTc;
static int _gnFadeValue = 0;

typedef struct ScrollBarState {
	int8_t upPressCounter;
	int8_t downPressCounter;
} ScrollBarState;
static ScrollBarState scrollBarState;

void UiInitScreen(unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)(), bool (*fnDelete)())
{
	gUiDrawCursor = true;
	SelectedItem = 0;
	SelectedItemMax = listSize != 0 ? listSize - 1 : 0;
	ListViewportSize = listSize;
	ListOffset = 0;
	gfnListFocus = fnFocus;
	gfnListSelect = fnSelect;
	gfnListEsc = fnEsc;
	gfnListDelete = fnDelete;
	if (fnFocus != NULL)
		fnFocus(0);

	gUiEditField = NULL;
#if !defined(__SWITCH__) && !defined(__vita__) && !defined(__3DS__)
	SDL_StopTextInput(); // input is enabled by default if !SDL_HasScreenKeyboardSupport
#endif
	for (unsigned i = 0; i < gUiItems.size(); i++) {
		if (gUiItems[i]->m_type == UI_EDIT) {
			gUiEditField = (UiEdit*)gUiItems[i];
#ifdef __SWITCH__
			switch_start_text_input(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_max_length);
#elif defined(__vita__)
			vita_start_text_input(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_max_length);
#elif defined(__3DS__)
			ctr_vkbdInput(gUiEditField->m_hint, gUiEditField->m_value, gUiEditField->m_value, gUiEditField->m_max_length);
#else
			SDL_SetTextInputRect(&gUiEditField->m_rect);
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

static void UiCatToText(char* inBuf)
{
	char* output = utf8_to_latin1(inBuf);
	unsigned pos = gUiEditField->m_curpos;
	char* text = gUiEditField->m_value;
	unsigned maxlen = gUiEditField->m_max_length;
	SStrCopy(tempstr, &text[pos], std::min((unsigned)sizeof(tempstr) - 1, maxlen - pos));
	SStrCopy(&text[pos], output, maxlen - pos);
	mem_free_dbg(output);
	pos = strlen(text);
	gUiEditField->m_curpos = pos;
	SStrCopy(&text[pos], tempstr, maxlen - pos);
}

#ifdef __vita__
static void UiSetText(char* inBuf)
{
	char* output = utf8_to_latin1(inBuf);
	SStrCopy(gUiEditField->m_value, output, gUiEditField->m_max_length);
	mem_free_dbg(output);
}
#endif

static bool HandleMenuAction(MenuAction menuAction)
{
	switch (menuAction) {
	case MenuAction_NONE:
		break;
	case MenuAction_SELECT:
		UiFocusNavigationSelect();
		return true;
	case MenuAction_BACK:
		UiFocusNavigationEsc();
		return true;
	case MenuAction_DELETE:
		UiFocusNavigationDelete();
		return true;
	case MenuAction_UP:
		UiFocusUp();
		return true;
	case MenuAction_DOWN:
		UiFocusDown();
		return true;
	case MenuAction_LEFT:
	case MenuAction_RIGHT:
		break;
	case MenuAction_PAGE_UP:
		UiFocusPageUp();
		return true;
	case MenuAction_PAGE_DOWN:
		UiFocusPageDown();
		return true;
	default:
		ASSUME_UNREACHABLE
	}
	return false;
}

void UiFocusNavigationSelect()
{
	if (gUiDrawCursor)
		UiPlaySelectSound();
#if !defined(__SWITCH__) && !defined(__vita__) && !defined(__3DS__)
	if (gUiEditField != NULL) {
		if (gUiEditField->m_value[0] == '\0') {
			return;
		}
		gUiEditField = NULL;
		//if (SDL_IsTextInputShown()) {
			SDL_StopTextInput();
		//	return;
		//}
	}
#endif
	if (gfnListSelect != NULL)
		gfnListSelect(SelectedItem);
}

void UiFocusNavigationEsc()
{
	if (gUiDrawCursor)
		UiPlayMoveSound();
	if (gfnListEsc != NULL)
		gfnListEsc();
}

void UiFocusNavigationDelete()
{
	if (gfnListDelete == NULL)
		return;

	if (gfnListDelete())
		UiPlaySelectSound();
}

static SDL_bool IsInsideRect(const SDL_Event& event, const SDL_Rect& rect)
{
	const SDL_Point point = { event.button.x, event.button.y };
	return SDL_PointInRect(&point, &rect);
}

static void LoadUiGFX()
{
	assert(gbLogoCelSmall == NULL);
	gbLogoCelSmall = CelLoadImage("ui_art\\smlogo.CEL", SMALL_LOGO_WIDTH);
	assert(gbFocusCelSmall == NULL);
	gbFocusCelSmall = CelLoadImage("ui_art\\focus16.CEL", FOCUS_SMALL);
	assert(gbFocusCelMed == NULL);
	gbFocusCelMed = CelLoadImage("ui_art\\focus.CEL", FOCUS_MEDIUM);
	assert(gbFocusCelBig == NULL);
	gbFocusCelBig = CelLoadImage("ui_art\\focus42.CEL", FOCUS_BIG);

	NewCursor(CURSOR_HAND);
}

static void UnloadUiGFX()
{
	MemFreeDbg(gbLogoCelSmall);
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
}

void LoadBackgroundArt(const char* pszFile, const char* palette)
{
	assert(gbBackCel == NULL);
	//FreeBackgroundArt();
	gbBackCel = CelLoadImage(pszFile, PANEL_WIDTH);

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

void FreeBackgroundArt()
{
	MemFreeDbg(gbBackCel);
}

void UiAddBackground(std::vector<UiItemBase*>* vecDialog)
{
	assert(gbBackCel != NULL);
	SDL_Rect rect = { PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT };
	vecDialog->push_back(new UiImage(gbBackCel, 0, rect, false));
}

void UiAddLogo(std::vector<UiItemBase*>* vecDialog)
{
	assert(gbLogoCelSmall != NULL);
	SDL_Rect rect = { PANEL_MIDX(SMALL_LOGO_WIDTH), SMALL_LOGO_TOP, SMALL_LOGO_WIDTH, SMALL_LOGO_HEIGHT };
	vecDialog->push_back(new UiImage(gbLogoCelSmall, 15, rect, true));
}

void UiFadeIn()
{
	Uint32 currTc;

	if (_gnFadeValue < FADE_LEVELS) {
		currTc = SDL_GetTicks();
		if (_gnFadeValue == 0 && _gdwFadeTc == 0)
			_gdwFadeTc = currTc;
		_gnFadeValue = (currTc - _gdwFadeTc) >> 0; // instead of >> 0 it was / 2.083 ... 32 frames @ 60hz
		if (_gnFadeValue > FADE_LEVELS) {
			_gnFadeValue = FADE_LEVELS;
			//_gdwFadeTc = 0;
		}
		SetFadeLevel(_gnFadeValue);
	}
	scrollrt_draw_screen(gUiDrawCursor);
}

int GetAnimationFrame(int frames, int animFrameLenMs)
{
	//assert(frames > 0 && fps > 0);
	return (SDL_GetTicks() / animFrameLenMs) % frames;
}

static void DrawSelector(const SDL_Rect& rect)
{
	int size, frame, x, y;
	CelImageBuf* selCel;

	assert(gbFocusCelSmall != NULL);
	assert(gbFocusCelMed != NULL);
	assert(gbFocusCelBig != NULL);
	selCel = gbFocusCelSmall;
	if (rect.h >= FOCUS_BIG) {
		selCel = gbFocusCelBig;
	} else if (rect.h >= FOCUS_MEDIUM) {
		selCel = gbFocusCelMed;
	}
	size = selCel->ciWidth;
	frame = GetAnimationFrame(FOCUS_FRAME_COUNT) + 1;
	x = SCREEN_X + rect.x;
	y = SCREEN_Y + rect.y + (unsigned)(rect.h + size) / 2 - 1; // TODO FOCUS_MED appears higher than the box

	CelDraw(x, y, selCel, frame);
	x += rect.w - size;
	CelDraw(x, y, selCel, frame);
}

void UiClearScreen()
{
	if (SCREEN_WIDTH > PANEL_WIDTH) { // Background size
		// SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
		ClearScreenBuffer();
	}
}

void UiRenderAndPoll(std::vector<UiItemBase*>* addUiItems)
{
	UiClearScreen();
	if (addUiItems != NULL)
		UiRenderItems(*addUiItems);
	UiRenderItems(gUiItems);
	UiFadeIn();

	SDL_Event event;
	while (UiPeekAndHandleEvents(&event)) {
		;
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
	int frame = uiImage->m_animated ? GetAnimationFrame(uiImage->m_frame) : uiImage->m_frame;
	int x = SCREEN_X + uiImage->m_rect.x;
	int y = SCREEN_Y + uiImage->m_rect.y + uiImage->m_rect.h - 1;

	CelDraw(x, y, uiImage->m_cel_data, frame + 1);
}

static void Render(const UiTxtButton* uiButton)
{
	DrawArtStr(uiButton->m_text, uiButton->m_rect, uiButton->m_iFlags);
}

static void Render(const UiButton* button)
{
	int frame = button->m_pressed ? 2 : 1;
	int x = SCREEN_X + button->m_rect.x;
	int y = SCREEN_Y + button->m_rect.y + 28 - 1;

	CelDraw(x, y, gbSmlButtonCel, frame);

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
		int bgYEnd = SCREEN_Y + DownArrowRect(uiSb).y - 1;
		int bgX = SCREEN_X + uiSb->m_rect.x;
		int bgY = SCREEN_Y + uiSb->m_rect.y + SCROLLBAR_ARROW_HEIGHT - 1;
		assert(scrollBarBackCel != NULL);
		while (bgY < bgYEnd) {
			bgY += SCROLLBAR_BG_HEIGHT;
			if (bgYEnd < bgY)
				bgY = bgYEnd;
			CelDraw(bgX, bgY, scrollBarBackCel, 1);
		}
	}
	// Arrows:
	assert(scrollBarArrowCel != NULL);
	{
		SDL_Rect rect = UpArrowRect(uiSb);
		rect.y--;
		int frame = scrollBarState.upPressCounter != -1 ? ScrollBarArrowFrame_UP_ACTIVE : ScrollBarArrowFrame_UP;
		CelDraw(SCREEN_X + rect.x, SCREEN_Y + rect.y, scrollBarArrowCel, frame + 1);
	}
	{
		SDL_Rect rect = DownArrowRect(uiSb);
		rect.y--;
		int frame = scrollBarState.downPressCounter != -1 ? ScrollBarArrowFrame_DOWN_ACTIVE : ScrollBarArrowFrame_DOWN;
		CelDraw(SCREEN_X + rect.x, SCREEN_Y + rect.y, scrollBarArrowCel, frame + 1);
	}
	// Thumb:
	assert(scrollBarThumbCel != NULL);
	if (SelectedItemMax > 0) {
		SDL_Rect rect = ThumbRect(uiSb, SelectedItem, SelectedItemMax);
		rect.y--;
		CelDraw(SCREEN_X + rect.x, SCREEN_Y + rect.y, scrollBarThumbCel, 1);
	}
}

static void Render(const UiEdit* uiEdit)
{
	DrawSelector(uiEdit->m_rect);
	SDL_Rect rect = uiEdit->m_rect;
	rect.x += 43;
	rect.y += 1;
	// rect.w -= 86;
	char* text = uiEdit->m_value;
	// render the text
	DrawArtStr(text, rect, UIS_LEFT | UIS_MED | UIS_GOLD);
	// render the cursor
	if (GetAnimationFrame(2, 512) != 0) {
		unsigned curpos = uiEdit->m_curpos;
		char tmp = text[curpos];
		text[curpos] = '\0';
		int w = GetBigStringWidth(text);
		int h = 22;
		text[curpos] = tmp;
		PrintBigChar(SCREEN_X + rect.x + w, SCREEN_Y + rect.y + h, '|', COL_WHITE);
	}
}

static void RenderItem(UiItemBase* item)
{
	if (item->m_iFlags & UIS_HIDDEN)
		return;

	switch (item->m_type) {
	case UI_TEXT:
		Render(static_cast<UiText*>(item));
		break;
	case UI_IMAGE:
		Render(static_cast<UiImage*>(item));
		break;
	case UI_TXT_BUTTON:
		Render(static_cast<UiTxtButton*>(item));
		break;
	case UI_BUTTON:
		Render(static_cast<UiButton*>(item));
		break;
	case UI_LIST:
		Render(static_cast<UiList*>(item));
		break;
	case UI_SCROLLBAR:
		Render(static_cast<UiScrollBar*>(item));
		break;
	case UI_EDIT:
		Render(static_cast<UiEdit*>(item));
		break;
	case UI_CUSTOM:
		static_cast<UiCustom*>(item)->m_render();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static bool HandleMouseEventArtTextButton(const SDL_Event& event, const UiTxtButton* uiButton)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return true;
	uiButton->m_action();
	return true;
}

static bool HandleMouseEventButton(const SDL_Event& event, UiButton* button)
{
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		button->m_pressed = true;
	} else if (button->m_pressed) {
		// assert(event.type == SDL_MOUSEBUTTONUP);
		button->m_action();
	}
	return true;
}

#ifdef USE_SDL1
Uint32 dbClickTimer;
#endif

static bool HandleMouseEventList(const SDL_Event& event, UiList* uiList)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return true;

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

static bool HandleMouseEventScrollBar(const SDL_Event& event, const UiScrollBar* uiSb)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return true;

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

static bool HandleMouseEvent(const SDL_Event& event, UiItemBase* item)
{
	if ((item->m_iFlags & (UIS_HIDDEN | UIS_DISABLED)) || !IsInsideRect(event, item->m_rect))
		return false;
	switch (item->m_type) {
	case UI_TXT_BUTTON:
		return HandleMouseEventArtTextButton(event, static_cast<UiTxtButton*>(item));
	case UI_BUTTON:
		return HandleMouseEventButton(event, static_cast<UiButton*>(item));
	case UI_LIST:
		return HandleMouseEventList(event, static_cast<UiList*>(item));
	case UI_SCROLLBAR:
		return HandleMouseEventScrollBar(event, static_cast<UiScrollBar*>(item));
	default:
		return false;
	}
}

static void UiHandleEvents(SDL_Event* event)
{
#if HAS_TOUCHPAD
	handle_touch(event);
#endif

	if (gUiEditField != NULL) {
		switch (event->type) {
		case SDL_KEYDOWN: {
			switch (event->key.keysym.sym) {
#ifndef USE_SDL1
			case SDLK_v:
				if (event->key.keysym.mod & KMOD_CTRL) {
					char* clipboard = SDL_GetClipboardText();
					if (clipboard != NULL) {
						UiCatToText(clipboard);
						SDL_free(clipboard);
					}
				}
				break;
#endif
			case SDLK_BACKSPACE: {
				unsigned i = gUiEditField->m_curpos;
				if (i > 0) {
					i--;
					gUiEditField->m_curpos = i;
					for ( ; ; i++) {
						// assert(gUiEditField->m_max_length != 0);
						if (gUiEditField->m_value[i] == '\0' || i >= gUiEditField->m_max_length - 1) {
							gUiEditField->m_value[i] = '\0';
							break;
						} else {
							gUiEditField->m_value[i] = gUiEditField->m_value[i + 1];
						}
					}
				}
			} break;
			case SDLK_DELETE: {
				for (unsigned i = gUiEditField->m_curpos; ; i++) {
					// assert(gUiEditField->m_max_length != 0);
					if (gUiEditField->m_value[i] == '\0' || i >= gUiEditField->m_max_length - 1) {
						gUiEditField->m_value[i] = '\0';
						break;
					} else {
						gUiEditField->m_value[i] = gUiEditField->m_value[i + 1];
					}
				}
			} break;
			case SDLK_LEFT: {
				unsigned pos = gUiEditField->m_curpos;
				if (pos > 0) {
					gUiEditField->m_curpos = pos - 1;
				}
			} break;
			case SDLK_RIGHT: {
				unsigned pos = gUiEditField->m_curpos;
				if (gUiEditField->m_value[pos] != '\0' && pos + 1 < gUiEditField->m_max_length) {
					gUiEditField->m_curpos = pos + 1;
				}
			} break;
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				UiFocusNavigationSelect();
				break;
			default:
#ifdef USE_SDL1
				if ((event->key.keysym.mod & KMOD_CTRL) == 0) {
					Uint16 unicode = event->key.keysym.unicode;
					if (unicode && (unicode & 0xFF80) == 0) {
						char utf8[2];
						utf8[0] = (char)unicode;
						utf8[1] = '\0';
						UiCatToText(utf8);
					}
				}
#endif
				break;
			}
			return;
		}
#ifndef USE_SDL1
		case SDL_TEXTINPUT:
#ifdef __vita__
			UiSetText(event->text.text);
#else
			UiCatToText(event->text.text);
#endif
			return;
#endif
		default:
			break;
		}
	}

	if (HandleMenuAction(GetMenuAction(*event)))
		return;

	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
		if (event->type == SDL_MOUSEBUTTONDOWN && !gUiDrawCursor) {
			UiFocusNavigationEsc();
			return;
		}
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
				UiItemBase* item = gUiItems[i];
				if (item->m_type == UI_BUTTON)
					static_cast<UiButton*>(item)->m_pressed = false;
			}
		}
		return; // handled
	}

	if (event->type == SDL_MOUSEMOTION) {
		// In SDL2 mouse events already use logical coordinates
#ifdef USE_SDL1
		OutputToLogical(&event->motion.x, &event->motion.y);
#endif
		MousePos.x = event->motion.x;
		MousePos.y = event->motion.y;
		return;
	}

	if (event->type == SDL_KEYDOWN) {
		if (event->key.keysym.sym == SDLK_RETURN && (event->key.keysym.mod & KMOD_ALT)) {
			ToggleFullscreen();
		} else if (!gUiDrawCursor) {
			UiFocusNavigationEsc();
		}
		return;
	}

	if (event->type == SDL_QUIT) {
		diablo_quit(EX_OK);
		return;
	}

#ifndef USE_SDL1
	if (event->type == SDL_WINDOWEVENT) {
		if (event->window.event == SDL_WINDOWEVENT_SHOWN || event->window.event == SDL_WINDOWEVENT_EXPOSED || event->window.event == SDL_WINDOWEVENT_RESTORED)
			gbWndActive = true;
		else if (event->window.event == SDL_WINDOWEVENT_HIDDEN || event->window.event == SDL_WINDOWEVENT_MINIMIZED)
			gbWndActive = false;
		return;
	}
#endif // !USE_SDL1
}

bool UiPeekAndHandleEvents(SDL_Event* event)
{
	if (SDL_PollEvent(event) == 0) {
		return false;
	}
	UiHandleEvents(event);
	return true;
}

void UiRenderItems(const std::vector<UiItemBase*>& uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++)
		RenderItem(uiItems[i]);
}

void UiClearItems(std::vector<UiItemBase*>& uiItems)
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

DEVILUTION_END_NAMESPACE
