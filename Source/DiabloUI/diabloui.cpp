#include "diabloui.h"

#include "utils/utf8.h"

#include "controls/controller_motion.h"

#include "DiabloUI/diablo.h"
#include "DiabloUI/scrollbar.h"
#include "DiabloUI/text_draw.h"
#include "DiabloUI/dialogs.h"
#include "controls/plrctrls.h"
#include "controls/touch.h"
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"
#include "utils/screen_reader.hpp"
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

static void (*gfnListFocus)(unsigned index);
static void (*gfnListSelect)(unsigned index);
static void (*gfnListEsc)();
static void (*gfnListDelete)();
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

void UiInitScreen(unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)())
{
	gUiDrawCursor = true;
	SelectedItem = 0;
	SelectedItemMax = listSize != 0 ? listSize - 1 : 0;
	ListViewportSize = listSize;
	ListOffset = 0;
	gfnListFocus = fnFocus;
	gfnListSelect = fnSelect;
	gfnListEsc = fnEsc;
	gfnListDelete = NULL;
#if SCREEN_READER_INTEGRATION
	if (gUIListItems.size() > SelectedItem) {
		SpeakText(gUIListItems[SelectedItem]->m_text);
	}
#endif
	if (fnFocus != NULL)
		fnFocus(SelectedItem);

	gUiEditField = NULL;
#if !defined(__SWITCH__) && !defined(__vita__) && !defined(__3DS__)
	SDL_StopTextInput(); // input is enabled by default if !SDL_HasScreenKeyboardSupport
#endif
}

void UiInitScrollBar(UiScrollBar* uiSb, unsigned viewportSize, void (*fnDelete)())
{
	gfnListDelete = fnDelete;
	ListViewportSize = viewportSize;
	if (ListViewportSize > SelectedItemMax) {
		uiSb->m_iFlags |= UIS_HIDDEN;
	} else {
		uiSb->m_iFlags &= ~UIS_HIDDEN;
	}
	scrollBarState.upPressCounter = -1;
	scrollBarState.downPressCounter = -1;
}

void UiInitEdit(UiEdit* uiEdit)
{
	gUiEditField = uiEdit;
#ifdef __SWITCH__
	switch_start_text_input(uiEdit->m_hint, uiEdit->m_value, uiEdit->m_max_length);
#elif defined(__vita__)
	vita_start_text_input(uiEdit->m_hint, uiEdit->m_value, uiEdit->m_max_length);
#elif defined(__3DS__)
	ctr_vkbdInput(uiEdit->m_hint, uiEdit->m_value, uiEdit->m_value, uiEdit->m_max_length);
#else
	SDL_SetTextInputRect(&uiEdit->m_rect);
	SDL_StartTextInput();
#endif
}

static void UiPlayMoveSound()
{
	PlaySFX(IS_TITLEMOV, 1);
}

static void UiPlaySelectSound()
{
	PlaySFX(IS_TITLSLCT, 1);
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
#if SCREEN_READER_INTEGRATION
	if (gUIListItems.size() > SelectedItem) {
		SpeakText(gUIListItems[SelectedItem]->m_text);
	}
#endif
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

static void UiCatToText(const char* inBuf)
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
static void UiSetText(const char* inBuf)
{
	char* output = utf8_to_latin1(inBuf);
	SStrCopy(gUiEditField->m_value, output, gUiEditField->m_max_length);
	mem_free_dbg(output);
}
#endif
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
static void HandleMenuMove()
{
	const AxisDirection move_dir = axisDirRepeater.Get(GetLeftStickOrDpadDirection(true));
	if (move_dir.y == AxisDirectionY_UP) {
		UiFocusUp();
	} else if (move_dir.y == AxisDirectionY_DOWN) {
		UiFocusDown();
	}
}
#endif
void UiFocusNavigationSelect()
{
	if (gUiDrawCursor)
		UiPlaySelectSound();
	if (gUiEditField != NULL) {
		if (gUiEditField->m_value[0] == '\0') {
			return;
		}
		gUiEditField = NULL;
#if !defined(__SWITCH__) && !defined(__vita__) && !defined(__3DS__)
		//if (SDL_IsTextInputShown()) {
			SDL_StopTextInput();
		//	return;
		//}
#endif
	}
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

	UiPlaySelectSound();
	gfnListDelete();
}

static SDL_bool IsInsideRect(const Dvl_Event& event, const SDL_Rect& rect)
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

void UiAddBackground()
{
	assert(gbBackCel != NULL);
	SDL_Rect rect = { PANEL_LEFT, PANEL_TOP, PANEL_WIDTH, PANEL_HEIGHT };
	gUiItems.push_back(new UiImage(gbBackCel, 0, rect, false));
}

void UiAddLogo()
{
	assert(gbLogoCelSmall != NULL);
	SDL_Rect rect = { PANEL_MIDX(SMALL_LOGO_WIDTH), SMALL_LOGO_TOP, SMALL_LOGO_WIDTH, SMALL_LOGO_HEIGHT };
	gUiItems.push_back(new UiImage(gbLogoCelSmall, 15, rect, true));
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

void UiRender()
{
	UiClearScreen();
	UiRenderItems();
	UiFadeIn();
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

void UiRenderAndPoll()
{
	UiRender();

	Dvl_Event event;
	while (UiPeekAndHandleEvents(&event)) {
		;
	}
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	HandleMenuMove();
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
	DrawArtStr(button->m_text, textRect, UIS_HCENTER | UIS_VCENTER | UIS_SMALL | UIS_GOLD);
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

static void RenderItem(const UiItemBase* item)
{
	if (item->m_iFlags & UIS_HIDDEN)
		return;

	switch (item->m_type) {
	case UI_TEXT:
		Render(static_cast<const UiText*>(item));
		break;
	case UI_IMAGE:
		Render(static_cast<const UiImage*>(item));
		break;
	case UI_TXT_BUTTON:
		Render(static_cast<const UiTxtButton*>(item));
		break;
	case UI_BUTTON:
		Render(static_cast<const UiButton*>(item));
		break;
	case UI_LIST:
		Render(static_cast<const UiList*>(item));
		break;
	case UI_SCROLLBAR:
		Render(static_cast<const UiScrollBar*>(item));
		break;
	case UI_EDIT:
		Render(static_cast<const UiEdit*>(item));
		break;
	case UI_CUSTOM:
		static_cast<const UiCustom*>(item)->m_render();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static bool HandleMouseEventArtTextButton(const Dvl_Event& event, const UiTxtButton* uiButton)
{
	if (event.type != DVL_WM_LBUTTONDOWN)
		return true;
	uiButton->m_action();
	return true;
}

static bool HandleMouseEventButton(const Dvl_Event& event, UiButton* button)
{
	if (event.type == DVL_WM_LBUTTONDOWN) {
		button->m_pressed = true;
	} else if (button->m_pressed) {
		// assert(event.type == DVL_WM_LBUTTONUP);
		button->m_action();
	}
	return true;
}

static bool HandleMouseEventList(const Dvl_Event& event, UiList* uiList)
{
	if (event.type != DVL_WM_LBUTTONDOWN)
		return true;

	const unsigned index = uiList->indexAt(event.button.y) + ListOffset;

	UiFocus(index);
	UiRender();
	UiFocusNavigationSelect();

	return true;
}

static bool HandleMouseEventScrollBar(const Dvl_Event& event, const UiScrollBar* uiSb)
{
	if (event.type != DVL_WM_LBUTTONDOWN)
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

static bool HandleMouseEventEdit(const Dvl_Event& event, UiEdit* uiEdit)
{
	if (event.type != DVL_WM_LBUTTONDOWN)
		return true;

	int x = event.button.x - (uiEdit->m_rect.x + 43);
	char* text = uiEdit->m_value;
	unsigned curpos = 0;
	for ( ; ; curpos++) {
		char tmp = text[curpos];
		if (tmp == '\0') {
			break;
		}
		BYTE w = bigFontWidth[gbStdFontFrame[tmp]];
		x -= w + FONT_KERN_BIG;
		if (x <= 0) {
			if (-x < (w + FONT_KERN_BIG) / 2) {
				curpos++;
			}
			break;
		}
	}
	// assert(uiEdit->m_max_length != 0);
	if (curpos >= uiEdit->m_max_length - 1) {
		curpos = uiEdit->m_max_length - 1;
	}
	uiEdit->m_curpos = curpos;
	return true;
}

static bool HandleMouseEvent(const Dvl_Event& event, UiItemBase* item)
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
	case UI_EDIT:
		return HandleMouseEventEdit(event, static_cast<UiEdit*>(item));
	default:
		return false;
	}
}

bool UiPeekAndHandleEvents(Dvl_Event* event)
{
	if (!PeekMessage(*event)) {
		return false;
	}

	switch (event->type) {
	case DVL_WM_QUIT:
		diablo_quit(EX_OK);
		break;
	case DVL_WM_LBUTTONDOWN:
		if (!gUiDrawCursor) {
			UiFocusNavigationEsc();
			break;
		}
	case DVL_WM_LBUTTONUP:
		//bool handled = false;
		for (unsigned i = 0; i < gUiItems.size(); i++) {
			if (HandleMouseEvent(*event, gUiItems[i])) {
				//handled = true;
				break;
			}
		}

		if (event->type == DVL_WM_LBUTTONUP) {
			scrollBarState.downPressCounter = scrollBarState.upPressCounter = -1;
			for (unsigned i = 0; i < gUiItems.size(); i++) {
				UiItemBase* item = gUiItems[i];
				if (item->m_type == UI_BUTTON)
					static_cast<UiButton*>(item)->m_pressed = false;
			}
		}
		break; // handled
	case DVL_WM_RBUTTONDOWN:
		UiFocusNavigationEsc();
		break;
	case DVL_WM_KEYDOWN:
		if (!gUiDrawCursor) {
			UiFocusNavigationEsc();
			break;
		}
		if (event->vkcode == DVL_VK_RETURN && (event->key.keysym.mod & KMOD_ALT)) {
			ToggleFullscreen();
			break;
		}
		if (gUiEditField != NULL) {
			switch (event->vkcode) {
#ifndef USE_SDL1
			case DVL_VK_MBUTTON:
			case DVL_VK_V:
				if (event->key.keysym.mod & KMOD_CTRL) {
					char* clipboard = SDL_GetClipboardText();
					if (clipboard != NULL) {
						UiCatToText(clipboard);
						SDL_free(clipboard);
					}
				}
				break;
#endif
			case DVL_VK_BACK: {
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
			case DVL_VK_DELETE: {
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
			case DVL_VK_LEFT: {
				unsigned pos = gUiEditField->m_curpos;
				if (pos > 0) {
					gUiEditField->m_curpos = pos - 1;
				}
			} break;
			case DVL_VK_RIGHT: {
				unsigned pos = gUiEditField->m_curpos;
				if (gUiEditField->m_value[pos] != '\0' && pos + 1 < gUiEditField->m_max_length) {
					gUiEditField->m_curpos = pos + 1;
				}
			} break;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
			case DVL_VK_LBUTTON:
#endif
			case DVL_VK_RETURN:
				UiFocusNavigationSelect();
				break;
			case DVL_VK_ESCAPE:
				UiFocusNavigationEsc();
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
			break; // gUiEditField != NULL
		}
		// TODO: use something like WMButtonInputTransTbl?
		switch (event->vkcode) {
		case DVL_VK_RETURN:
		case DVL_VK_RIGHT:
			UiFocusNavigationSelect();
			break;
		case DVL_VK_XBUTTON1:
		case DVL_VK_ESCAPE:
		case DVL_VK_LEFT:
			UiFocusNavigationEsc();
			break;
		// case DVL_VK_OEM_PLUS:
		case DVL_VK_UP:
			UiFocusUp();
			break;
		// case DVL_VK_OEM_MINUS:
		case DVL_VK_DOWN:
			UiFocusDown();
			break;
		case DVL_VK_DELETE:
			UiFocusNavigationDelete();
			break;
		case DVL_VK_PRIOR:
			UiFocusPageUp();
			break;
		case DVL_VK_NEXT:
			UiFocusPageDown();
			break;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		case DVL_VK_LBUTTON:
			UiFocusNavigationSelect();
			break;
#endif
		}
		break;
#ifndef USE_SDL1
	case DVL_WM_TEXT:
		if (gUiEditField != NULL) {
#ifdef __vita__
			UiSetText(event->text.text);
#else
			UiCatToText(event->text.text);
#endif
		}
		break;
#endif
	}
	return true;
}

void UiRenderItems()
{
	for (const UiItemBase* uiItem : gUiItems) {
		RenderItem(uiItem);
	}
}

void UiClearItems()
{
	for (UiItemBase* uiItem : gUiItems) {
		delete uiItem;
	}
	gUiItems.clear();
}

void UiClearListItems()
{
	for (UiListItem* uiItem : gUIListItems) {
		delete uiItem;
	}
	gUIListItems.clear();
}

DEVILUTION_END_NAMESPACE
