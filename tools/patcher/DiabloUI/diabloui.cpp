#include "diabloui.h"

#include "controls/controller_motion.h"

#include "DiabloUI/diablo.h"
//#include "controls/plrctrls.h"
#include "controls/touch.h"
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"
#include "utils/screen_reader.hpp"
#if FULL_UI
#include "engine/render/raw_render.h"
#include "utils/utf8.h"
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
#endif // FULL_UI
DEVILUTION_BEGIN_NAMESPACE

#define FOCUS_FRAME_COUNT   8
#define EDIT_SELECTOR_WIDTH 43

static CelImageBuf* gbBackCel;
static CelImageBuf* gbLogoCelSmall;
static CelImageBuf* gbFocusCelSmall;
static CelImageBuf* gbFocusCelMed;
static CelImageBuf* gbFocusCelBig;
#if FULL_UI
static CelImageBuf* scrollBarBackCel;
static CelImageBuf* scrollBarThumbCel;
static CelImageBuf* scrollBarArrowCel;
#endif

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
#if FULL_UI
/** The edit field on the current screen (if exists) */
UiEdit* gUiEditField;
#endif
/** Specifies whether the cursor should be shown on the current screen + controlls key/mouse-press events if set to false. TODO: better solution? */
bool gUiDrawCursor;

static Uint32 _gdwFadeTc;

UiProgressBar::UiProgressBar(const SDL_Rect& rect)
	: UiItemBase(UI_PROGRESSBAR, rect, 0)//, m_Progress(0)
{
	int i;
	CelImageBuf* progFillCel;

	m_ProgBackCel = CelLoadImage("ui_art\\spopup.CEL", SMALL_POPUP_WIDTH);
	m_ProgEmptyCel = CelLoadImage("ui_art\\prog_bg.CEL", PRBAR_WIDTH);

	m_ProgFillBmp = DiabloAllocPtr(PRBAR_HEIGHT * PRBAR_WIDTH);
	progFillCel = CelLoadImage("ui_art\\prog_fil.CEL", PRBAR_WIDTH);
	CelDraw(SCREEN_X, SCREEN_Y + PRBAR_HEIGHT - 1, progFillCel, 1);
	for (i = 0; i < PRBAR_HEIGHT; i++) {
		memcpy(&m_ProgFillBmp[0 + i * PRBAR_WIDTH], &gpBuffer[SCREENXY(0, i)], PRBAR_WIDTH);
	}
	mem_free_dbg(progFillCel);
}

UiProgressBar::~UiProgressBar()
{
	MemFreeDbg(m_ProgBackCel);
	MemFreeDbg(m_ProgEmptyCel);
	MemFreeDbg(m_ProgFillBmp);
}

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
	if (fnFocus != NULL) {
		fnFocus(SelectedItem);
#if SCREEN_READER_INTEGRATION
		unsigned idx = SelectedItem - ListOffset;
		SpeakText(gUIListItems[idx]->m_text);
#endif
	}

#if FULL_UI
	gUiEditField = NULL;
#endif // FULL_UI
#if !defined(__SWITCH__) && !defined(__vita__) && !defined(__3DS__)
	SDL_StopTextInput(); // input is enabled by default if !SDL_HasScreenKeyboardSupport
#endif
}
#if FULL_UI
void UiInitScrollBar(UiScrollBar* uiSb, unsigned viewportSize, void (*fnDelete)())
{
	gfnListDelete = fnDelete;
	ListViewportSize = viewportSize;
	if (ListViewportSize > SelectedItemMax) {
		uiSb->m_iFlags |= UIS_HIDDEN;
	//} else {
	//	uiSb->m_iFlags &= ~UIS_HIDDEN;
	}
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
#endif // FULL_UI
static void UiPlayMoveSound()
{
	PlaySfx(IS_TITLEMOV);
}

static void UiPlaySelectSound()
{
	PlaySfx(IS_TITLSLCT);
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

void UiFocus(unsigned itemIndex)
{
	if (SelectedItem == itemIndex)
		return;

	SelectedItem = itemIndex;

	UiScrollIntoView();
	UiPlayMoveSound();

	if (gfnListFocus != NULL) {
		gfnListFocus(itemIndex);
#if SCREEN_READER_INTEGRATION
		unsigned idx = itemIndex - ListOffset;
		SpeakText(gUIListItems[idx]->m_text);
#endif
	}
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
#if FULL_UI
#ifndef USE_SDL1
static bool UiCopyToClipboard()
{
	unsigned sp = gUiEditField->m_selpos;
	unsigned cp = gUiEditField->m_curpos;
	if (sp == cp) {
		return false;
	}
	if (sp > cp) {
		std::swap(sp, cp);
	}
	char tmp = gUiEditField->m_value[cp];
	gUiEditField->m_value[cp] = '\0';
	char* output = latin1_to_utf8(&gUiEditField->m_value[sp]);
	gUiEditField->m_value[cp] = tmp;
	SDL_SetClipboardText(output);
	SDL_free(output);
	return true;
}
#endif
static void UiCatToText(const char* inBuf)
{
	char* output = utf8_to_latin1(inBuf);
	unsigned cp = gUiEditField->m_curpos;
	unsigned sp = gUiEditField->m_selpos;
	if (sp > cp) {
		std::swap(sp, cp);
	}
	char* text = gUiEditField->m_value;
	unsigned maxlen = gUiEditField->m_max_length;
	char tmpstr[UIEDIT_MAXLENGTH];
	SStrCopy(tmpstr, &text[cp], std::min((unsigned)sizeof(tmpstr) - 1, maxlen - cp));
	int len = SStrCopy(&text[sp], output, maxlen - sp);
	SDL_free(output);
	// assert(strlen(text) == len + sp);
	sp += len;
	gUiEditField->m_curpos = sp;
	gUiEditField->m_selpos = sp;
	SStrCopy(&text[sp], tmpstr, maxlen - sp);
}

#ifdef __vita__
static void UiSetText(const char* inBuf)
{
	char* output = utf8_to_latin1(inBuf);
	char* text = gUiEditField->m_value;
	int len = SStrCopy(text, output, gUiEditField->m_max_length);
	SDL_free(output);
	// assert(strlen(text) == len);
	unsigned pos = (unsigned)len;
	gUiEditField->m_curpos = pos;
	gUiEditField->m_selpos = pos;
}
#endif
#endif // FULL_UI
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
#if FULL_UI
	if (gUiEditField != NULL) {
		if (gUiEditField->m_value[0] == '\0') {
			return;
		}
		gUiEditField = NULL;
#if !defined(__SWITCH__) && !defined(__vita__) && !defined(__3DS__)
		//if (SDL_IsTextInputShown()) {
			SDL_StopTextInput();
		//}
#endif
	}
#endif // FULL_UI
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
#if FULL_UI
	assert(scrollBarBackCel == NULL);
	scrollBarBackCel = CelLoadImage("ui_art\\sb_bg.CEL", SCROLLBAR_BG_WIDTH);
	assert(scrollBarThumbCel == NULL);
	scrollBarThumbCel = CelLoadImage("ui_art\\sb_thumb.CEL", SCROLLBAR_THUMB_WIDTH);
	assert(scrollBarArrowCel == NULL);
	scrollBarArrowCel = CelLoadImage("ui_art\\sb_arrow.CEL", SCROLLBAR_ARROW_WIDTH);
#endif
	NewCursor(CURSOR_HAND);
}

static void UnloadUiGFX()
{
	MemFreeDbg(gbLogoCelSmall);
	MemFreeDbg(gbFocusCelSmall);
	MemFreeDbg(gbFocusCelMed);
	MemFreeDbg(gbFocusCelBig);
#if FULL_UI
	MemFreeDbg(scrollBarBackCel);
	MemFreeDbg(scrollBarThumbCel);
	MemFreeDbg(scrollBarArrowCel);
#endif
}

void UiInitialize()
{
	LoadUiGFX();
}

void UiDestroy()
{
	UnloadUiGFX();
}

void LoadBackgroundArt(const char* pszFile, const char* palette)
{
	assert(gbBackCel == NULL);
	// FreeBackgroundArt();
	if (pszFile != NULL)
		gbBackCel = CelLoadImage(pszFile, BACKGROUND_ART_WIDTH);
	// assert(palette != NULL);
	LoadPalette(palette);

	// initiate fading
	gnFadeValue = -1;
}

void FreeBackgroundArt()
{
	MemFreeDbg(gbBackCel);
}

void UiAddBackground()
{
	assert(gbBackCel != NULL);
	SDL_Rect rect = { SCREEN_MIDX(BACKGROUND_ART_WIDTH), BACKGROUND_ART_TOP, BACKGROUND_ART_WIDTH, BACKGROUND_ART_HEIGHT };
	gUiItems.push_back(new UiImage(gbBackCel, 0, rect, false));
}

void UiAddLogo()
{
	assert(gbLogoCelSmall != NULL);
	SDL_Rect rect = { SCREEN_MIDX(SMALL_LOGO_WIDTH), SMALL_LOGO_TOP, SMALL_LOGO_WIDTH, SMALL_LOGO_HEIGHT };
	gUiItems.push_back(new UiImage(gbLogoCelSmall, 15, rect, true));
}

static void UiClearScreen()
{
	// if (SCREEN_WIDTH > BACKGROUND_ART_WIDTH || SCREEN_HEIGHT > BACKGROUND_ART_HEIGHT) { // Background size
		ClearScreenBuffer();
	// }
}

static void UiFadeIn()
{
	int fv;
	Uint32 currTc;
	bool draw_cursor;

	fv = gnFadeValue;
	if (fv < FADE_LEVELS) {
		currTc = SDL_GetTicks();
		if (fv < 0) {
			_gdwFadeTc = currTc;
			PaletteFadeIn(true);
		}
		fv = (currTc - _gdwFadeTc) >> 0; // instead of >> 0 it was / 2.083 ... 32 frames @ 60hz
		if ((unsigned)fv > FADE_LEVELS) {
			fv = FADE_LEVELS;
			//_gdwFadeTc = 0;
		}
		SetFadeLevel(fv);
	}

	draw_cursor = gUiDrawCursor;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (sgbControllerActive)
		draw_cursor = false;
#endif
	scrollrt_render_screen(draw_cursor);
}

static int GetAnimationFrame(int frames, int animFrameLenMs)
{
	//assert(frames > 0 && fps > 0);
	return (SDL_GetTicks() / animFrameLenMs) % frames;
}

static void UiDrawSelector(const SDL_Rect& rect)
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
	frame = GetAnimationFrame(FOCUS_FRAME_COUNT, 64) + 1;
	x = SCREEN_X + rect.x;
	y = SCREEN_Y + rect.y + (unsigned)(rect.h + size) / 2 - 1; // TODO FOCUS_MED appears higher than the box

	CelDraw(x, y, selCel, frame);
	x += rect.w - size;
	CelDraw(x, y, selCel, frame);
}

static void DrawArtStr(const char* text, const SDL_Rect& rect, int flags)
{
	flags &= ~(UIS_OPTIONAL | UIS_DISABLED | UIS_HIDDEN),
	PrintString(flags, text, SCREEN_X + rect.x, SCREEN_Y + rect.y, rect.w, rect.h);
}

static void UiDraw(const UiText* uiArtText)
{
	DrawArtStr(uiArtText->m_text, uiArtText->m_rect, uiArtText->m_iFlags);
}
#if FULL_UI
static void UiDraw(const UiTextBox* uiTextBox)
{
	DrawColorTextBox(SCREEN_X + uiTextBox->m_rect.x, SCREEN_Y + uiTextBox->m_rect.y, uiTextBox->m_rect.w, uiTextBox->m_rect.h, (uiTextBox->m_iFlags >> UIS_COLOR_SHL) & UIS_COLORS);

	if (uiTextBox->m_iFlags & UIS_HCENTER)
		DrawColorTextBoxSLine(SCREEN_X + uiTextBox->m_rect.x, SCREEN_Y + uiTextBox->m_rect.y, uiTextBox->m_rect.w, SELGAME_HEADER_HEIGHT);
}
#endif
static void UiDraw(const UiImage* uiImage)
{
	int frame = uiImage->m_animated ? GetAnimationFrame(uiImage->m_frame, 64) : uiImage->m_frame;
	int x = SCREEN_X + uiImage->m_rect.x;
	int y = SCREEN_Y + uiImage->m_rect.y + uiImage->m_rect.h - 1;

	CelDraw(x, y, uiImage->m_cel_data, frame + 1);
}

static void UiDrawStr(const char* text, const SDL_Rect& rect, int flags)
{
	const SDL_Point point = { MousePos.x, MousePos.y };
	if (SDL_PointInRect(&point, &rect)) {
		flags |= UIS_LIGHT;
#if SCREEN_READER_INTEGRATION
		SpeakText(text);
#endif
	}
	DrawArtStr(text, rect, flags);
}
#if FULL_UI
static void UiDraw(const UiTxtButton* uiButton)
{
	UiDrawStr(uiButton->m_text, uiButton->m_rect, uiButton->m_iFlags);
}

static void UiDraw(const UiTextScroll* uiTxtScroll)
{
	uiTxtScroll->m_draw(uiTxtScroll);
}
#endif
static void UiDraw(const UiButton* button)
{
	int frame = button->m_pressed ? 2 : 1;
	int x = SCREEN_X + button->m_rect.x;
	int y = SCREEN_Y + button->m_rect.y + 28 - 1;

	CelDraw(x, y, button->m_image, frame);

	SDL_Rect textRect = button->m_rect;
	if (button->m_pressed)
		textRect.y++;
	DrawArtStr(button->m_text, textRect, UIS_HCENTER | UIS_VCENTER | UIS_SMALL | UIS_GOLD);
}

static void UiDraw(const UiList* uiList)
{
	for (unsigned i = 0; i < uiList->m_vecItems->size(); i++) {
		SDL_Rect rect = uiList->itemRect(i);
		if (i + ListOffset == SelectedItem)
			UiDrawSelector(rect);
		UiListItem* item = (*uiList->m_vecItems)[i];
		UiDrawStr(item->m_text, rect, uiList->m_iFlags);
	}
}

static void UiDraw(const UiProgressBar* uiPb)
{
	int x, y, i, dx;

	x = uiPb->m_rect.x + SCREEN_X;
	y = uiPb->m_rect.y + SCREEN_Y;
	// draw the popup window
	CelDraw(x, y + SMALL_POPUP_HEIGHT, uiPb->m_ProgBackCel, 1);
	x += (SMALL_POPUP_WIDTH - PRBAR_WIDTH) / 2;
	y += 46 + PRBAR_HEIGHT;
	// draw the frame of the progress bar
	CelDraw(x, y - 1, uiPb->m_ProgEmptyCel, 1);
	// draw the progress bar
	dx = uiPb->m_Progress;
	if (dx > 100)
		dx = 100;
	dx = PRBAR_WIDTH * dx / 100;
	for (i = 0; i < PRBAR_HEIGHT && dx != 0; i++) {
		memcpy(&gpBuffer[BUFFERXY(x, y + i - PRBAR_HEIGHT)], &uiPb->m_ProgFillBmp[0 + i * PRBAR_WIDTH], dx);
	}
}
#if FULL_UI
static void UiDraw(const UiScrollBar* uiSb)
{
	const int sx = SCREEN_X + uiSb->m_rect.x;
	int sy = SCREEN_Y + uiSb->m_rect.y - 1;
	// Up Arrow:
	{
		const int frame = uiSb->m_pressMode == 1 ? 1 : 2;
		// assert(scrollBarArrowCel != NULL);
		sy += SCROLLBAR_ARROW_HEIGHT;
		CelDraw(sx, sy, scrollBarArrowCel, frame);
	}
	// Bar background (tiled):
	{
		int bgYEnd = sy + uiSb->m_rect.h - 2 * SCROLLBAR_ARROW_HEIGHT;
		// assert(uiSb->m_rect.h - 2 * SCROLLBAR_ARROW_HEIGHT >= SCROLLBAR_BG_HEIGHT);
		// assert(scrollBarBackCel != NULL);
		const int frame = 1;
		do {
			sy += SCROLLBAR_BG_HEIGHT;
			if (bgYEnd < sy)
				sy = bgYEnd;
			CelDraw(sx, sy, scrollBarBackCel, frame);
		} while (sy < bgYEnd);
	}
	// Down Arrow:
	{
		// assert(scrollBarArrowCel != NULL);
		const int frame = uiSb->m_pressMode == 2 ? 3 : 4;
		sy += SCROLLBAR_ARROW_HEIGHT;
		CelDraw(sx, sy, scrollBarArrowCel, frame);
	}
	// Thumb:
	{
		const int frame = 1;
		const int thumb_max_y = uiSb->m_rect.h - (2 * SCROLLBAR_ARROW_HEIGHT + SCROLLBAR_THUMB_HEIGHT);
		const int thumb_y = (SelectedItemMax - SelectedItem) * thumb_max_y / SelectedItemMax;
		sy -= thumb_y + SCROLLBAR_ARROW_HEIGHT;

		// assert(scrollBarThumbCel != NULL);
		CelDraw(sx + SCROLLBAR_THUMB_OFFSET_X, sy, scrollBarThumbCel, frame);
	}
}

static void UiDraw(const UiEdit* uiEdit)
{
	// UiDrawSelector(uiEdit->m_rect);
	SDL_Rect rect = uiEdit->m_rect;
	// rect.x += EDIT_SELECTOR_WIDTH;
	rect.y += 1;
	// rect.w -= 2 * EDIT_SELECTOR_WIDTH;
	char* text = uiEdit->m_value;
	// render the text
	DrawArtStr(text, rect, UIS_LEFT | UIS_MED | UIS_GOLD);
	{   // render the selection
		unsigned curpos = uiEdit->m_curpos;
		unsigned selpos = uiEdit->m_selpos;
		if (selpos != curpos) {
			int sp, w;
			if (selpos > curpos) {
				std::swap(selpos, curpos);
			}
			char tmp = text[selpos];
			text[selpos] = '\0';
			sp = GetBigStringWidth(text);
			text[selpos] = tmp;
			tmp = text[curpos];
			text[curpos] = '\0';
			w = GetBigStringWidth(&text[selpos]);
			text[curpos] = tmp;
			int h = BIG_FONT_HEIGHT;
			DrawRectTrans(SCREEN_X + rect.x + sp + FONT_KERN_BIG, SCREEN_Y + rect.y, w, h, PAL16_GRAY);
		}
	}
	// render the cursor
	if (GetAnimationFrame(2, 512) != 0) {
		unsigned curpos = uiEdit->m_curpos;
		char tmp = text[curpos];
		text[curpos] = '\0';
		int w = GetBigStringWidth(text);
		int h = BIG_FONT_HEIGHT;
		text[curpos] = tmp;
		PrintBigChar(SCREEN_X + rect.x + w, SCREEN_Y + rect.y + h, '|', COL_WHITE);
	}
}
#endif
static void UiDrawItem(const UiItemBase* item)
{
	if (item->m_iFlags & UIS_HIDDEN)
		return;

	switch (item->m_type) {
	case UI_TEXT:
		UiDraw(static_cast<const UiText*>(item));
		break;
#if FULL_UI
	case UI_TEXTBOX:
		UiDraw(static_cast<const UiTextBox*>(item));
		break;
#endif
	case UI_IMAGE:
		UiDraw(static_cast<const UiImage*>(item));
		break;
#if FULL_UI
	case UI_TXT_BUTTON:
		UiDraw(static_cast<const UiTxtButton*>(item));
		break;
	case UI_TXT_SCROLL:
		UiDraw(static_cast<const UiTextScroll*>(item));
		break;
#endif
	case UI_BUTTON:
		UiDraw(static_cast<const UiButton*>(item));
		break;
	case UI_LIST:
		UiDraw(static_cast<const UiList*>(item));
		break;
	case UI_PROGRESSBAR:
		UiDraw(static_cast<const UiProgressBar*>(item));
		break;
#if FULL_UI
	case UI_SCROLLBAR:
		UiDraw(static_cast<const UiScrollBar*>(item));
		break;
	case UI_EDIT:
		UiDraw(static_cast<const UiEdit*>(item));
		break;
#endif
	case UI_CUSTOM:
		static_cast<const UiCustom*>(item)->m_draw();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void UiDrawItems()
{
	for (const UiItemBase* uiItem : gUiItems) {
		UiDrawItem(uiItem);
	}
}

void UiRender()
{
	if (gbWndActive) {
		UiClearScreen();
		UiDrawItems();

#if HAS_TOUCHPAD
		DrawGamepad();
#endif
	}
	UiFadeIn();
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
#if FULL_UI
#ifdef __3DS__
	// Keyboard blocks until input is finished
	// so defer until after render and fade-in
	ctr_vkbdFlush();
#endif
#endif // FULL_UI
}
#if FULL_UI
static bool HandleMouseEventArtTextButton(const Dvl_Event& event, const UiTxtButton* uiButton)
{
	if (event.type != DVL_WM_LBUTTONDOWN)
		return true;
	uiButton->m_action();
	return true;
}
#endif
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
#if FULL_UI
static bool HandleMouseEventScrollBar(const Dvl_Event& event, UiScrollBar* uiSb)
{
	if (event.type != DVL_WM_LBUTTONDOWN)
		return true;

	int y = event.button.y - (uiSb->m_rect.y + SCROLLBAR_ARROW_HEIGHT);
	int mode = 0;
	if (y < 0) {
		// up arrow
		mode = 1;
		UiFocusUp();
	} else if (y >= uiSb->m_rect.h - 2 * SCROLLBAR_ARROW_HEIGHT) {
		// down arrow
		mode = 2;
		UiFocusDown();
	} else {
		// Scroll up or down based on thumb position.
		const int thumb_max_y = uiSb->m_rect.h - (2 * SCROLLBAR_ARROW_HEIGHT + SCROLLBAR_THUMB_HEIGHT);
		const int thumb_y = SelectedItem * thumb_max_y / SelectedItemMax;
		if (y < thumb_y) {
			UiFocusPageUp();
		} else if (y >= thumb_y + SCROLLBAR_THUMB_HEIGHT) {
			UiFocusPageDown();
		}
	}
	uiSb->m_pressMode = mode;
	return true;
}

static unsigned EditCursPos(int x, UiEdit* uiEdit)
{
	x -= (uiEdit->m_rect.x /* + EDIT_SELECTOR_WIDTH */);
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
	return curpos;
}

static bool HandleMouseEventEdit(const Dvl_Event& event, UiEdit* uiEdit)
{
	uiEdit->m_selecting = event.type == DVL_WM_LBUTTONDOWN;
	if (uiEdit->m_selecting) {
		unsigned curpos = EditCursPos(event.button.x, uiEdit);
		uiEdit->m_curpos = curpos;
		if (!(SDL_GetModState() & KMOD_SHIFT)) {
			uiEdit->m_selpos = curpos;
		}
	}
	return true;
}
#endif // FULL_UI
static void HandleMouseEvent(const Dvl_Event& event)
{
	for (UiItemBase* item : gUiItems) {
		if ((item->m_iFlags & (UIS_HIDDEN | UIS_DISABLED)) || !IsInsideRect(event, item->m_rect))
			continue;
		switch (item->m_type) {
#if FULL_UI
		case UI_TXT_BUTTON:
			HandleMouseEventArtTextButton(event, static_cast<UiTxtButton*>(item));
			break;
#endif
		case UI_BUTTON:
			HandleMouseEventButton(event, static_cast<UiButton*>(item));
			break;
		case UI_LIST:
			HandleMouseEventList(event, static_cast<UiList*>(item));
			break;
#if FULL_UI
		case UI_SCROLLBAR:
			HandleMouseEventScrollBar(event, static_cast<UiScrollBar*>(item));
			break;
		case UI_EDIT:
			HandleMouseEventEdit(event, static_cast<UiEdit*>(item));
			break;
#endif
		default:
			continue;
		}
		return;
	}
}
#if FULL_UI
static bool HandleMouseMoveEventEdit(const Dvl_Event& event, UiEdit* uiEdit)
{
	if (uiEdit->m_selecting) {
		unsigned curpos = EditCursPos(event.motion.x, uiEdit);
		uiEdit->m_curpos = curpos;
	}
	return true;
}

static void HandleMouseMoveEvent(const Dvl_Event& event)
{
	/*for (UiItemBase* item : gUiItems) {
		if ((item->m_iFlags & (UIS_HIDDEN | UIS_DISABLED)) || !IsInsideRect(event, item->m_rect))
			continue;
		switch (item->m_type) {
		case UI_EDIT:
			HandleMouseMoveEventEdit(event, static_cast<UiEdit*>(item));
			break;
		default:
			continue;
		}
		return;
	}*/
	if (gUiEditField != NULL) {
		HandleMouseMoveEventEdit(event, gUiEditField);
	}
}

static void UiDelFromText(bool back)
{
	char* text = gUiEditField->m_value;
	unsigned max_length = gUiEditField->m_max_length;

	int w = gUiEditField->m_curpos - gUiEditField->m_selpos;
	if (w != 0) {
		if (w < 0) {
			w = -w;
			gUiEditField->m_selpos = gUiEditField->m_curpos;
		} else {
			gUiEditField->m_curpos = gUiEditField->m_selpos;
		}
	} else {
		w = 1;
		if (back) {
			unsigned i = gUiEditField->m_curpos;
			if (i == 0) {
				return;
			}
			i--;
			gUiEditField->m_curpos = i;
			gUiEditField->m_selpos = i;
		}
	}

	for (unsigned i = gUiEditField->m_curpos; ; i++) {
		// assert(max_length != 0);
		if (text[i] == '\0' || (i + w) >= max_length) {
			text[i] = '\0';
			break;
		} else {
			text[i] = text[i + w];
		}
	}
}
#endif // FULL_UI
bool UiPeekAndHandleEvents(Dvl_Event* event)
{
	if (!PeekMessage(*event)) {
		return false;
	}

	switch (event->type) {
#if FULL_UI
	case DVL_WM_MOUSEMOVE:
		HandleMouseMoveEvent(*event);
		break;
#endif
	case DVL_WM_QUIT:
		diablo_quit(EX_OK);
		break;
	case DVL_WM_LBUTTONDOWN:
		if (!gUiDrawCursor) {
			UiFocusNavigationEsc();
			break;
		}
		HandleMouseEvent(*event);
		break;
	case DVL_WM_LBUTTONUP:
		HandleMouseEvent(*event);

		for (UiItemBase* item : gUiItems) {
			if (item->m_type == UI_BUTTON) {
				static_cast<UiButton*>(item)->m_pressed = false;
#if FULL_UI
			} else if (item->m_type == UI_SCROLLBAR) {
				static_cast<UiScrollBar*>(item)->m_pressMode = 0;
#endif
			}
		}
		break;
	case DVL_WM_RBUTTONDOWN:
		UiFocusNavigationEsc();
		break;
	case DVL_WM_KEYDOWN:
		if (!gUiDrawCursor) {
			UiFocusNavigationEsc();
			break;
		}
#if !FULLSCREEN_ONLY
		if (event->vkcode == DVL_VK_RETURN && (event->key.keysym.mod & KMOD_ALT)) {
			ToggleFullscreen();
			break;
		}
#endif
#if FULL_UI
		if (gUiEditField != NULL) {
			switch (event->vkcode) {
#ifndef USE_SDL1
			case DVL_VK_V:
				if (!(event->key.keysym.mod & KMOD_CTRL)) {
					break;
				}
				// fall-through
			case DVL_VK_MBUTTON: {
				char* clipboard = SDL_GetClipboardText();
				if (clipboard != NULL) {
					UiCatToText(clipboard);
					SDL_free(clipboard);
				}
			} break;
			case DVL_VK_C:
			case DVL_VK_X:
				if (!(event->key.keysym.mod & KMOD_CTRL)) {
					break;
				}
				if (!UiCopyToClipboard()) {
					break;
				}
				if (event->vkcode == DVL_VK_C) {
					break;
				}
				// fall-through
#endif
			case DVL_VK_BACK:
				UiDelFromText(true);
				break;
			case DVL_VK_DELETE:
				UiDelFromText(false);
				break;
			case DVL_VK_LEFT: {
				unsigned pos = gUiEditField->m_curpos;
				if (pos > 0) {
					gUiEditField->m_curpos = pos - 1;
					if (!(event->key.keysym.mod & KMOD_SHIFT)) {
						gUiEditField->m_selpos = pos - 1;
					}
				}
			} break;
			case DVL_VK_RIGHT: {
				unsigned pos = gUiEditField->m_curpos;
				if (gUiEditField->m_value[pos] != '\0' && pos + 1 < gUiEditField->m_max_length) {
					gUiEditField->m_curpos = pos + 1;
					if (!(event->key.keysym.mod & KMOD_SHIFT)) {
						gUiEditField->m_selpos = pos + 1;
					}
				}
			} break;
			case DVL_VK_HOME: {
				gUiEditField->m_curpos = 0;
				if (!(event->key.keysym.mod & KMOD_SHIFT)) {
					gUiEditField->m_selpos = 0;
				}
			} break;
			case DVL_VK_END: {
				unsigned pos = (unsigned)strlen(gUiEditField->m_value);
				gUiEditField->m_curpos = pos;
				if (!(event->key.keysym.mod & KMOD_SHIFT)) {
					gUiEditField->m_selpos = pos;
				}
			} break;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
			case DVL_VK_LBUTTON:
#endif
			case DVL_VK_RETURN:
				UiFocusNavigationSelect();
				break;
			case DVL_VK_ESCAPE:
				if (gUiEditField->m_curpos != gUiEditField->m_selpos) {
					gUiEditField->m_selpos = gUiEditField->m_curpos;
				} else {
					UiFocusNavigationEsc();
				}
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
#endif // FULL_UI
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
#if FULL_UI
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
#endif // !USE_SDL1
#endif // FULL_UI
	}
	return true;
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
