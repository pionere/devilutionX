/**
 * @file lighting.cpp
 *
 * Implementation of light and vision.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/*
 * In-game color translation tables.
 * 0-MAXDARKNESS: inverse brightness translations.
 * MAXDARKNESS+1: RED color translation.
 * MAXDARKNESS+2: GRAY color translation.
 * MAXDARKNESS+3: CORAL color translation.
 * MAXDARKNESS+4.. translations of unique monsters.
 */
BYTE ColorTrns[NUM_COLOR_TRNS][NUM_COLORS];

void InitLighting()
{
	LoadFileWithMem("Levels\\TownData\\Town.TRS", ColorTrns[0]);
	LoadFileWithMem("PlrGFX\\Infra.TRN", ColorTrns[COLOR_TRN_RED]);
	LoadFileWithMem("PlrGFX\\Stone.TRN", ColorTrns[COLOR_TRN_GRAY]);
	LoadFileWithMem("PlrGFX\\Coral.TRN", ColorTrns[COLOR_TRN_CORAL]);
}

DEVILUTION_END_NAMESPACE
