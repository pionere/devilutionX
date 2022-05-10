#pragma once

#include "DiabloUI/ui_item.h"
#include "../gameui.h"

DEVILUTION_BEGIN_NAMESPACE

extern CelImageBuf* scrollBarBackCel;
extern CelImageBuf* scrollBarThumbCel;
extern CelImageBuf* scrollBarArrowCel;

enum ScrollBarArrowFrame {
	ScrollBarArrowFrame_UP_ACTIVE,
	ScrollBarArrowFrame_UP,
	ScrollBarArrowFrame_DOWN_ACTIVE,
	ScrollBarArrowFrame_DOWN,
};

inline SDL_Rect UpArrowRect(const UiScrollBar *sb)
{
	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x;
	Tmp.y = sb->m_rect.y + SCROLLBAR_ARROW_HEIGHT;
	Tmp.w = SCROLLBAR_ARROW_WIDTH;
	Tmp.h = SCROLLBAR_ARROW_HEIGHT;

	return Tmp;
}

inline SDL_Rect DownArrowRect(const UiScrollBar *sb)
{
	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x;
	Tmp.y = sb->m_rect.y + sb->m_rect.h;
	Tmp.w = SCROLLBAR_ARROW_WIDTH,
	Tmp.h = SCROLLBAR_ARROW_HEIGHT;

	return Tmp;
}

inline int BarHeight(const UiScrollBar *sb)
{
	return sb->m_rect.h - 2 * SCROLLBAR_ARROW_HEIGHT;
}

inline SDL_Rect BarRect(const UiScrollBar *sb)
{
	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x;
	Tmp.y = sb->m_rect.y + SCROLLBAR_ARROW_HEIGHT;
	Tmp.w = SCROLLBAR_ARROW_WIDTH,
	Tmp.h = BarHeight(sb);

	return Tmp;
}

inline SDL_Rect ThumbRect(const UiScrollBar* sb, unsigned selected_index, unsigned maxIndex)
{
	const int thumb_max_y = BarHeight(sb) - SCROLLBAR_THUMB_HEIGHT;
	const int thumb_y = selected_index * thumb_max_y / maxIndex;

	SDL_Rect Tmp;
	Tmp.x = sb->m_rect.x + SCROLLBAR_THUMB_OFFSET_X;
	Tmp.y = sb->m_rect.y + SCROLLBAR_ARROW_HEIGHT + thumb_y + SCROLLBAR_THUMB_HEIGHT;
	Tmp.w = sb->m_rect.w - SCROLLBAR_THUMB_OFFSET_X;
	Tmp.h = SCROLLBAR_THUMB_HEIGHT;

	return Tmp;
}

void LoadScrollBar();
void UnloadScrollBar();

DEVILUTION_END_NAMESPACE
