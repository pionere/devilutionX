#pragma once

#include "DiabloUI/art.h"
#include "DiabloUI/ui_item.h"

DEVILUTION_BEGIN_NAMESPACE

extern Art SmlButton;
void LoadSmlButtonArt();
inline void UnloadSmlButtonArt()
{
	SmlButton.Unload();
}
const uint16_t SML_BUTTON_WIDTH = 110;
const uint16_t SML_BUTTON_HEIGHT = 28;

DEVILUTION_END_NAMESPACE
