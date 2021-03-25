/**
 * @file sound.cpp
 *
 * Implementation of functions setting up the audio pipeline.
 */
#include "all.h"
#include "stubs.h"
#include <SDL.h>
#include <SDL_mixer.h>

DEVILUTION_BEGIN_NAMESPACE

BOOLEAN gbSndInited;
int sglMusicVolume;
int sglSoundVolume;
/** Specifies whether background music is enabled. */
HANDLE sghMusic;

Mix_Music *music;
SDL_RWops *musicRw;
char *musicBuffer;

/* data */

BOOLEAN gbMusicOn = true;
/** Specifies whether sound effects are enabled. */
BOOLEAN gbSoundOn = true;
/** Specifies the active background music track id. */
int sgnMusicTrack = NUM_MUSIC;
/** Maps from track ID to track name. */
const char *const sgszMusicTracks[NUM_MUSIC] = {
#ifdef SPAWN
	"Music\\sTowne.wav",
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
#ifdef HELLFIRE
	"Music\\sLvlA.wav",
	"Music\\sLvlA.wav",
#endif
	"Music\\sintro.wav",
#else
	"Music\\DTowne.wav",
	"Music\\DLvlA.wav",
	"Music\\DLvlB.wav",
	"Music\\DLvlC.wav",
	"Music\\DLvlD.wav",
#ifdef HELLFIRE
	"Music\\DLvlE.wav",
	"Music\\DLvlF.wav",
#endif
	"Music\\Dintro.wav",
#endif
};

static void snd_get_volume(const char *value_name, int *value)
{
	int v;

	if (SRegLoadValue("Diablo", value_name, 0, value)) {
		v = *value;
		if (v < VOLUME_MIN) {
			v = VOLUME_MIN;
		} else if (v > VOLUME_MAX) {
			v = VOLUME_MAX;
		}
	} else
		v = VOLUME_MAX;
	v -= v % 100;

	*value = v;
}

static void snd_set_volume(const char *key, int value)
{
	SRegSaveValue("Diablo", key, 0, value);
}

BOOL snd_playing(TSnd *pSnd)
{
	if (pSnd == NULL)
		return false;
	assert(pSnd->DSB != NULL);
	return pSnd->DSB->IsPlaying();
}

void snd_play_snd(TSnd *pSnd, int lVolume, int lPan)
{
	DWORD tc;

	if (pSnd == NULL || !gbSoundOn) {
		return;
	}

	tc = SDL_GetTicks();
	if (tc - pSnd->start_tc < 80) {
		return;
	}

	lVolume += sglSoundVolume;
	if (lVolume < VOLUME_MIN) {
		lVolume = VOLUME_MIN;
	} else if (lVolume > VOLUME_MAX) {
		lVolume = VOLUME_MAX;
	}
	assert(pSnd->DSB != NULL);
	pSnd->DSB->Play(lVolume, lPan);
	pSnd->start_tc = tc;
}

TSnd *sound_file_load(const char *path)
{
	HANDLE file;
	BYTE *wave_file;
	TSnd *pSnd;
	DWORD dwBytes;
	int error;

	SFileOpenFile(path, &file);
	pSnd = (TSnd *)DiabloAllocPtr(sizeof(TSnd));
	memset(pSnd, 0, sizeof(TSnd));
	pSnd->sound_path = path;
	pSnd->start_tc = SDL_GetTicks() - 80 - 1;

	dwBytes = SFileGetFileSize(file, NULL);
	wave_file = DiabloAllocPtr(dwBytes);
	SFileReadFile(file, wave_file, dwBytes, NULL, NULL);

	pSnd->DSB = new SoundSample();
	error = pSnd->DSB->SetChunk(wave_file, dwBytes);
	SFileCloseFile(file);
	mem_free_dbg(wave_file);
	if (error != 0) {
		ErrSdl();
	}

	return pSnd;
}

void sound_file_cleanup(TSnd *sound_file)
{
	if (sound_file != NULL) {
		assert(sound_file->DSB != NULL);
		sound_file->DSB->Stop();
		sound_file->DSB->Release();
		delete sound_file->DSB;
		sound_file->DSB = NULL;

		mem_free_dbg(sound_file);
	}
}

void snd_init()
{
	snd_get_volume("Sound Volume", &sglSoundVolume);
	gbSoundOn = sglSoundVolume > VOLUME_MIN;
	sgbSaveSoundOn = gbSoundOn;

	snd_get_volume("Music Volume", &sglMusicVolume);
	gbMusicOn = sglMusicVolume > VOLUME_MIN;

	int result = Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024);
	if (result < 0) {
		SDL_Log(Mix_GetError());
	}
	Mix_AllocateChannels(25);
	Mix_ReserveChannels(1); // reserve one channel for naration (SFileDda*)

	gbSndInited = true;
}

void sound_cleanup()
{
	if (gbSndInited) {
		gbSndInited = false;
		snd_set_volume("Sound Volume", sglSoundVolume);
		snd_set_volume("Music Volume", sglMusicVolume);
	}
}

void music_stop()
{
	if (sghMusic != NULL) {
		Mix_HaltMusic();
		SFileCloseFile(sghMusic);
		sghMusic = NULL;
		Mix_FreeMusic(music);
		music = NULL;
		musicRw = NULL;
		mem_free_dbg(musicBuffer);
		sgnMusicTrack = NUM_MUSIC;
	}
}

void music_start(int nTrack)
{
	BOOL success;

	assert((DWORD)nTrack < NUM_MUSIC);
	music_stop();
	if (gbMusicOn) {
		success = SFileOpenFile(sgszMusicTracks[nTrack], &sghMusic);
		if (!success) {
			sghMusic = NULL;
		} else {
			int bytestoread = SFileGetFileSize(sghMusic, 0);
			musicBuffer = (char *)DiabloAllocPtr(bytestoread);
			SFileReadFile(sghMusic, musicBuffer, bytestoread, NULL, 0);

			musicRw = SDL_RWFromConstMem(musicBuffer, bytestoread);
			if (musicRw == NULL) {
				ErrSdl();
			}
			music = Mix_LoadMUSType_RW(musicRw, MUS_NONE, 1);
			Mix_VolumeMusic(MIX_MAX_VOLUME - MIX_MAX_VOLUME * sglMusicVolume / VOLUME_MIN);
			Mix_PlayMusic(music, -1);

			sgnMusicTrack = nTrack;
		}
	}
}

void sound_disable_music(BOOL disable)
{
	if (disable) {
		music_stop();
	} else if (sgnMusicTrack != NUM_MUSIC) {
		music_start(sgnMusicTrack);
	}
}

int sound_get_or_set_music_volume(int volume)
{
	if (volume == 1)
		return sglMusicVolume;

	sglMusicVolume = volume;

	if (sghMusic != NULL)
		Mix_VolumeMusic(MIX_MAX_VOLUME - MIX_MAX_VOLUME * volume / VOLUME_MIN);

	return sglMusicVolume;
}

int sound_get_or_set_sound_volume(int volume)
{
	if (volume == 1)
		return sglSoundVolume;

	sglSoundVolume = volume;

	return sglSoundVolume;
}

DEVILUTION_END_NAMESPACE
