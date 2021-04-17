#pragma once

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

bool UiSelHeroYesNoDialog(const char *title, const char *body);
void selyesno_Free();
void selyesno_Select(unsigned index);
void selyesno_Esc();

DEVILUTION_END_NAMESPACE
