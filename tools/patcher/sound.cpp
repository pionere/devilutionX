/**
 * @file sound.cpp
 *
 * Implementation of functions setting up the audio pipeline.
 */
#include "all.h"
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"
#endif
#include "storm/storm_cfg.h"

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
::Mix_Audio* _gMusic;

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

void sound_play(SoundSample* pSnd, int lVolume, int lPan)
{
	Uint32 currTc;

	assert(gbSoundOn);
	assert(pSnd != NULL);

	lVolume = ADJUST_VOLUME(lVolume, VOLUME_MIN, gnSoundVolume);
	if (lVolume <= VOLUME_MIN)
		return;
	assert(lVolume <= VOLUME_MAX);
	currTc = SDL_GetTicks();
	if (/*pSnd->lastTc != 0 && */!SDL_TICKS_AFTER(currTc, pSnd->lastTc, 80))
		return;
	pSnd->lastTc = currTc;
	pSnd->Play(lVolume, lPan, -1);
}

void sound_stream(const char* path, SoundSample* pSnd, int lVolume, int lPan)
{
	BYTE* wave_file;
	size_t dwBytes;

	assert(gbSoundOn);
	assert(pSnd != NULL);

	assert(!pSnd->IsLoaded());
	{
		// sound_file_load(path, pSnd);
		wave_file = LoadFileInMem(path, &dwBytes);

		pSnd->SetChunk(wave_file, dwBytes, true);

		mem_free_dbg(wave_file);
	}

	lVolume = ADJUST_VOLUME(lVolume, VOLUME_MIN, gnSoundVolume);

	pSnd->Play(lVolume, lPan, SFX_STREAM_CHANNEL);
}

void sound_stop()
{
	Mix_HaltChannel(-1);
}

void sound_pause(bool pause)
{
	if (pause)
		Mix_Pause(-1);
	else
		Mix_Resume(-1);
}

void sound_file_load(const char* path, SoundSample* pSnd)
{
	BYTE* wave_file;
	size_t dwBytes;

	wave_file = LoadFileInMem(path, &dwBytes);

	pSnd->lastTc = 0;
	pSnd->SetChunk(wave_file, dwBytes, false);
	mem_free_dbg(wave_file);
}

void RestartMixer()
{
	if (Mix_OpenAudioDevice(SND_DEFAULT_FREQUENCY, SND_DEFAULT_FORMAT, SND_DEFAULT_CHANNELS, 1024, NULL, SDL_AUDIO_ALLOW_SAMPLES_CHANGE) < 0) {
		DoLog(Mix_GetError());
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
	if (_gMusic != NULL) {
		// Mix_HaltMusic(); -- no need, Mix_FreeMusic halts the music as well
		Mix_FreeMusic();
		_gnMusicTrack = NUM_MUSIC;
		_gMusic = NULL;
	}
}

void music_start(int nTrack)
{
	BYTE* musicBuf;
	size_t dwBytes;

	assert((unsigned)nTrack < NUM_MUSIC);
	if (gbMusicOn) {
		music_stop();

		musicBuf = LoadFileInMem(sgszMusicTracks[nTrack], &dwBytes);

		//Mix_RWops* musicRw = Mix_RWFromConstMem(_gMusicBuffer, dwBytes);
		Mix_RWops musicRw;
		Mix_RWFromMem(&musicRw, musicBuf, dwBytes);
		//if (musicRw == NULL || !Mix_LoadMUS_RW(musicRw))
		_gMusic = Mix_LoadMUS_RW(&musicRw);
		if (_gMusic == NULL)
			sdl_error(ERR_SDL_MUSIC_FILE);

		Mix_PlayMusic(-1);

		_gnMusicTrack = nTrack;
	}
}

void sound_disable_music()
{
	int lastMusic = _gnMusicTrack;

	music_stop();

	_gnMusicTrack = lastMusic;
}

void sound_restart_music()
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
