#pragma once

//#include <cstddef>
//#include <cstdint>
#include <vector>

/*#include "../defs.h"
#include "../enums.h"
#include "../structs.h"
#include "../appfat.h"*/
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

enum UiType : uint8_t {
	UI_TEXT,
#if FULL_UI
	UI_TXT_BUTTON,
	UI_TXT_SCROLL,
#endif
	UI_IMAGE,
	UI_BUTTON,
	UI_LIST,
	UI_PROGRESSBAR,
#if FULL_UI
	UI_SCROLLBAR,
	UI_EDIT,
#endif
	UI_CUSTOM,
};

enum UiAlignment {
	UIA_LEFT,
	UIA_CENTER,
	UIA_RIGHT,
};

enum UiFlags : uint16_t {
	// clang-format off
	UIS_SMALL       = AFT_SMALL << 0,
	UIS_MED         = AFT_MED << 0,
	UIS_BIG         = AFT_BIG << 0,
	UIS_HUGE        = AFT_HUGE << 0,
	UIS_LEFT        = UIA_LEFT << 4,
	UIS_HCENTER     = UIA_CENTER << 4,
	UIS_RIGHT       = UIA_RIGHT << 4,
	UIS_VCENTER     = 1 << 6,
	UIS_SILVER      = AFC_SILVER << 7,
	UIS_GOLD        = AFC_GOLD << 7,
	UIS_LIGHT       = 1 << 8,
	UIS_DISABLED    = 1 << 12,
	UIS_HIDDEN      = 1 << 13,

	UIS_SIZE = 7 << 0,
	UIS_XALIGN = 3 << 4,
	UIS_COLOR = 1 << 7,
	// clang-format on
};

class UiItemBase {
public:
	UiItemBase(UiType type, const SDL_Rect& rect, int flags)
		: m_type(type), m_rect(rect), m_iFlags(flags)
	{
	}

	virtual ~UiItemBase() = default;

	//protected:
	UiType m_type;
	SDL_Rect m_rect;
	int m_iFlags;
};

//=============================================================================

class UiImage : public UiItemBase {
public:
	UiImage(CelImageBuf* celData, int frame, const SDL_Rect& rect, bool animated)
	    : UiItemBase(UI_IMAGE, rect, 0), m_cel_data(celData), m_frame(frame), m_animated(animated)
	{
	}

	~UiImage() = default;

	//private:
	CelImageBuf* m_cel_data;
	int m_frame;
	bool m_animated;
};

//=============================================================================

class UiText : public UiItemBase {
public:
	UiText(const char* text, const SDL_Rect& rect, int flags)
	    : UiItemBase(UI_TEXT, rect, flags), m_text(text)
	{
	}

	~UiText() = default;

	//private:
	const char* m_text;
};

//=============================================================================

class UiProgressBar : public UiItemBase {
public:
	UiProgressBar(const SDL_Rect& rect);

	~UiProgressBar();

	//private:
	CelImageBuf* m_ProgBackCel;
	CelImageBuf* m_ProgEmptyCel;
	BYTE* m_ProgFillBmp;
	int m_Progress;
};

//=============================================================================
#if FULL_UI
class UiScrollBar : public UiItemBase {
public:
	UiScrollBar(const SDL_Rect& rect)
	    : UiItemBase(UI_SCROLLBAR, rect, 0)
	{
	}

	~UiScrollBar() = default;
};

//=============================================================================

class UiTxtButton : public UiItemBase {
public:
	UiTxtButton(const char* text, void (*action)(), const SDL_Rect& rect, int flags)
	    : UiItemBase(UI_TXT_BUTTON, rect, flags), m_text(text), m_action(action)
	{
	}

	~UiTxtButton() = default;

	//private:
	const char* m_text;
	void (*m_action)();
};

//=============================================================================

class UiTextScroll : public UiItemBase {
public:
	UiTextScroll(const char* name, int lines, Uint32 ticks_begin, void (*drawFn)(const UiItemBase* _this), const SDL_Rect& rect)
	    : UiItemBase(UI_TXT_SCROLL, rect, 0), m_ticks_begin(ticks_begin), m_draw(drawFn)
	{
		m_text = LoadTxtFile(name, lines);
	}

	~UiTextScroll() {
		MemFreeTxtFile(m_text);
	}

	//private:
	Uint32 m_ticks_begin;
	char** m_text;
	void (*m_draw)(const UiItemBase* _this);
};

//=============================================================================
// maximum length of the string (with the null-terminating character) in the text-box
#define UIEDIT_MAXLENGTH 32
class UiEdit : public UiItemBase {
public:
	UiEdit(const char* hint, char* value, unsigned max_length, const SDL_Rect& rect)
	    : UiItemBase(UI_EDIT, rect, 0)
	{
		// assert(max_length <= UIEDIT_MAXLENGTH);
#if defined(__SWITCH__) || defined(__vita__) || defined(__3DS__)
		m_hint = hint;
#endif
		m_value = value;
		m_max_length = max_length;
		m_curpos = (unsigned)strlen(value);
		m_selpos = m_curpos;
		m_selecting = false;
	}

	~UiEdit() = default;

	//private:
#if defined(__SWITCH__) || defined(__vita__) || defined(__3DS__)
	const char* m_hint;
#endif
	char* m_value;
	unsigned m_max_length;
	unsigned m_curpos;
	unsigned m_selpos;
	// State
	bool m_selecting;
};
#endif // FULL_UI
//=============================================================================

// A button (uses Diablo sprites)

class UiButton : public UiItemBase {
public:
	UiButton(const char* text, void (*action)(), const SDL_Rect& rect)
	    : UiItemBase(UI_BUTTON, rect, 0), m_text(text), m_action(action)
	{
		m_pressed = false;
		m_image = CelLoadImage("ui_art\\smbutton.CEL", SML_BUTTON_WIDTH);
	}

	~UiButton() {
		mem_free_dbg(m_image);
	}

	//private:
	CelImageBuf* m_image;
	const char* m_text;
	void (*m_action)();

	// State
	bool m_pressed;
};

//=============================================================================

class UiListItem {
public:
	UiListItem(const char* text, int value)
		: m_text(text), m_value(value)
	{
	}

	~UiListItem() = default;

	//private:
	const char* m_text;
	int m_value;
};

class UiList : public UiItemBase {
public:
	UiList(std::vector<UiListItem*>* vItems, unsigned numItems, const SDL_Rect& rect, int flags)
	    : UiItemBase(UI_LIST, rect, flags)
	{
		m_vecItems = vItems;
		m_height = m_rect.h / numItems;
	};

	~UiList() = default;

	SDL_Rect itemRect(int i) const
	{
		SDL_Rect tmp = m_rect;
		tmp.y += m_height * i;
		tmp.h = m_height;

		return tmp;
	}

	unsigned indexAt(int y) const
	{
		assert(y >= m_rect.y);
		unsigned index = (y - m_rect.y) / m_height;
		assert(index < m_vecItems->size());
		return index;
	}

	//private:
	int m_height;
	std::vector<UiListItem*>* m_vecItems;
};

//=============================================================================

class UiCustom : public UiItemBase {
public:
	UiCustom(void (*drawFn)(), const SDL_Rect& rect)
	    : UiItemBase(UI_CUSTOM, rect, 0), m_draw(drawFn)
	{
	}

	~UiCustom() = default;

	//private:
	void (*m_draw)();
};

DEVILUTION_END_NAMESPACE
