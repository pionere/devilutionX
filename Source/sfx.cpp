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

/** Audio data of the sound effects. */
static SoundSample sgSndSamples[NUM_SFXS] = { };

/** Maps from monster sfx to monster sound letter. */
static const char MonstSndChar[NUM_MON_SFX] = { 'a', 'h', 'd', 's' };

bool IsSfxStreaming(int nsfx)
{
	return nsfx == sgpStreamSFX;
}
#if 0
bool IsSfxPlaying(int nsfx)
{
	if (sfxdata[nsfx].bFlags & sfx_STREAM)
		return nsfx == sgpStreamSFX;
	return sgSndSamples[nsfx].IsPlaying();
}
#endif
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
	// assert(sfxdata[nsfx].bFlags & sfx_STREAM);
	if (nsfx == sgpStreamSFX)
		return;
	StopStreamSFX();
	sgpStreamSFX = nsfx;

	sound_stream(sfxfiledata[nsfx].pszName, &sgSndSamples[nsfx], lVolume, lPan);
}

void CheckStreamSFX()
{
	if (SFX_VALID(sgpStreamSFX) && !sgSndSamples[sgpStreamSFX].IsPlaying()) {
		StopStreamSFX();
	}
}

void InitMonsterSFX(int midx)
{
	char name[DATA_ARCHIVE_MAX_PATH];
	int i, n, j;
	MapMonData* cmon;
	const MonsterData* mdata;
	const MonFileData* mfdata;

	assert(gbSndInited);

	cmon = &mapMonTypes[midx];
	mdata = &monsterdata[cmon->cmType];
	mfdata = &monfiledata[mdata->moFileNum];
	static_assert((int)MS_SPECIAL + 1 == NUM_MON_SFX, "InitMonsterSND requires MS_SPECIAL at the end of the enum.");
	n = mfdata->moSndSpecial ? NUM_MON_SFX : MS_SPECIAL;
	for (i = 0; i < n; i++) {
		for (j = 0; j < lengthof(cmon->cmSnds[i]); j++) {
			snprintf(name, sizeof(name), mfdata->moSndFile, MonstSndChar[i], j + 1);
			assert(!cmon->cmSnds[i][j].IsLoaded());
			sound_file_load(name, &cmon->cmSnds[i][j]);
			assert(cmon->cmSnds[i][j].IsLoaded());
		}
	}
}

void FreeMonsterSFX()
{
	MapMonData* cmon;
	int i, j, k;

	cmon = mapMonTypes;
	for (i = 0; i < nummtypes; i++, cmon++) {
		for (j = 0; j < NUM_MON_SFX; ++j) {
			for (k = 0; k < lengthof(cmon->cmSnds[j]); ++k) {
				//if (cmon->cmSnds[j][k].IsLoaded())
					cmon->cmSnds[j][k].Release();
			}
		}
	}
}

static bool calc_snd_position(int x, int y, int* plVolume, int* plPan)
{
	int pan, volume;

	x -= myplr._px;
	y -= myplr._py;

	pan = (x - y);
	*plPan = pan;

	if (abs(pan) > SFX_DIST_MAX)
		return false;

	volume = std::max(abs(x), abs(y));
	if (volume >= SFX_DIST_MAX)
		return false;

	static_assert(((VOLUME_MAX - VOLUME_MIN) % SFX_DIST_MAX) == 0, "Volume calculation in calc_snd_position requires matching VOLUME_MIN/MAX and SFX_DIST_MAX values.");
	static_assert(((((VOLUME_MAX - VOLUME_MIN) / SFX_DIST_MAX)) & ((VOLUME_MAX - VOLUME_MIN) / SFX_DIST_MAX - 1)) == 0, "Volume calculation in calc_snd_position is no longer optimal for performance.");
	volume *= (VOLUME_MAX - VOLUME_MIN) / SFX_DIST_MAX;
	*plVolume = VOLUME_MAX - volume;

	return true;
}

static void PlaySfx_priv(int nsfx, bool loc, int x, int y)
{
	int lPan, lVolume;
	const SFXData* pSFX;

	if (!gbSoundOn || gbLvlLoad)
		return;

	lPan = 0;
	lVolume = VOLUME_MAX;
	if (loc && !calc_snd_position(x, y, &lVolume, &lPan)) {
		return;
	}

	pSFX = &sfxdata[nsfx];
	/* not necessary, since non-streamed sfx should be loaded at this time
	   streams are loaded in StartStreamSFX
	if (!sgSndSamples[nsfx].IsLoaded()) {
		sound_file_load(sfxfiledata[nsfx].pszName, &sgSndSamples[nsfx]);
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

void PlayMonSfx(int mnum, int mode)
{
	MonsterStruct* mon;
	int sndIdx, lVolume, lPan;
	SoundSample* snd;

	sndIdx = random_(164, lengthof(mapMonTypes[0].cmSnds[0]));
	if (!gbSoundOn || gbLvlLoad)
		return;

	mon = &monsters[mnum];
	snd = &mapMonTypes[mon->_mMTidx].cmSnds[mode][sndIdx];
	if (!snd->IsLoaded()) {
		assert(mode == MS_SPECIAL);
		assert(!monfiledata[monsterdata[mon->_mType].moFileNum].moSndSpecial);
		return;
	}
	if (snd->IsPlaying()) {
		return;
	}

	if (!calc_snd_position(mon->_mx, mon->_my, &lVolume, &lPan))
		return;

	sound_play(snd, lVolume, lPan);
}

void PlaySfx(int nsfx)
{
	PlaySfx_priv(nsfx, false, 0, 0);
}

void PlaySfxN(int nsfx, int rndCnt)
{
	if (rndCnt > 1)
		nsfx += random_low(165, rndCnt);
	PlaySfx(nsfx);
}

void PlaySfxLoc(int nsfx, int x, int y)
{
	PlaySfx_priv(nsfx, true, x, y);
}

void PlaySfxLocN(int nsfx, int x, int y, int rndCnt)
{
	if (rndCnt > 1)
		nsfx += random_low(165, rndCnt);

	PlaySfxLoc(nsfx, x, y);
}

void PlayWalkSfx(int pnum)
{
	int nsfx = PS_WALK1;

	sgSndSamples[nsfx].lastTc = 0;

	PlaySfxLoc(nsfx, plr._px, plr._py);
}

static void priv_sound_free(BYTE bLoadMask)
{
	int i;
	static_assert(lengthof(sfxdata) == lengthof(sgSndSamples), "priv_sound_free must traverse sgSndSamples parallel to sfxdata");	
	for (i = 0; i < lengthof(sfxdata); i++) {
		if (/*sgSndSamples[i].IsLoaded() &&*/ (sfxdata[i].bFlags & bLoadMask)) {
			sgSndSamples[i].Release();
		}
	}
}

static void priv_sound_init(BYTE bLoadMask)
{
	int i;

	assert(gbSndInited);
	static_assert(lengthof(sfxdata) == lengthof(sfxfiledata), "priv_sound_init must traverse sfxfiledata parallel to sfxdata");
	static_assert(lengthof(sfxdata) == lengthof(sgSndSamples), "priv_sound_init must traverse sgSndSamples parallel to sfxdata");	
	for (i = 0; i < lengthof(sfxdata); i++) {
		if ((sfxdata[i].bFlags & bLoadMask) != sfxdata[i].bFlags) {
			continue;
		}

		assert(!sgSndSamples[i].IsLoaded());

		sound_file_load(sfxfiledata[i].pszName, &sgSndSamples[i]);
	}
}

void InitGameSFX()
{
#ifdef HELLFIRE
	BYTE mask = sfx_MISC | sfx_HELLFIRE;
#else
	BYTE mask = sfx_MISC;
#endif
	if (IsLocalGame) {
		mask |= sgSFXSets[SFXS_MASK][myplr._pClass];
	} else {
		mask |= sfx_WARRIOR | sfx_ROGUE | sfx_SORCERER;
#ifdef HELLFIRE
		mask |= sfx_MONK;
#endif
	}

	priv_sound_init(mask);
}

void InitUiSFX()
{
	priv_sound_init(sfx_UI);
}

void FreeGameSFX()
{
	priv_sound_free(~(sfx_UI | sfx_STREAM));
}

void FreeUiSFX()
{
	priv_sound_free(sfx_UI);
}

DEVILUTION_END_NAMESPACE

#endif // NOSOUND
