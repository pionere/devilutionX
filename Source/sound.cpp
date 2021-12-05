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
/** Mix_Music entity of the background music */
Mix_Music* _gMusic;
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
	HANDLE file;
	BYTE* wave_file;
	DWORD dwBytes;

	file = SFileOpenFile(path);

	dwBytes = SFileGetFileSize(file);
	wave_file = DiabloAllocPtr(dwBytes);
	SFileReadFile(file, wave_file, dwBytes, NULL);
	SFileCloseFile(file);

	pSnd->nextTc = 0;
	pSnd->SetChunk(wave_file, dwBytes);
	mem_free_dbg(wave_file);
}

void RestartMixer()
{
	int chans;

	if (Mix_OpenAudio(SND_DEFAULT_FREQUENCY, SND_DEFAULT_FORMAT, SND_DEFAULT_CHANNELS, 1024) < 0) {
		SDL_Log("%s", Mix_GetError());
	}
	chans = Mix_AllocateChannels(SND_NUM_CHANNELS);
	assert(chans == SND_NUM_CHANNELS);
	Mix_ReserveChannels(1); // reserve one channel for narration (SFileDda*)
}

void InitSound()
{
	snd_get_volume("Sound Volume", &gnSoundVolume);
	gbSoundOn = gnSoundVolume > VOLUME_MIN;

	snd_get_volume("Music Volume", &gnMusicVolume);
	gbMusicOn = gnMusicVolume > VOLUME_MIN;

	RestartMixer();
}

void music_stop()
{
	if (_gMusic != NULL) {
		Mix_HaltMusic();
		Mix_FreeMusic(_gMusic);
		_gMusic = NULL;
		_gnMusicTrack = NUM_MUSIC;
		MemFreeDbg(_gMusicBuffer);
	}
}

void music_start(int nTrack)
{
	assert((unsigned)nTrack < NUM_MUSIC);
	if (gbMusicOn) {
		music_stop();
		HANDLE hMusic = SFileOpenFile(sgszMusicTracks[nTrack]);
		if (hMusic != NULL) {
			DWORD bytestoread = SFileGetFileSize(hMusic);
			assert(_gMusicBuffer == NULL);
			_gMusicBuffer = DiabloAllocPtr(bytestoread);
			SFileReadFile(hMusic, _gMusicBuffer, bytestoread, NULL);

			SDL_RWops* musicRw = SDL_RWFromConstMem(_gMusicBuffer, bytestoread);
			if (musicRw == NULL) {
				sdl_fatal(ERR_SDL_MUSIC_FILE);
			}
			assert(_gMusic == NULL);
			_gMusic = Mix_LoadMUSType_RW(musicRw, MUS_NONE, 1);
			Mix_VolumeMusic(MIX_VOLUME(gnMusicVolume));
			Mix_PlayMusic(_gMusic, -1);

			_gnMusicTrack = nTrack;
		}
		SFileCloseFile(hMusic);
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

	if (_gMusic != NULL)
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
