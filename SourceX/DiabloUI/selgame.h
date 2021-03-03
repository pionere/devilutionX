#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

void selgame_GameSelection_Init();
void selgame_GameSelection_Focus(std::size_t index);
void selgame_GameSelection_Select(std::size_t index);
void selgame_GameSelection_Esc();
void selgame_Diff_Focus(std::size_t index);
void selgame_Diff_Select(std::size_t index);
void selgame_Diff_Esc();
void selgame_GameSpeedSelection();
void selgame_Speed_Focus(std::size_t index);
void selgame_Speed_Select(std::size_t index);
void selgame_Speed_Esc();
void selgame_Password_Init(std::size_t index);
void selgame_Password_Select(std::size_t index);
void selgame_Password_Esc();

DEVILUTION_END_NAMESPACE
