#pragma once

#include <cstddef>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

void SelgameModeInit();
void SelgameModeFocus(unsigned index);
void SelgameModeSelect(unsigned index);
void SelgameModeEsc();
void SelgameDiffFocus(unsigned index);
void SelgameDiffSelect(unsigned index);
void SelgameDiffEsc();
void SelgameSpeedInit();
void SelgameSpeedFocus(unsigned index);
void SelgameSpeedSelect(unsigned index);
void SelgameSpeedEsc();
void SelgamePasswordInit(unsigned index);
void SelgamePasswordSelect(unsigned index);
void SelgamePasswordEsc();

DEVILUTION_END_NAMESPACE
