#pragma once

#include <cstddef>
#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

void selgame_GameSelection_Init();
void selgame_GameSelection_Focus(unsigned index);
void selgame_GameSelection_Select(unsigned index);
void selgame_GameSelection_Esc();
void selgame_Diff_Focus(unsigned index);
void selgame_Diff_Select(unsigned index);
void selgame_Diff_Esc();
void selgame_GameSpeedSelection();
void selgame_Speed_Focus(unsigned index);
void selgame_Speed_Select(unsigned index);
void selgame_Speed_Esc();
void selgame_Password_Init(unsigned index);
void selgame_Password_Select(unsigned index);
void selgame_Password_Esc();

DEVILUTION_END_NAMESPACE
