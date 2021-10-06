#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../types.h"
#include "../appfat.h"

#include "DiabloUI/art.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/text_draw.h"

DEVILUTION_BEGIN_NAMESPACE

enum UiType : uint8_t {
	UI_ART_TEXT,
	UI_ART_TEXT_BUTTON,
	UI_IMAGE,
	UI_BUTTON,
	UI_LIST,
	UI_SCROLLBAR,
	UI_EDIT,
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
	UIS_CENTER      = UIA_CENTER << 4,
	UIS_RIGHT       = UIA_RIGHT << 4,
	UIS_VCENTER     = 1 << 6,
	UIS_SILVER      = AFC_SILVER << 7,
	UIS_GOLD        = AFC_GOLD << 7,
	UIS_DISABLED    = 1 << 12,
	UIS_HIDDEN      = 1 << 13,

	UIS_SIZE = 7 << 0,
	UIS_XALIGN = 3 << 4,
	UIS_COLOR = 1 << 7,
	// clang-format on
};

class UiItemBase {
public:
	UiItemBase(UiType type, SDL_Rect &rect, int flags)
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
	UiImage(Art* art, int frame, SDL_Rect &rect, int flags, bool animated)
	    : UiItemBase(UI_IMAGE, rect, flags), m_art(art), m_frame(frame), m_animated(animated)
	{
	}

	~UiImage() = default;

	//private:
	Art* m_art;
	int m_frame;
	bool m_animated;
};

//=============================================================================

class UiArtText : public UiItemBase {
public:
	UiArtText(const char* text, SDL_Rect &rect, int flags)
	    : UiItemBase(UI_ART_TEXT, rect, flags), m_text(text)
	{
	}

	~UiArtText() = default;

	//private:
	const char *m_text;
};

//=============================================================================

class UiScrollBar : public UiItemBase {
public:
	UiScrollBar(SDL_Rect &rect)
	    : UiItemBase(UI_SCROLLBAR, rect, 0)
	{
	}

	~UiScrollBar() = default;
};

//=============================================================================

class UiArtTextButton : public UiItemBase {
public:
	UiArtTextButton(const char* text, void (*action)(), SDL_Rect &rect, int flags)
	    : UiItemBase(UI_ART_TEXT_BUTTON, rect, flags), m_text(text), m_action(action)
	{
	}

	~UiArtTextButton() = default;

	//private:
	const char *m_text;
	void (*m_action)();
};

//=============================================================================

class UiEdit : public UiItemBase {
public:
	UiEdit(const char* hint, char* value, unsigned max_length, SDL_Rect &rect)
	    : UiItemBase(UI_EDIT, rect, 0)
	{
		m_hint = hint;
		m_value = value;
		m_max_length = max_length;
	}

	~UiEdit() = default;

	//private:
	const char *m_hint;
	char *m_value;
	unsigned m_max_length;
};

//=============================================================================

// A button (uses Diablo sprites)

class UiButton : public UiItemBase {
public:
	UiButton(const char* text, void (*action)(), SDL_Rect &rect)
	    : UiItemBase(UI_BUTTON, rect, 0)
	{
		m_text = text;
		m_action = action;
		m_pressed = false;
	}

	~UiButton() = default;

	enum FrameKey : uint8_t {
		DEFAULT,
		PRESSED,
		DISABLED
	};

	//private:

	const char *m_text;
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
	const char *m_text;
	int m_value;
};

class UiList : public UiItemBase {
public:
	UiList(std::vector<UiListItem*>* vItems, unsigned numItems, SDL_Rect &rect, int flags)
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

DEVILUTION_END_NAMESPACE
