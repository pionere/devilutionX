/**
 * @file sfx.cpp
 *
 * Implementation of functions for loading and playing sounds.
 */
#include "all.h"
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"
#include "sfxdat.h"

DEVILUTION_BEGIN_NAMESPACE

/** Specifies the sound file and the playback state of the current sound effect. */
static SFXStruct* sgpStreamSFX = NULL;

bool IsSfxPlaying(int nsfx)
{
	SFXStruct* pSFX = &sgSFX[nsfx];

	if (pSFX->bFlags & sfx_STREAM)
		return pSFX == sgpStreamSFX;
	return pSFX->pSnd.IsPlaying();
}

void StopStreamSFX()
{
	if (sgpStreamSFX != NULL) {
		Mix_HaltChannel(SFX_STREAM_CHANNEL);
		sgpStreamSFX->pSnd.Release();
		sgpStreamSFX = NULL;
	}
}

void StopSFX()
{
	StopStreamSFX();
	sound_stop();
}

static void StartStreamSFX(SFXStruct* pSFX, int lVolume, int lPan)
{
	// assert(pSFX != NULL);
	// assert(pSFX->bFlags & sfx_STREAM);
	// assert(pSFX->pSnd != NULL);
	if (pSFX == sgpStreamSFX)
		return;
	StopStreamSFX();
	sgpStreamSFX = pSFX;

	sound_stream(pSFX->pszName, &pSFX->pSnd, lVolume, lPan);
}

void CheckStreamSFX()
{
	if (sgpStreamSFX != NULL && !sgpStreamSFX->pSnd.IsPlaying()) {
		StopStreamSFX();
	}
}

static void PlaySfx_priv(int nsfx)
{
	int lPan, lVolume;
	SFXStruct* pSFX;

	if (!gbSoundOn)
		return;

	lPan = 0;
	lVolume = VOLUME_MAX;

	pSFX = &sgSFX[nsfx];
	/* not necessary, since non-streamed sfx should be loaded at this time
	   streams are loaded in StartStreamSFX
	if (!pSFX->pSnd.IsLoaded()) {
		sound_file_load(pSFX->pszName, &pSFX->pSnd);
		// assert(pSFX->pSnd.IsLoaded());
	}*/
	if (pSFX->bFlags & sfx_STREAM) {
		StartStreamSFX(pSFX, lVolume, lPan);
		return;
	}
	assert(pSFX->pSnd.IsLoaded());
	if (!(pSFX->bFlags & sfx_MISC) && pSFX->pSnd.IsPlaying()) {
		return;
	}

	sound_play(&pSFX->pSnd, lVolume, lPan);
}

void PlaySfx(int nsfx)
{
	PlaySfx_priv(nsfx);
}

static void priv_sound_free(BYTE bLoadMask)
{
	int i;

	for (i = 0; i < lengthof(sgSFX); i++) {
		if (/*sgSFX[i].pSnd.IsLoaded() &&*/ (sgSFX[i].bFlags & bLoadMask)) {
			sgSFX[i].pSnd.Release();
		}
	}
}

static void priv_sound_init(BYTE bLoadMask)
{
	int i;

	assert(gbSndInited);

	for (i = 0; i < lengthof(sgSFX); i++) {
		if ((sgSFX[i].bFlags & bLoadMask) != sgSFX[i].bFlags) {
			continue;
		}

		assert(!sgSFX[i].pSnd.IsLoaded());

		sound_file_load(sgSFX[i].pszName, &sgSFX[i].pSnd);
	}
}

void InitUiSFX()
{
	priv_sound_init(sfx_UI);
}

void FreeUiSFX()
{
	priv_sound_free(sfx_UI);
}

DEVILUTION_END_NAMESPACE

#endif // NOSOUND
