#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

bool UiSelHeroYesNoDialog(const char *title, const char *body);
void SelyesnoFree();
void SelyesnoSelect(std::size_t index);
void SelyesnoEsc();

DEVILUTION_END_NAMESPACE
