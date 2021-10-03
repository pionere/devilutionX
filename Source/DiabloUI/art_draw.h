#pragma once

#include "../types.h"

#include "DiabloUI/art.h"

DEVILUTION_BEGIN_NAMESPACE

void DrawArt(int screenX, int screenY, Art* art, int nFrame = 0, int srcW = 0, int srcH = 0);

int GetAnimationFrame(int frames, int fps = 60);

DEVILUTION_END_NAMESPACE
