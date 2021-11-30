#include "soundsample.h"

#ifndef NOSOUND

#include <SDL.h>
#ifdef USE_SDL1
#include "utils/sdl2_to_1_2_backports.h"
#else
#include "utils/sdl2_backports.h"
#endif
#include "../types.h"
#include "appfat.h"

DEVILUTION_BEGIN_NAMESPACE

///// SoundSample /////

void SoundSample::Release()
{
	Mix_FreeChunk(chunk);
};

/**
 * @brief Check if a the sound is being played atm
 */
bool SoundSample::IsPlaying()
{
	assert(chunk != NULL);

	int channels = Mix_AllocateChannels(-1);
	for (int i = 0; i < channels; i++) {
		if (Mix_GetChunk(i) == chunk) {
			return Mix_Playing(i) != 0;
		}
	}

	return false;
};

/**
 * @brief Start playing the sound
 */
void SoundSample::Play(int lVolume, int lPan, int channel)
{
	assert(chunk != NULL);

	channel = Mix_PlayChannel(channel, chunk, 0);
	if (channel == -1) {
		SDL_Log("Too few channels, skipping sound");
		return;
	}
	/*lVolume = (int)(pow(10.0, lVolume / 2000.0) * MIX_MAX_VOLUME);
	Mix_Volume(channel, lVolume);
	int sign = lPan;
	lPan = (int)(pow(10.0, -abs(lPan) / 2000.0) * 255);
	Uint8 left, right;
	left = right = 255;
	if (lPan != 0) {
		if (sign < 0)
			right = lPan;
		else
			left = lPan;
	}
	Mix_SetPanning(channel, left, right);*/
	Mix_Volume(channel, MIX_MAX_VOLUME * (lVolume - VOLUME_MIN) / (VOLUME_MAX - VOLUME_MIN));
	//int panned = 255 - 255 * abs(lPan * 256) / 10000;
	static_assert(((SFX_DIST_MAX + 7) & ((SFX_DIST_MAX + 7) - 1)) == 0, "Adjust ad-hoc panning logic for better performance.");
	int panned = 255 - 255 * abs(lPan) / (SFX_DIST_MAX + 7);
	Mix_SetPanning(channel, lPan < 0 ? 255 : panned, lPan < 0 ? panned : 255);
};

/**
 * @brief Stop playing the sound
 */
void SoundSample::Stop()
{
	assert(chunk != NULL);

	int channels = Mix_AllocateChannels(-1);
	for (int i = 0; i < channels; i++) {
		if (Mix_GetChunk(i) == chunk) {
			Mix_HaltChannel(i);
			break;
		}
	}
};

/**
 * @brief This can load WAVE, AIFF, RIFF, OGG, and VOC formats
 * @param fileData Buffer containing file data
 * @param dwBytes Length of buffer
 * @return 0 on success, -1 otherwise
 */
int SoundSample::SetChunk(BYTE *fileData, DWORD dwBytes)
{
	SDL_RWops *buf1 = SDL_RWFromConstMem(fileData, dwBytes);
	if (buf1 == NULL) {
		return -1;
	}

	chunk = Mix_LoadWAV_RW(buf1, 1);
	if (chunk == NULL) {
		return -1;
	}

	return 0;
};

DEVILUTION_END_NAMESPACE

#endif // NOSOUND
