#pragma once

#ifndef NOSOUND

#include <SDL_mixer.h>
#include "miniwin/miniwin.h"

namespace dvl {

typedef struct SoundSample final {
public:
	void Release();
	bool IsPlaying();
	void Play(int lVolume, int lPan, int channel = -1);
	void Stop();
	int SetChunk(BYTE *fileData, DWORD dwBytes);

private:
	Mix_Chunk *chunk;
} SoundSample;

} // namespace dvl
#else
typedef struct SoundSample {
} SoundSample;
#endif // NOSOUND
