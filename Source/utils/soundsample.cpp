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
bool SoundSample::IsPlaying()
{
	assert(soundData != NULL);

	/*int channels = Mix_AllocateChannels(-1);
	for (int i = 0; i < channels; i++) {
		if (Mix_GetChunk(i) == soundData) {
			return Mix_Playing(i) != 0;
		}
	}

	return false;*/
	int channel = soundData->allocated >> 8;
	return Mix_GetChunk(channel) == soundData && Mix_Playing(channel) != 0;
};

/**
 * @brief Start playing the sound
 */
void SoundSample::Play(int lVolume, int lPan, int channel)
{
	assert(soundData != NULL);

	channel = Mix_PlayChannel(channel, soundData, 0);
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
	Mix_Volume(channel, MIX_VOLUME(lVolume));
	//int panned = 255 - 255 * abs(lPan * 256) / 10000;
	static_assert(((SFX_DIST_MAX + 0) & ((SFX_DIST_MAX + 0) - 1)) == 0, "Adjust ad-hoc panning logic for better performance.");
	int panned = 255 - 255 * abs(lPan) / (SFX_DIST_MAX + 0);
	Mix_SetPanning(channel, lPan < 0 ? 255 : panned, lPan < 0 ? panned : 255);

	soundData->allocated = (channel << 8) | 1;
};

/**
 * @brief Stop playing the sound on one channel!
 */
void SoundSample::Stop()
{
	assert(soundData != NULL);

	//int channels = Mix_AllocateChannels(-1);
	int channels = SND_NUM_CHANNELS;
	for (int i = 0; i < channels; i++) {
		if (Mix_GetChunk(i) == soundData) {
			// Mix_UnregisterAllEffects(i);
			Mix_HaltChannel(i);
			break;
		}
	}
};

/*
 * Load a wave file
 *
 * Taken from SDL_mixer/mixer.c, adjusted for internal use.
 *  - Support only WAV files
 *  - use SND_DEFAULT_* defines instead of the mixer's properties to check if conversion is necessary
 *  - allocated field is misused to store the last channel in the higher bits
 */
static Mix_Chunk *Mix_LoadWAV_RW_priv(SDL_RWops *src, int freesrc)
{
    Uint8 magic[4];
    Mix_Chunk *chunk;
    SDL_AudioSpec wavespec, *loaded;
    SDL_AudioCVT wavecvt;
    int samplesize;
#ifdef FULL
    /* rcg06012001 Make sure src is valid */
    if (!src) {
        SDL_SetError("Mix_LoadWAV_RW with NULL src");
        return(NULL);
    }
    /* Make sure audio has been opened */
    if (!audio_opened) {
        SDL_SetError("Audio device hasn't been opened");
        if (freesrc) {
            SDL_RWclose(src);
        }
        return(NULL);
    }
#endif
    /* Allocate the chunk memory */
    chunk = (Mix_Chunk *)SDL_malloc(sizeof(Mix_Chunk));
    if (chunk == NULL) {
        SDL_SetError("Out of memory");
        if (freesrc) {
            SDL_RWclose(src);
        }
        return(NULL);
    }

    /* Find out what kind of audio file this is */
    if (SDL_RWread(src, magic, 1, 4) != 4) {
        SDL_free(chunk);
        if (freesrc) {
            SDL_RWclose(src);
        }
        Mix_SetError("Couldn't read first 4 bytes of audio data");
        return NULL;
    }
    /* Seek backwards for compatibility with older loaders */
    SDL_RWseek(src, -4, RW_SEEK_CUR);
    if (SDL_memcmp(magic, "WAVE", 4) == 0 || SDL_memcmp(magic, "RIFF", 4) == 0) {
        loaded = SDL_LoadWAV_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
#ifdef FULL
    } else if (SDL_memcmp(magic, "FORM", 4) == 0) {
        loaded = Mix_LoadAIFF_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
    } else if (SDL_memcmp(magic, "Crea", 4) == 0) {
        loaded = Mix_LoadVOC_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
    } else {
        loaded = Mix_LoadMusic_RW(src, freesrc, &wavespec, (Uint8 **)&chunk->abuf, &chunk->alen);
#else
	} else {
		SDL_SetError("Unsupported format");
		if (freesrc) {
			SDL_RWclose(src);
		}
		loaded = NULL;
#endif
    }
    if (!loaded) {
        /* The individual loaders have closed src if needed */
        SDL_free(chunk);
        return(NULL);
    }
#if 0
    PrintFormat("Audio device", &mixer);
    PrintFormat("-- Wave file", &wavespec);
#endif
    /* Build the audio converter and create conversion buffers */
    if (wavespec.format != SND_DEFAULT_FORMAT || //mixer.format ||
         wavespec.channels != SND_DEFAULT_CHANNELS || //mixer.channels ||
         wavespec.freq != SND_DEFAULT_FREQUENCY) { //mixer.freq) {
        if (SDL_BuildAudioCVT(&wavecvt,
                wavespec.format, wavespec.channels, wavespec.freq,
                //mixer.format, mixer.channels, mixer.freq) < 0) {
				SND_DEFAULT_FORMAT, SND_DEFAULT_CHANNELS, SND_DEFAULT_FREQUENCY) < 0) {
            SDL_free(chunk->abuf);
            SDL_free(chunk);
            return(NULL);
        }
        samplesize = ((wavespec.format & 0xFF)/8)*wavespec.channels;
        wavecvt.len = chunk->alen & ~(samplesize-1);
        wavecvt.buf = (Uint8 *)SDL_calloc(1, wavecvt.len*wavecvt.len_mult);
        if (wavecvt.buf == NULL) {
            SDL_SetError("Out of memory");
            SDL_free(chunk->abuf);
            SDL_free(chunk);
            return(NULL);
        }
        SDL_memcpy(wavecvt.buf, chunk->abuf, wavecvt.len);
        SDL_free(chunk->abuf);
        /* Run the audio converter */
        if (SDL_ConvertAudio(&wavecvt) < 0) {
            SDL_free(wavecvt.buf);
            SDL_free(chunk);
            return(NULL);
        }
        chunk->abuf = wavecvt.buf;
        chunk->alen = wavecvt.len_cvt;
    }
    chunk->allocated = 1;
    chunk->volume = MIX_MAX_VOLUME;
    return(chunk);
}

/**
 * @brief This can load WAVE, AIFF, RIFF, OGG, and VOC formats
 * @param fileData Buffer containing file data
 * @param dwBytes Length of buffer
 */
void SoundSample::SetChunk(BYTE* fileData, DWORD dwBytes)
{
	SDL_RWops* buf = SDL_RWFromConstMem(fileData, dwBytes);
	if (buf != NULL) {
		//soundData = Mix_LoadWAV_RW(buf, 1);
		soundData = Mix_LoadWAV_RW_priv(buf, 1);
		if (soundData != NULL)
			return; // 0;
	}

	sdl_fatal(ERR_SDL_SOUND_FILE);
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
