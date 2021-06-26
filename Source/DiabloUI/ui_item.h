#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "DiabloUI/art.h"
#include "DiabloUI/text_draw.h"
#include "utils/stubs.h"

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
	UIS_SMALL       = 1 << 0,
	UIS_MED         = 1 << 1,
	UIS_BIG         = 1 << 2,
	UIS_HUGE        = 1 << 3,
	UIS_CENTER      = 1 << 4,
	UIS_RIGHT       = 1 << 5,
	UIS_VCENTER     = 1 << 6,
	UIS_SILVER      = 1 << 7,
	UIS_GOLD        = 1 << 8,
	UIS_RED         = 1 << 9,
	UIS_BLUE        = 1 << 10,
	UIS_BLACK       = 1 << 11,
	UIS_DISABLED    = 1 << 12,
	UIS_HIDDEN      = 1 << 13,
	// clang-format on
};

class UiItemBase {
public:
	UiItemBase(SDL_Rect rect, int flags)
	{
		m_rect = rect;
		m_iFlags = flags;
	};

	UiItemBase(int x, int y, int item_width, int item_height, int flags)
	{
		SDL_Rect tmp;
		tmp.x = x;
		tmp.y = y;
		tmp.w = item_width;
		tmp.h = item_height;

		m_rect = tmp;
		m_iFlags = flags;
	};

	virtual ~UiItemBase() {};

	bool has_flag(UiFlags flag) const
	{
		return m_iFlags & flag;
	}

	bool has_any_flag(int flags) const
	{
		return (m_iFlags & flags) != 0;
	}

	void add_flag(UiFlags flag)
	{
		m_iFlags |= flag;
	}

	void remove_flag(UiFlags flag)
	{
		m_iFlags &= ~flag;
	}

	//protected:
	UiType m_type;
	SDL_Rect m_rect;
	int m_iFlags;
};

//=============================================================================

class UiImage : public UiItemBase {
public:
	UiImage(Art *art, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_IMAGE;
		m_art = art;
		m_animated = false;
		m_frame = 0;
	};

	UiImage(Art *art, bool bAnimated, int iFrame, SDL_Rect rect, int flags)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_IMAGE;
		m_art = art;
		m_animated = bAnimated;
		m_frame = iFrame;
	};

	UiImage(Art *art, int frame, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_IMAGE;
		m_art = art;
		m_animated = false;
		m_frame = frame;
	}

	~UiImage() {};

	//private:
	Art *m_art;
	bool m_animated;
	int m_frame;
};

//=============================================================================

class UiArtText : public UiItemBase {
public:
	UiArtText(const char *text, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_ART_TEXT;
		m_text = text;
	};

	~UiArtText() {};

	//private:
	const char *m_text;
};

//=============================================================================

class UiScrollBar : public UiItemBase {
public:
	UiScrollBar(Art *bg, Art *thumb, Art *arrow, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_SCROLLBAR;
		m_bg = bg;
		m_thumb = thumb;
		m_arrow = arrow;
	};

	//private:
	Art *m_bg;
	Art *m_thumb;
	Art *m_arrow;
};

//=============================================================================

class UiArtTextButton : public UiItemBase {
public:
	UiArtTextButton(const char *text, void (*action)(), SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_ART_TEXT_BUTTON;
		m_text = text;
		m_action = action;
	};
	//private:
	const char *m_text;
	void (*m_action)();
};

//=============================================================================

class UiEdit : public UiItemBase {
public:
	UiEdit(const char *hint, char *value, unsigned max_length, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_EDIT;
		m_hint = hint;
		m_value = value;
		m_max_length = max_length;
	}

	//private:
	const char *m_hint;
	char *m_value;
	unsigned m_max_length;
};

//=============================================================================

// Plain text (TTF)

class UiText : public UiItemBase {
public:
	UiText(const char *text, SDL_Color color1, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_TEXT;
		m_color = color1;

		SDL_Color color2 = { 0, 0, 0, 0 };
		m_shadow_color = color2;

		m_text = text;
	}

	UiText(const char *text, SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_TEXT;

		SDL_Color color1 = { 243, 243, 243, 0 };
		m_color = color1;

		SDL_Color color2 = { 0, 0, 0, 0 };
		m_shadow_color = color2;

		m_text = text;
	}

	//private:
	SDL_Color m_color;
	SDL_Color m_shadow_color;
	const char *m_text;

	// State:
	TtfSurfaceCache m_render_cache;
};

//=============================================================================

// A button (uses Diablo sprites)

class UiButton : public UiItemBase {
public:
	UiButton(Art *art, const char *text, void (*action)(), SDL_Rect rect, int flags = 0)
	    : UiItemBase(rect, flags)
	{
		m_type = UI_BUTTON;
		m_art = art;
		m_text = text;
		m_action = action;
		m_pressed = false;
	}

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
	UiListItem(const char *text = "", int value = 0)
	{
		m_text = text;
		m_value = value;
	}

	~UiListItem()
	{
	}

	//private:
	const char *m_text;
	int m_value;
};

typedef std::vector<UiListItem *> vUiListItem;

class UiList : public UiItemBase {
public:
	UiList(vUiListItem vItems, int16_t x, int16_t y, uint16_t item_width, uint16_t item_height, int flags = 0)
	    : UiItemBase(x, y, item_width, item_height * vItems.size(), flags)
	{
		m_type = UI_LIST;
		m_vecItems = vItems;
		m_x = x;
		m_y = y;
		m_width = item_width;
		m_height = item_height;
	};

	~UiList() {};

	SDL_Rect itemRect(int i) const
	{
		SDL_Rect tmp;
		tmp.x = m_x;
		tmp.y = m_y + m_height * i;
		tmp.w = m_width;
		tmp.h = m_height;

		return tmp;
	}

	int indexAt(int16_t y) const
	{
		ASSERT(y >= m_rect.y);
		const unsigned index = (y - m_rect.y) / m_height;
		ASSERT(index < m_vecItems.size());
		return index;
	}

	UiListItem *GetItem(int i) const
	{
		return m_vecItems[i];
	}

	//private:
	int16_t m_x, m_y;
	uint16_t m_width, m_height;
	std::vector<UiListItem *> m_vecItems;
};
DEVILUTION_END_NAMESPACE
