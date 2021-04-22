#pragma once

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

extern bool selhero_isMultiPlayer;
extern bool selhero_endMenu;

void SelheroInit();
void SelheroListInit();
void SelheroListFocus(std::size_t index);
void SelheroListSelect(std::size_t index);
bool SelheroListDeleteYesNo();
void SelheroListEsc();
void SelheroClassSelectorFocus(std::size_t index);
void SelheroClassSelectorSelect(std::size_t index);
void SelheroClassSelectorEsc();
void SelheroUiFocusNavigationYesNo();
void SelheroNameSelect(std::size_t index);
void SelheroNameEsc();
void SelheroLoadFocus(std::size_t index);
void SelheroLoadSelect(std::size_t index);

DEVILUTION_END_NAMESPACE
