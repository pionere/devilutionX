#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

extern _uiheroinfo selhero_heroInfo;

void selhero_Init();
void selhero_List_Init();
void selhero_List_Focus(unsigned index);
void selhero_List_Select(unsigned index);
bool selhero_List_DeleteYesNo();
void selhero_List_Esc();
void selhero_ClassSelector_Focus(unsigned index);
void selhero_ClassSelector_Select(unsigned index);
void selhero_ClassSelector_Esc();
void selhero_UiFocusNavigationYesNo();
void selhero_Name_Select(unsigned index);
void selhero_Name_Esc();
void selhero_Load_Focus(unsigned index);
void selhero_Load_Select(unsigned index);

DEVILUTION_END_NAMESPACE
