/**
 * @file sound.cpp
 *
 * Implementation of functions setting up the audio pipeline.
 */
#include "all.h"
#include <SDL.h>
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

/** Specifies whether sound effects are enabled. */
bool gbSoundOn = false;
/** Specifies whether music effects are enabled. */
bool gbMusicOn = false;
/** The volume of the sound channel. */
int gnSoundVolume;
/** The volume of the music channel. */
int gnMusicVolume;

#ifndef NOSOUND
/** Buffer containing the data of the background music. */
BYTE* _gMusicBuffer;

/** Specifies the active background music track id. */
int _gnMusicTrack = NUM_MUSIC;
/** Maps from track ID to track name. */
const char* const sgszMusicTracks[NUM_MUSIC] = {
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
};

static void snd_get_volume(const char* value_name, int* value)
{
	int v;

	if (getIniInt("Audio", value_name, value)) {
		v = *value;
		if (v < VOLUME_MIN) {
			v = VOLUME_MIN;
		} else if (v > VOLUME_MAX) {
			v = VOLUME_MAX;
		}
	} else
		v = VOLUME_MAX;
	// v -= v % 100;

	*value = v;
}

void snd_play_snd(SoundSample* pSnd, int lVolume, int lPan)
{
	Uint32 currTc;

	assert(gbSoundOn);
	assert(pSnd != NULL);

	lVolume = ADJUST_VOLUME(lVolume, VOLUME_MIN, gnSoundVolume);
	if (lVolume <= VOLUME_MIN)
		return;
	assert(lVolume <= VOLUME_MAX);
	currTc = SDL_GetTicks();
	if (currTc < pSnd->nextTc)
		return;
	pSnd->nextTc = currTc + 80;
	pSnd->Play(lVolume, lPan);
}

void sound_file_load(const char* path, SoundSample* pSnd)
{
	BYTE* wave_file;
	size_t dwBytes;

	wave_file = LoadFileInMem(path, &dwBytes);

	pSnd->nextTc = 0;
	pSnd->SetChunk(wave_file, dwBytes);
	mem_free_dbg(wave_file);
}

void RestartMixer()
{
	if (Mix_OpenAudio(SND_DEFAULT_FREQUENCY, SND_DEFAULT_FORMAT, SND_DEFAULT_CHANNELS, 1024) < 0) {
		SDL_Log("%s", Mix_GetError());
	}
	Mix_VolumeMusic(MIX_VOLUME(gnMusicVolume));
}

void InitSound()
{
	snd_get_volume("Sound Volume", &gnSoundVolume);
	gbSoundOn = gnSoundVolume > VOLUME_MIN;

	snd_get_volume("Music Volume", &gnMusicVolume);
	gbMusicOn = gnMusicVolume > VOLUME_MIN;

	Mix_Init(0);
	RestartMixer();
}

void FreeSound()
{
	Mix_CloseAudio();
	Mix_Quit();
}

void music_stop()
{
	if (_gMusicBuffer != NULL) {
		// Mix_HaltMusic(); -- no need, Mix_FreeMusic halts the music as well
		Mix_FreeMusic();
		_gnMusicTrack = NUM_MUSIC;
		MemFreeDbg(_gMusicBuffer);
	}
}

void music_start(int nTrack)
{
	size_t dwBytes;

	assert((unsigned)nTrack < NUM_MUSIC);
	if (gbMusicOn) {
		music_stop();

		assert(_gMusicBuffer == NULL);
		_gMusicBuffer = LoadFileInMem(sgszMusicTracks[nTrack], &dwBytes);

		Mix_RWops* musicRw = Mix_RWFromConstMem(_gMusicBuffer, dwBytes);
		if (musicRw == NULL || !Mix_LoadMUS_RW(musicRw))
			sdl_fatal(ERR_SDL_MUSIC_FILE);

		Mix_PlayMusic(-1);

		_gnMusicTrack = nTrack;
	}
}

void sound_disable_music()
{
	music_stop();
}

void sound_start_music()
{
	if (_gnMusicTrack != NUM_MUSIC) {
		music_start(_gnMusicTrack);
	}
}

void sound_set_music_volume(int volume)
{
	gnMusicVolume = volume;
	gbMusicOn = volume > VOLUME_MIN;
	setIniInt("Audio", "Music Volume", volume);

	Mix_VolumeMusic(MIX_VOLUME(gnMusicVolume));
}

void sound_set_sound_volume(int volume)
{
	gnSoundVolume = volume;
	gbSoundOn = volume > VOLUME_MIN;
	setIniInt("Audio", "Sound Volume", volume);
}

#endif // NOT NOSOUND

DEVILUTION_END_NAMESPACE
