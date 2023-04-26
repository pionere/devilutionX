/**
 * @file effects.cpp
 *
 * Implementation of functions for loading and playing sounds.
 */
#include "all.h"
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#ifndef NOSOUND
/** Specifies the sound file and the playback state of the current sound effect. */
static SFXStruct* sgpStreamSFX = NULL;

/** List of all sounds, except monsters and music */
static SFXStruct sgSFX[] = {
	// clang-format off
//_sfx_id           bFlags,                   pszName,                       pSnd
/*PS_WALK1*/     { sfx_MISC,                  "Sfx\\Misc\\Walk1.wav",        { 0, NULL } },
/*PS_WALK2*///   { sfx_MISC,                  "Sfx\\Misc\\Walk2.wav",        { 0, NULL } },
/*PS_WALK3*///   { sfx_MISC,                  "Sfx\\Misc\\Walk3.wav",        { 0, NULL } },
/*PS_WALK4*///   { sfx_MISC,                  "Sfx\\Misc\\Walk4.wav",        { 0, NULL } },
/*PS_BFIRE*/     { sfx_MISC,                  "Sfx\\Misc\\BFire.wav",        { 0, NULL } },
/*PS_FMAG*///    { sfx_MISC,                  "Sfx\\Misc\\Fmag.wav",         { 0, NULL } },
/*PS_TMAG*///    { sfx_MISC,                  "Sfx\\Misc\\Tmag.wav",         { 0, NULL } },
/*PS_LGHIT*///   { sfx_MISC,                  "Sfx\\Misc\\Lghit.wav",        { 0, NULL } },
/*PS_LGHIT1*///  { sfx_MISC,                  "Sfx\\Misc\\Lghit1.wav",       { 0, NULL } },
/*PS_SWING*/     { sfx_MISC,                  "Sfx\\Misc\\Swing.wav",        { 0, NULL } },
/*PS_SWING2*/    { sfx_MISC,                  "Sfx\\Misc\\Swing2.wav",       { 0, NULL } },
/*PS_DEAD*/      { sfx_MISC,                  "Sfx\\Misc\\Dead.wav",         { 0, NULL } }, // Aaauh...
/*IS_STING1*///  { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\Sting1.wav",       { 0, NULL } },
/*IS_FBALLBOW*///{ sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\FBallBow.wav",     { 0, NULL } },
/*IS_QUESTDN*/   { sfx_STREAM,                "Sfx\\Misc\\Questdon.wav",     { 0, NULL } },
/*IS_ARMRFKD*/// { sfx_MISC,                  "Sfx\\Items\\Armrfkd.wav",     { 0, NULL } },
/*IS_BARLFIRE*/  { sfx_MISC,                  "Sfx\\Items\\Barlfire.wav",    { 0, NULL } },
/*IS_BARREL*/    { sfx_MISC,                  "Sfx\\Items\\Barrel.wav",      { 0, NULL } },
/*IS_POPPOP8*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\PodPop8.wav",     { 0, NULL } },
/*IS_POPPOP5*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\PodPop5.wav",     { 0, NULL } },
/*IS_POPPOP3*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\UrnPop3.wav",     { 0, NULL } },
/*IS_POPPOP2*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\UrnPop2.wav",     { 0, NULL } },
/*IS_CRCLOS*/    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\Crclos.wav",      { 0, NULL } },
/*IS_CROPEN*/    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\Cropen.wav",      { 0, NULL } },
/*IS_BHIT*///    { sfx_MISC,                  "Sfx\\Items\\Bhit.wav",        { 0, NULL } },
/*IS_BHIT1*///   { sfx_MISC,                  "Sfx\\Items\\Bhit1.wav",       { 0, NULL } },
/*IS_CHEST*/     { sfx_MISC,                  "Sfx\\Items\\Chest.wav",       { 0, NULL } },
/*IS_DOORCLOS*/  { sfx_MISC,                  "Sfx\\Items\\Doorclos.wav",    { 0, NULL } },
/*IS_DOOROPEN*/  { sfx_MISC,                  "Sfx\\Items\\Dooropen.wav",    { 0, NULL } },
/*IS_FANVL*/     { sfx_MISC,                  "Sfx\\Items\\Flipanvl.wav",    { 0, NULL } },
/*IS_FAXE*/      { sfx_MISC,                  "Sfx\\Items\\Flipaxe.wav",     { 0, NULL } },
/*IS_FBLST*/     { sfx_MISC,                  "Sfx\\Items\\Flipblst.wav",    { 0, NULL } },
/*IS_FBODY*/     { sfx_MISC,                  "Sfx\\Items\\Flipbody.wav",    { 0, NULL } },
/*IS_FBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Flipbook.wav",    { 0, NULL } },
/*IS_FBOW*/      { sfx_MISC,                  "Sfx\\Items\\Flipbow.wav",     { 0, NULL } },
/*IS_FCAP*/      { sfx_MISC,                  "Sfx\\Items\\Flipcap.wav",     { 0, NULL } },
/*IS_FHARM*/     { sfx_MISC,                  "Sfx\\Items\\Flipharm.wav",    { 0, NULL } },
/*IS_FLARM*/     { sfx_MISC,                  "Sfx\\Items\\Fliplarm.wav",    { 0, NULL } },
/*IS_FMAG*///    { sfx_MISC,                  "Sfx\\Items\\Flipmag.wav",     { 0, NULL } },
/*IS_FMAG1*///   { sfx_MISC,                  "Sfx\\Items\\Flipmag1.wav",    { 0, NULL } },
/*IS_FMUSH*/     { sfx_MISC,                  "Sfx\\Items\\Flipmush.wav",    { 0, NULL } },
/*IS_FPOT*/      { sfx_MISC,                  "Sfx\\Items\\Flippot.wav",     { 0, NULL } },
/*IS_FRING*/     { sfx_MISC,                  "Sfx\\Items\\Flipring.wav",    { 0, NULL } },
/*IS_FROCK*/     { sfx_MISC,                  "Sfx\\Items\\Fliprock.wav",    { 0, NULL } },
/*IS_FSCRL*/     { sfx_MISC,                  "Sfx\\Items\\Flipscrl.wav",    { 0, NULL } },
/*IS_FSHLD*/     { sfx_MISC,                  "Sfx\\Items\\Flipshld.wav",    { 0, NULL } },
/*IS_FSIGN*///   { sfx_MISC,                  "Sfx\\Items\\Flipsign.wav",    { 0, NULL } },
/*IS_FSTAF*/     { sfx_MISC,                  "Sfx\\Items\\Flipstaf.wav",    { 0, NULL } },
/*IS_FSWOR*/     { sfx_MISC,                  "Sfx\\Items\\Flipswor.wav",    { 0, NULL } },
/*IS_GOLD*/      { sfx_MISC,                  "Sfx\\Items\\Gold.wav",        { 0, NULL } },
/*IS_HLMTFKD*/// { sfx_MISC,                  "Sfx\\Items\\Hlmtfkd.wav",     { 0, NULL } },
/*IS_IANVL*/     { sfx_MISC,                  "Sfx\\Items\\Invanvl.wav",     { 0, NULL } },
/*IS_IAXE*/      { sfx_MISC,                  "Sfx\\Items\\Invaxe.wav",      { 0, NULL } },
/*IS_IBLST*/     { sfx_MISC,                  "Sfx\\Items\\Invblst.wav",     { 0, NULL } },
/*IS_IBODY*/     { sfx_MISC,                  "Sfx\\Items\\Invbody.wav",     { 0, NULL } },
/*IS_IBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Invbook.wav",     { 0, NULL } },
/*IS_IBOW*/      { sfx_MISC,                  "Sfx\\Items\\Invbow.wav",      { 0, NULL } },
/*IS_ICAP*/      { sfx_MISC,                  "Sfx\\Items\\Invcap.wav",      { 0, NULL } },
/*IS_IGRAB*/     { sfx_MISC,                  "Sfx\\Items\\Invgrab.wav",     { 0, NULL } },
/*IS_IHARM*/     { sfx_MISC,                  "Sfx\\Items\\Invharm.wav",     { 0, NULL } },
/*IS_ILARM*/     { sfx_MISC,                  "Sfx\\Items\\Invlarm.wav",     { 0, NULL } },
/*IS_IMUSH*/     { sfx_MISC,                  "Sfx\\Items\\Invmush.wav",     { 0, NULL } },
/*IS_IPOT*/      { sfx_MISC,                  "Sfx\\Items\\Invpot.wav",      { 0, NULL } },
/*IS_IRING*/     { sfx_MISC,                  "Sfx\\Items\\Invring.wav",     { 0, NULL } },
/*IS_IROCK*/     { sfx_MISC,                  "Sfx\\Items\\Invrock.wav",     { 0, NULL } },
/*IS_ISCROL*/    { sfx_MISC,                  "Sfx\\Items\\Invscrol.wav",    { 0, NULL } },
/*IS_ISHIEL*/    { sfx_MISC,                  "Sfx\\Items\\Invshiel.wav",    { 0, NULL } },
/*IS_ISIGN*/     { sfx_MISC,                  "Sfx\\Items\\Invsign.wav",     { 0, NULL } },
/*IS_ISTAF*/     { sfx_MISC,                  "Sfx\\Items\\Invstaf.wav",     { 0, NULL } },
/*IS_ISWORD*/    { sfx_MISC,                  "Sfx\\Items\\Invsword.wav",    { 0, NULL } },
/*IS_LEVER*/     { sfx_MISC,                  "Sfx\\Items\\Lever.wav",       { 0, NULL } },
/*IS_MAGIC*/     { sfx_MISC,                  "Sfx\\Items\\Magic.wav",       { 0, NULL } },
/*IS_MAGIC1*/    { sfx_MISC,                  "Sfx\\Items\\Magic1.wav",      { 0, NULL } },
/*IS_RBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Readbook.wav",    { 0, NULL } },
/*IS_SARC*/      { sfx_MISC,                  "Sfx\\Items\\Sarc.wav",        { 0, NULL } },
/*IS_SHLDFKD*/// { sfx_MISC,                  "Sfx\\Items\\Shielfkd.wav",    { 0, NULL } },
/*IS_SWRDFKD*/// { sfx_MISC,                  "Sfx\\Items\\Swrdfkd.wav",     { 0, NULL } },
/*IS_TITLEMOV*/  { sfx_UI,                    "Sfx\\Items\\Titlemov.wav",    { 0, NULL } },
/*IS_TITLSLCT*/  { sfx_UI,                    "Sfx\\Items\\Titlslct.wav",    { 0, NULL } },
/*SFX_SILENCE*/  { sfx_UI,                    "Sfx\\Misc\\blank.wav",        { 0, NULL } },
	// clang-format on
};

bool IsSFXPlaying(int nSFX)
{
	SFXStruct* sfx = &sgSFX[nSFX];

	if (sfx->bFlags & sfx_STREAM)
		return sfx == sgpStreamSFX;
	return sfx->pSnd.IsPlaying();
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

static void PlaySFX_priv(int psfx, bool loc, int x, int y)
{
	int lPan, lVolume;
	SFXStruct* pSFX;

	if (!gbSoundOn)
		return;

	lPan = 0;
	lVolume = VOLUME_MAX;

	pSFX = &sgSFX[psfx];
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

void PlaySFX(int psfx, int rndCnt)
{
	PlaySFX_priv(psfx, false, 0, 0);
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

#endif // NOSOUND

DEVILUTION_END_NAMESPACE
