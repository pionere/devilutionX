#pragma once

#include "ui_item.h"
#include "../gameui.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern CelImageBuf* scrollBarBackCel;
extern CelImageBuf* scrollBarThumbCel;
extern CelImageBuf* scrollBarArrowCel;

void LoadScrollBar();
void UnloadScrollBar();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
