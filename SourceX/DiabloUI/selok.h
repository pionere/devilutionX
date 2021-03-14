#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

void UiSelOkDialog(const char *title, const char *body, bool background);
void selok_Free();
void selok_Select(unsigned index);
void selok_Esc();

DEVILUTION_END_NAMESPACE
