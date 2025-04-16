#include "text_draw.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "all.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

void DrawArtStr(const char* text, const SDL_Rect& rect, int flags)
{
	flags &= ~(UIS_OPTIONAL | UIS_DISABLED | UIS_HIDDEN),
	PrintString(flags, text, SCREEN_X + rect.x, SCREEN_Y + rect.y, rect.w, rect.h);
}

DEVILUTION_END_NAMESPACE
