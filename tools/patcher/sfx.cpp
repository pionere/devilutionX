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

/** Specifies the currently streamed sound effect. */
static int sgpStreamSFX = SFX_NONE;

static SoundSample sgSndSamples[61] = { };

void StopStreamSFX()
{
	if (SFX_VALID(sgpStreamSFX)) {
		Mix_HaltChannel(SFX_STREAM_CHANNEL);
		sgSndSamples[sgpStreamSFX].Release();
		sgpStreamSFX = SFX_NONE;
	}
}

void StopSFX()
{
	StopStreamSFX();
	sound_stop();
}

static void StartStreamSFX(int nsfx, int lVolume, int lPan)
{
	// assert(sgSFX[nsfx].bFlags & sfx_STREAM);
	if (nsfx == sgpStreamSFX)
		return;
	StopStreamSFX();
	sgpStreamSFX = nsfx;

	sound_stream(sgSFX[nsfx].pszName, &sgSndSamples[nsfx], lVolume, lPan);
}

static void PlaySfx_priv(int nsfx)
{
	int lPan, lVolume;
	const SFXStruct* pSFX;

	if (!gbSoundOn)
		return;

	lPan = 0;
	lVolume = VOLUME_MAX;

	pSFX = &sgSFX[nsfx];
	/* not necessary, since non-streamed sfx should be loaded at this time
	   streams are loaded in StartStreamSFX
	if (!sgSndSamples[nsfx].IsLoaded()) {
		sound_file_load(pSFX->pszName, &sgSndSamples[nsfx]);
		// assert(sgSndSamples[nsfx].IsLoaded());
	}*/
	if (pSFX->bFlags & sfx_STREAM) {
		StartStreamSFX(nsfx, lVolume, lPan);
		return;
	}
	assert(sgSndSamples[nsfx].IsLoaded());
	if (!(pSFX->bFlags & sfx_MISC) && sgSndSamples[nsfx].IsPlaying()) {
		return;
	}

	sound_play(&sgSndSamples[nsfx], lVolume, lPan);
}

void PlaySfx(int nsfx)
{
	PlaySfx_priv(nsfx);
}

static void priv_sound_free(BYTE bLoadMask)
{
	int i;

	for (i = 0; i < lengthof(sgSFX); i++) {
		if (/*sgSndSamples[i].IsLoaded() &&*/ (sgSFX[i].bFlags & bLoadMask)) {
			sgSndSamples[i].Release();
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

		assert(!sgSndSamples[i].IsLoaded());

		sound_file_load(sgSFX[i].pszName, &sgSndSamples[i]);
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
