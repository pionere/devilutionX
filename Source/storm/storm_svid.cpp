#include "storm_svid.h"

//#include <cstddef>
//#include <cstdint>
//#include <cstring>

//#include <SDL.h>

//#include "all.h"
#include "utils/display.h"
#include "utils/sdl_compat.h"
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"
#endif
#include <smacker.h>

#if !SDL_VERSION_ATLEAST(2, 0, 4)
#include <queue>
#endif

DEVILUTION_BEGIN_NAMESPACE

#define SMK_COLORS 256

static double SVidFrameEnd;
static double SVidFrameLength;
static bool SVidLoop;
static smk SVidSMK;
static SDL_Palette* SVidPalette;
static SDL_Surface* SVidSurface;
static BYTE* SVidBuffer;
static unsigned long SVidWidth, SVidHeight;
#ifndef NOSOUND
static BYTE SVidAudioDepth;
/* 1: adjust the volume of a 16bit audio, 0: adjust the volume of a 8bit audio, -1: do not adjust the volume (on max sound or when looping) */
static int8_t SVidAudioAdjust;
#endif

static bool IsLandscapeFit(unsigned long srcW, unsigned long srcH, unsigned long dstW, unsigned long dstH)
{
	return srcW * dstH > dstW * srcH;
}

#ifdef USE_SDL1
// Whether we've changed the video mode temporarily for SVid.
// If true, we must restore it once the video has finished playing.
static bool IsSVidVideoMode = false;

// Set the video mode close to the SVid resolution while preserving aspect ratio.
void TrySetVideoModeToSVidForSDL1()
{
	const SDL_Surface* display = SDL_GetVideoSurface();
#if defined(SDL1_VIDEO_MODE_SVID_FLAGS)
	const int flags = SDL1_VIDEO_MODE_SVID_FLAGS;
#elif defined(SDL1_VIDEO_MODE_FLAGS)
	const int flags = SDL1_VIDEO_MODE_FLAGS;
#else
	const int flags = display->flags;
#endif
#ifdef SDL1_FORCE_SVID_VIDEO_MODE
	IsSVidVideoMode = true;
#else
	IsSVidVideoMode = (flags & (SDL_FULLSCREEN | SDL_NOFRAME)) != 0;
#endif
	if (!IsSVidVideoMode)
		return;

	int w;
	int h;
	if (IsLandscapeFit(SVidWidth, SVidHeight, display->w, display->h)) {
		w = SVidWidth;
		h = SVidWidth * display->h / display->w;
	} else {
		w = SVidHeight * display->w / display->h;
		h = SVidHeight;
	}

#ifndef SDL1_FORCE_SVID_VIDEO_MODE
	if (!SDL_VideoModeOK(w, h, /*bpp=*/display->format->BitsPerPixel, flags)) {
		// Get available fullscreen/hardware modes
		SDL_Rect** modes = SDL_ListModes(NULL, flags);

		// Check is there are any modes available.
		if (modes == NULL
		 || modes == reinterpret_cast<SDL_Rect**>(-1)) { // should not happen, since the first try was rejected...
			return;
		}

		// Search for a usable video mode
		bool found = false;
		for (int i = 0; modes[i] != NULL; i++) {
			if (modes[i]->w == w || modes[i]->h == h) {
				found = true;
				break;
			}
		}
		if (!found)
			return;
		IsSVidVideoMode = true;
	}
#endif

	SetVideoMode(w, h, display->format->BitsPerPixel, flags);
	// Set the background to black.
	SDL_FillRect(GetOutputSurface(), NULL, 0x000000);
}
#endif

#ifndef NOSOUND
#if SDL_VERSION_ATLEAST(2, 0, 4)
SDL_AudioDeviceID deviceId = 0;
static bool HaveAudio()
{
	return deviceId != 0;
}
#else
static bool HaveAudio()
{
	return SDL_GetAudioStatus() != SDL_AUDIO_STOPPED;
}
#endif // SDL_VERSION_ATLEAST

#if !SDL_VERSION_ATLEAST(2, 0, 4)
struct AudioQueueItem {
	unsigned char* data;
	unsigned long len;
	const unsigned char* pos;
};

class AudioQueue {
public:
	static void Callback(void* userdata, Uint8* out, int out_len)
	{
		static_cast<AudioQueue*>(userdata)->Dequeue(out, out_len);
	}

	void Subscribe(SDL_AudioSpec* spec)
	{
		spec->userdata = this;
		spec->callback = AudioQueue::Callback;
	}

	void Enqueue(const unsigned char* data, unsigned long len)
	{
//#if SDL_VERSION_ATLEAST(2, 0, 4)
//		SDL_LockAudioDevice(deviceId);
//		EnqueueUnsafe(data, len);
//		SDL_UnlockAudioDevice(deviceId);
//#else
		SDL_LockAudio();
		EnqueueUnsafe(data, len);
		SDL_UnlockAudio();
//#endif
	}

	void Clear()
	{
		while (!queue_.empty())
			Pop();
	}

private:
	void EnqueueUnsafe(const unsigned char* data, unsigned long len)
	{
		AudioQueueItem item;
		item.data = new unsigned char[len];
		memcpy(item.data, data, len * sizeof(item.data[0]));
		item.len = len;
		item.pos = item.data;
		queue_.push(item);
	}

	void Dequeue(Uint8* out, int out_len)
	{
		AudioQueueItem* item;
		while ((item = Next()) != NULL) {
			if (static_cast<unsigned long>(out_len) <= item->len) {
				memcpy(out, item->pos, out_len);
				item->pos += out_len;
				item->len -= out_len;
				if (item->len == 0)
					Pop();
				return;
			}

			memcpy(out, item->pos, item->len);
			out += item->len;
			out_len -= item->len;
			Pop();
		}
		// fill silence at the end
		SDL_memset(out, SVidAudioDepth == 16 ? 0 : 0x80, out_len);
	}

	AudioQueueItem* Next()
	{
		if (queue_.empty())
			return NULL;
		return &queue_.front();
	}

	void Pop()
	{
		delete[] queue_.front().data;
		queue_.pop();
	}

	std::queue<AudioQueueItem> queue_;
};

static AudioQueue* sVidAudioQueue = new AudioQueue();
#endif
#endif // !NOSOUND

static void SVidUpdatePalette()
{
	SDL_Color* colors = SVidPalette->colors;

	palette_create_sdl_colors(*(SDL_Color(*)[NUM_COLORS])colors, *(BYTE(*)[SMK_COLORS][3])smk_get_palette(SVidSMK));
	ApplyGamma(colors, colors);

#ifdef USE_SDL1
#if SDL1_VIDEO_MODE_BPP == 8
	// When the video surface is 8bit, we need to set the output palette as well.
	SDL_SetColors(SDL_GetVideoSurface(), colors, 0, NUM_COLORS);
#endif
	// In SDL1, the surface always has its own distinct palette, so we need to
	// update it as well.
	if (SDL_SetPalette(SVidSurface, SDL_LOGPAL, colors, 0, NUM_COLORS) <= 0)
		sdl_error(ERR_SDL_VIDEO_SURFACE);
#else // !USE_SDL1
	if (SDL_SetSurfacePalette(SVidSurface, SVidPalette) < 0) {
		sdl_error(ERR_SDL_VIDEO_SURFACE);
	}
#endif
	//if (SDLC_SetSurfaceAndPaletteColors(SVidSurface, SVidPalette, colors, 0, NUM_COLORS) < 0) {
	//	sdl_error(ERR_SDL_VIDEO_SURFACE);
	//}
}

HANDLE SVidPlayBegin(const char* filename, int flags)
{
	//if (flags & (0x10000 | 0x20000000)) {
	//	return NULL;
	//}

	SVidLoop = (flags & MOV_LOOP) != 0; // (flags & 0x40000) != 0;

	bool enableVideo = true; //!(flags & 0x100000);
#ifndef NOSOUND
	bool enableAudio = true; //!(flags & 0x1000000);
#endif
	//0x8 // Non-interlaced
	//0x200, 0x800 // Upscale video
	//0x80000 // Center horizontally
	//0x800000 // Edge detection
	//0x200800 // Clear FB
	size_t dwBytes;

	SVidBuffer = LoadFileInMem(filename, &dwBytes);

	SVidSMK = smk_open_memory(SVidBuffer, dwBytes);
	if (SVidSMK == NULL) {
		MemFreeDbg(SVidBuffer);
		return NULL;
	}

#ifndef NOSOUND
	if (enableAudio && gbSoundOn) {
		unsigned char channels, depth;
		unsigned long rate;
		smk_info_audio(SVidSMK, &channels, &depth, &rate);
		if (depth != 0) {
			smk_enable_audio(SVidSMK, 0, true);

			SVidAudioDepth = depth;
			SVidAudioAdjust = gnSoundVolume == VOLUME_MAX ? -1 : (depth == 16 ? 1 : 0);

			SDL_AudioSpec audioFormat;
			memset(&audioFormat, 0, sizeof(audioFormat));
			audioFormat.freq = rate;
			audioFormat.format = depth == 16 ? AUDIO_S16SYS : AUDIO_U8;
			audioFormat.channels = channels;

			Mix_CloseAudio();

#if SDL_VERSION_ATLEAST(2, 0, 4)
			deviceId = SDL_OpenAudioDevice(NULL, 0, &audioFormat, NULL, SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
			if (deviceId != 0) {
				SDL_PauseAudioDevice(deviceId, 0); /* start audio playing. */
			} else {
				sdl_issue(ERR_SDL_AUDIO_DEVICE_SDL2);
			}
#else
			sVidAudioQueue->Subscribe(&audioFormat);
			if (SDL_OpenAudio(&audioFormat, NULL) == 0) {
				SDL_PauseAudio(0);
			} else {
				sdl_issue(ERR_SDL_AUDIO_DEVICE_SDL1);
			}
#endif
		}
	}
#endif // NOSOUND

	smk_info_all(SVidSMK, NULL, NULL, &SVidFrameLength);
	smk_info_video(SVidSMK, &SVidWidth, &SVidHeight, NULL);

	smk_enable_video(SVidSMK, enableVideo);
	smk_first(SVidSMK); // Decode first frame
#ifdef USE_SDL1
	TrySetVideoModeToSVidForSDL1();
#endif

	// Copy frame to buffer
	SVidSurface = SDL_CreateRGBSurfaceWithFormatFrom(
	    (unsigned char*)smk_get_video(SVidSMK),
	    SVidWidth,
	    SVidHeight,
	    0,
	    SVidWidth,
	    SDL_PIXELFORMAT_INDEX8);
#ifdef USE_SDL1
	SVidPalette = SDL_AllocPalette(NUM_COLORS);
	if (SVidSurface == NULL || SVidPalette == NULL) {
		if (SVidSurface == NULL) {
			sdl_issue(ERR_SDL_VIDEO_CREATE);
		} else {
			sdl_issue(ERR_SDL_VIDEO_PALETTE);
		}
		SVidPlayEnd();
	//} else {
	//	assert(smk_palette_updated(SVidSMK));
	//	SVidUpdatePalette();
	}
#else
	if (SVidSurface == NULL) {
		sdl_issue(ERR_SDL_VIDEO_CREATE);
		SVidPlayEnd();
	} else {
		SVidPalette = SVidSurface->format->palette;
	}
#endif
	SVidFrameEnd = SDL_GetTicks() * 1000.0 + SVidFrameLength;
	return SVidSMK;
}

static bool SVidLoadNextFrame()
{
	char result;

	SVidFrameEnd += SVidFrameLength;

	result = smk_next(SVidSMK);
	if (result != SMK_MORE /* && result != SMK_LAST*/) {
		if (SMK_ERR(result) || !SVidLoop) {
			return false;
		}
#ifndef NOSOUND
		SVidAudioAdjust = -1;
#endif
		smk_first(SVidSMK);
	}

	return true;
}
#ifndef NOSOUND
static BYTE* SVidApplyVolume(BYTE* raw, unsigned long rawLen)
{
	//BYTE* scaled = DiabloAllocPtr(rawLen);
	BYTE* scaled = raw;

	if (SVidAudioAdjust > 0) {
		for (unsigned long i = 0; i < rawLen / 2; i++)
			((Sint16*)scaled)[i] = ADJUST_VOLUME(((Sint16*)raw)[i], 0, gnSoundVolume);
	} else if (SVidAudioAdjust == 0) {
		for (unsigned long i = 0; i < rawLen; i++)
			scaled[i] = ADJUST_VOLUME((raw[i] - 128), 0, gnSoundVolume) + 128;
	}

	return scaled;
}
#endif
bool SVidPlayContinue()
{
	if (smk_palette_updated(SVidSMK)) {
		SVidUpdatePalette();
	}

	if (SDL_GetTicks() * 1000.0 >= SVidFrameEnd) {
		return SVidLoadNextFrame(); // Skip video and audio if the system is too slow
	}
#ifndef NOSOUND
	if (HaveAudio()) {
		unsigned long len = smk_get_audio_size(SVidSMK, 0);
		BYTE* audio = SVidApplyVolume(smk_get_audio(SVidSMK, 0), len);
#if SDL_VERSION_ATLEAST(2, 0, 4)
		if (SDL_QueueAudio(deviceId, audio, len) < 0) {
			sdl_issue(ERR_SDL_VIDEO_AUDIO);
		}
#else
		sVidAudioQueue->Enqueue(audio, len);
#endif
		//mem_free_dbg(audio);
	}
#endif // NOSOUND
	if (SDL_GetTicks() * 1000.0 >= SVidFrameEnd) {
		return SVidLoadNextFrame(); // Skip video if the system is too slow
	}

	SDL_Surface* outputSurface = GetOutputSurface();
	SDL_PixelFormat* outputFormat = outputSurface->format;
#ifdef USE_SDL1
	const bool isIndexedOutputFormat = SDLBackport_IsPixelFormatIndexed(outputFormat);
#else
	const bool isIndexedOutputFormat = SDL_ISPIXELFORMAT_INDEXED(outputFormat->format);
#endif
	SDL_Rect outputRect;
	if (isIndexedOutputFormat) {
		// Cannot scale if the output format is indexed (8-bit palette).
		outputRect.w = static_cast<int>(SVidWidth);
		outputRect.h = static_cast<int>(SVidHeight);
	} else if (IsLandscapeFit(SVidWidth, SVidHeight, outputSurface->w, outputSurface->h)) {
		outputRect.w = outputSurface->w;
		outputRect.h = SVidHeight * outputSurface->w / SVidWidth;
	} else {
		outputRect.w = SVidWidth * outputSurface->h / SVidHeight;
		outputRect.h = outputSurface->h;
	}
	outputRect.x = (outputSurface->w - outputRect.w) >> 1;
	outputRect.y = (outputSurface->h - outputRect.h) >> 1;

	if (isIndexedOutputFormat
	 || outputSurface->w == static_cast<int>(SVidWidth)
	 || outputSurface->h == static_cast<int>(SVidHeight)) {
		if (SDL_BlitSurface(SVidSurface, NULL, outputSurface, &outputRect) < 0) {
			sdl_issue(ERR_SDL_VIDEO_BLIT_B);
			return false;
		}
	} else {
		// The source surface is always 8-bit, and the output surface is never 8-bit in this branch.
		// We must convert to the output format before calling SDL_BlitScaled.
#ifdef USE_SDL1
		SDL_Surface* tmp = SDL_ConvertSurface(SVidSurface, outputFormat, 0);
#else
		SDL_Surface* tmp = SDL_ConvertSurfaceFormat(SVidSurface, outputFormat->format, 0);
#endif
		int result = SDL_BlitScaled(tmp, NULL, outputSurface, &outputRect);
		SDL_FreeSurface(tmp);
		if (result < 0) {
			sdl_issue(ERR_SDL_VIDEO_BLIT_SCALED);
			return false;
		}
	}

	RenderPresent();

	double now = SDL_GetTicks() * 1000.0;
	if (now < SVidFrameEnd) {
		SDL_Delay((Uint32)((SVidFrameEnd - now) / 1000)); // wait with next frame if the system is too fast
	}

	return SVidLoadNextFrame();
}

void SVidPlayEnd()
{
#ifndef NOSOUND
	if (HaveAudio()) {
#if SDL_VERSION_ATLEAST(2, 0, 4)
		SDL_ClearQueuedAudio(deviceId);
		SDL_CloseAudioDevice(deviceId);
		deviceId = 0;
#else
		SDL_CloseAudio();
		sVidAudioQueue->Clear();
#endif
		RestartMixer();
	}
#endif // !NOSOUND
	smk_close(SVidSMK);
	SVidSMK = NULL;

	MemFreeDbg(SVidBuffer);
#ifdef USE_SDL1
	SDL_FreePalette(SVidPalette);
	SVidPalette = NULL;
#endif
	SDL_FreeSurface(SVidSurface);
	SVidSurface = NULL;

#ifdef USE_SDL1
	if (IsSVidVideoMode) {
		SetVideoModeToPrimary(SCREEN_WIDTH, SCREEN_HEIGHT);
		IsSVidVideoMode = false;
	}
#endif
}

DEVILUTION_END_NAMESPACE
