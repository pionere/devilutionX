#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

extern bool selhero_isMultiPlayer;
extern bool selhero_endMenu;

void selhero_Init();
void selhero_List_Init();
void selhero_List_Focus(std::size_t index);
void selhero_List_Select(std::size_t index);
bool selhero_List_DeleteYesNo();
void selhero_List_Esc();
void selhero_ClassSelector_Focus(std::size_t index);
void selhero_ClassSelector_Select(std::size_t index);
void selhero_ClassSelector_Esc();
void selhero_UiFocusNavigationYesNo();
void selhero_Name_Select(std::size_t index);
void selhero_Name_Esc();
void selhero_Load_Focus(std::size_t index);
void selhero_Load_Select(std::size_t index);

DEVILUTION_END_NAMESPACE
