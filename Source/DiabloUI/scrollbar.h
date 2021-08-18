#pragma once

#include "DiabloUI/art.h"
#include "DiabloUI/ui_item.h"

DEVILUTION_BEGIN_NAMESPACE

extern Art ArtScrollBarBackground;
extern Art ArtScrollBarThumb;
const int SCROLLBAR_BG_WIDTH = 25;

extern Art ArtScrollBarArrow;
enum ScrollBarArrowFrame {
	ScrollBarArrowFrame_UP_ACTIVE,
	ScrollBarArrowFrame_UP,
	ScrollBarArrowFrame_DOWN_ACTIVE,
	ScrollBarArrowFrame_DOWN,
};
const int SCROLLBAR_ARROW_WIDTH = 25;

inline SDL_Rect UpArrowRect(const UiScrollBar *sb)
{
	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x;
	Tmp.y = sb->m_rect.y;
	Tmp.w = SCROLLBAR_ARROW_WIDTH;
	Tmp.h = sb->m_arrow->h();

	return Tmp;
}

inline SDL_Rect DownArrowRect(const UiScrollBar *sb)
{
	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x;
	Tmp.y = sb->m_rect.y + sb->m_rect.h - sb->m_arrow->h();
	Tmp.w = SCROLLBAR_ARROW_WIDTH,
	Tmp.h = sb->m_arrow->h();

	return Tmp;
}

inline int BarHeight(const UiScrollBar *sb)
{
	return sb->m_rect.h - 2 * sb->m_arrow->h();
}

inline SDL_Rect BarRect(const UiScrollBar *sb)
{
	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x;
	Tmp.y = sb->m_rect.y + sb->m_arrow->h();
	Tmp.w = SCROLLBAR_ARROW_WIDTH,
	Tmp.h = BarHeight(sb);

	return Tmp;
}

inline SDL_Rect ThumbRect(const UiScrollBar* sb, unsigned selected_index, unsigned maxIndex)
{
	const int THUMB_OFFSET_X = 3;
	const int thumb_max_y = BarHeight(sb) - sb->m_thumb->h();
	const int thumb_y = selected_index * thumb_max_y / maxIndex;

	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x + THUMB_OFFSET_X;
	Tmp.y = sb->m_rect.y + sb->m_arrow->h() + thumb_y;
	Tmp.w = sb->m_rect.w - THUMB_OFFSET_X;
	Tmp.h = sb->m_thumb->h();

	return Tmp;
}

void LoadScrollBar();
void UnloadScrollBar();

DEVILUTION_END_NAMESPACE
