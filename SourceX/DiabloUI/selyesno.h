#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

bool UiSelHeroYesNoDialog(const char *title, const char *body);
void selyesno_Free();
void selyesno_Select(int value);
void selyesno_Esc();

DEVILUTION_END_NAMESPACE
