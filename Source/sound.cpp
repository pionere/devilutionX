/**
 * @file sound.cpp
 *
 * Implementation of functions setting up the audio pipeline.
 */
#include "all.h"
#include <SDL.h>
#ifndef NOSOUND
#include <SDL_mixer.h>
#endif

DEVILUTION_BEGIN_NAMESPACE

/** Specifies whether sound effects are enabled. */
bool gbSoundOn = false;
/** Specifies whether music effects are enabled. */
bool gbMusicOn = false;

#ifndef NOSOUND
/** Mix_Music entity of the background music */
Mix_Music* _gMusic;
/** Buffer containing the data of the background music. */
BYTE* _gMusicBuffer;

/** The volume of the sound channel. */
int _gnSoundVolume;
/** The volume of the music channel. */
int _gnMusicVolume;
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

bool snd_playing(TSnd* pSnd)
{
	if (pSnd == NULL)
		return false;
	assert(pSnd->DSB != NULL);
	return pSnd->DSB->IsPlaying();
}

void snd_play_snd(TSnd* pSnd, int lVolume, int lPan)
{
	Uint32 currTc;

	assert(pSnd != NULL);
	assert(gbSoundOn);

	currTc = SDL_GetTicks();
	if (currTc < pSnd->nextTc)
		return;
	pSnd->nextTc = currTc + 80;

	lVolume += _gnSoundVolume;
	if (lVolume < VOLUME_MIN) {
		lVolume = VOLUME_MIN;
	} else if (lVolume > VOLUME_MAX) {
		lVolume = VOLUME_MAX;
	}
	assert(pSnd->DSB != NULL);
	pSnd->DSB->Play(lVolume, lPan);
}

TSnd* sound_file_load(const char* path)
{
	HANDLE file;
	BYTE* wave_file;
	TSnd* pSnd;
	DWORD dwBytes;
	int error;

	file = SFileOpenFile(path);
	pSnd = (TSnd*)DiabloAllocPtr(sizeof(TSnd));
	memset(pSnd, 0, sizeof(TSnd));
	pSnd->nextTc = 0;

	dwBytes = SFileGetFileSize(file);
	wave_file = DiabloAllocPtr(dwBytes);
	SFileReadFile(file, wave_file, dwBytes, NULL);

	pSnd->DSB = new SoundSample();
	error = pSnd->DSB->SetChunk(wave_file, dwBytes);
	SFileCloseFile(file);
	mem_free_dbg(wave_file);
	if (error != 0) {
		sdl_fatal(ERR_SDL_SOUND_FILE);
	}

	return pSnd;
}

void sound_file_cleanup(TSnd* sound_file)
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
	snd_get_volume("Sound Volume", &_gnSoundVolume);
	gbSoundOn = _gnSoundVolume > VOLUME_MIN;

	snd_get_volume("Music Volume", &_gnMusicVolume);
	gbMusicOn = _gnMusicVolume > VOLUME_MIN;

	if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0) {
		SDL_Log("%s", Mix_GetError());
	}
	Mix_AllocateChannels(25);
	Mix_ReserveChannels(1); // reserve one channel for narration (SFileDda*)
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
			Mix_VolumeMusic(MIX_MAX_VOLUME - MIX_MAX_VOLUME * _gnMusicVolume / VOLUME_MIN);
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

int sound_get_music_volume()
{
	return _gnMusicVolume;
}

void sound_set_music_volume(int volume)
{
	_gnMusicVolume = volume;
	gbMusicOn = volume > VOLUME_MIN;
	setIniInt("Audio", "Music Volume", volume);

	if (_gMusic != NULL)
		Mix_VolumeMusic(MIX_MAX_VOLUME - MIX_MAX_VOLUME * volume / VOLUME_MIN);
}

int sound_get_sound_volume()
{
	return _gnSoundVolume;
}

void sound_set_sound_volume(int volume)
{
	_gnSoundVolume = volume;
	gbSoundOn = volume > VOLUME_MIN;
	setIniInt("Audio", "Sound Volume", volume);
}

#endif // NOT NOSOUND

DEVILUTION_END_NAMESPACE
