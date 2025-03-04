#include "soundsample.h"

#ifndef NOSOUND

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

///// SoundSample /////

void SoundSample::Release()
{
	if (soundData == NULL)
		return;
	assert(!IsPlaying());
	//Stop();
	Mix_FreeChunk(soundData);
	soundData = NULL;
};

/**
 * @brief Check if a the sound is being played atm
 */
bool SoundSample::IsPlaying() const
{
	assert(soundData != NULL);

	/*int channels = Mix_AllocateChannels(-1);
	for (int i = 0; i < channels; i++) {
		if (Mix_GetChunk(i) == soundData) {
			return Mix_Playing(i) != 0;
		}
	}

	return false;*/
	return Mix_PlayingChunk(soundData);
};

/**
 * @brief Start playing the sound
 */
void SoundSample::Play(int lVolume, int lPan, int channel)
{
	assert(soundData != NULL);

	channel = Mix_PlayChannel(channel, soundData, 0);
	if (channel == -1) {
		DoLog(Mix_GetError());
		return;
	}
	/*lVolume = (int)(pow(10.0, lVolume / 2000.0) * MIX_MAX_VOLUME);
	Mix_Volume(channel, lVolume);
	int sign = lPan;
	lPan = (int)(pow(10.0, -abs(lPan) / 2000.0) * MIX_MAX_POS_EFFECT);
	Uint8 left, right;
	left = right = MIX_MAX_POS_EFFECT;
	if (lPan != 0) {
		if (sign < 0)
			right = lPan;
		else
			left = lPan;
	}
	Mix_SetPanning(channel, left, right);*/
	Mix_Volume(channel, MIX_VOLUME(lVolume));
	//int panned = MIX_MAX_POS_EFFECT - MIX_MAX_POS_EFFECT * abs(lPan * 256) / 10000;
	static_assert(((SFX_DIST_MAX + 0) & ((SFX_DIST_MAX + 0) - 1)) == 0, "Adjust ad-hoc panning logic for better performance.");
	int panned = MIX_MAX_POS_EFFECT - MIX_MAX_POS_EFFECT * abs(lPan) / (SFX_DIST_MAX + 0);
	Mix_SetPanning(channel, lPan < 0 ? MIX_MAX_POS_EFFECT : panned, lPan < 0 ? panned : MIX_MAX_POS_EFFECT);
};

/**
 * @brief This can load WAVE, AIFF, RIFF, OGG, and VOC formats
 * @param fileData Buffer containing file data
 * @param dwBytes Length of buffer
 * @param stream whether the sfx is going to be streamed
 */
void SoundSample::SetChunk(BYTE* fileData, size_t dwBytes, bool stream)
{
	Mix_RWops buf;

	Mix_RWFromMem(&buf, fileData, dwBytes);
#if STREAM_ALL_AUDIO
	soundData = Mix_LoadWAV_RW(&buf, SDL_TRUE);
#else
	soundData = Mix_LoadWAV_RW(&buf, stream ? SDL_TRUE : SDL_FALSE);
#endif
	if (soundData != NULL)
		return; // 0;

	sdl_error(ERR_SDL_SOUND_FILE);
	//return -1;
};

/*int SoundSample::TrackLength()
{
	int chans = SND_DEFAULT_CHANNELS;
	int freq = SND_DEFAULT_FREQUENCY;
	Uint32 points, frames;

	// bytes / samplesize == sample points
	points = (soundData->alen / ((AUDIO_S16LSB & 0xFF) / 8));

	// sample points / channels == sample frames
	frames = (points / chans);

	// (sample frames * 1000) / frequency == play length in ms
	return (frames * 1000) / freq;
}*/

DEVILUTION_END_NAMESPACE

#endif // NOSOUND
