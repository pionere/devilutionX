#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "DiabloUI/art.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/text_draw.h"

DEVILUTION_BEGIN_NAMESPACE

enum UiType : uint8_t {
	UI_TEXT,
	UI_ART_TEXT,
	UI_ART_TEXT_BUTTON,
	UI_IMAGE,
	UI_BUTTON,
	UI_LIST,
	UI_SCROLLBAR,
	UI_EDIT,
};

enum UiFlags : uint16_t {
	// clang-format off
	UIS_SMALL       = AFT_SMALL << 0,
	UIS_MED         = AFT_MED << 0,
	UIS_BIG         = AFT_BIG << 0,
	UIS_HUGE        = AFT_HUGE << 0,
	UIS_CENTER      = 1 << 4,
	UIS_RIGHT       = 1 << 5,
	UIS_VCENTER     = 1 << 6,
	UIS_SILVER      = AFC_SILVER << 7,
	UIS_GOLD        = AFC_GOLD << 7,
	UIS_DISABLED    = 1 << 12,
	UIS_HIDDEN      = 1 << 13,

	UIS_SIZE = 7 << 0,
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
	UiImage(Art* art, int frame, SDL_Rect &rect, int flags)
	    : UiItemBase(UI_IMAGE, rect, flags), m_art(art), m_frame(frame), m_animated(false)
	{
	}

	UiImage(Art* art, SDL_Rect &rect)
	    : UiItemBase(UI_IMAGE, rect, UIS_CENTER), m_art(art), m_frame(0), m_animated(true)
	{
	}

	~UiImage() = default;

	//private:
	Art *m_art;
	bool m_animated;
	int m_frame;
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

// Plain text (TTF)

class UiText : public UiItemBase {
public:
	UiText(const char* text, SDL_Color color, SDL_Rect &rect)
	    : UiItemBase(UI_TEXT, rect, 0), m_color(color), m_text(text)
	{
	}

	~UiText() = default;

	//private:
	SDL_Color m_color;
	const char *m_text;

	// State:
	TtfSurfaceCache m_render_cache;
};

//=============================================================================

// A button (uses Diablo sprites)

class UiButton : public UiItemBase {
public:
	UiButton(Art* art, const char* text, void (*action)(), SDL_Rect &rect)
	    : UiItemBase(UI_BUTTON, rect, 0)
	{
		m_art = art;
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
	Art *m_art;

	const char *m_text;
	void (*m_action)();

	// State
	bool m_pressed;
	TtfSurfaceCache m_render_cache;
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
private:
	static SDL_Rect& FormatRect(SDL_Rect &rect, std::vector<UiListItem*>* vItems) {
		rect.h *= vItems->size();
		return rect;
	}
public:
	UiList(std::vector<UiListItem *>* vItems, SDL_Rect &rect, int flags)
	    : UiItemBase(UI_LIST, FormatRect(rect, vItems), flags)
	{
		m_vecItems = vItems;
		m_height = m_rect.h / vItems->size();
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
