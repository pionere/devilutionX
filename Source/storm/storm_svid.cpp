#include "storm_svid.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <SDL.h>
#ifndef NOSOUND
#include <SDL_mixer.h>
#endif
#include <smacker.h>

#include "all.h"
#include "utils/display.h"
#include "utils/sdl_compat.h"

#if !SDL_VERSION_ATLEAST(2, 0, 4)
#include <queue>
#endif

DEVILUTION_BEGIN_NAMESPACE

static double SVidFrameEnd;
static double SVidFrameLength;
static bool SVidLoop;
static smk SVidSMK;
static SDL_Color SVidPreviousPalette[256];
static SDL_Palette *SVidPalette;
static SDL_Surface *SVidSurface;
static BYTE *SVidBuffer;
static unsigned long SVidWidth, SVidHeight;
static BYTE SVidAudioDepth;
static double SVidVolume;
//int SVidVolume;

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
	const SDL_Surface *display = SDL_GetVideoSurface();
	IsSVidVideoMode = (display->flags & (SDL_FULLSCREEN | SDL_NOFRAME)) != 0;
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
	IsSVidVideoMode = SDL_VideoModeOK(
	    w, h, /*bpp=*/display->format->BitsPerPixel, display->flags);

	if (!IsSVidVideoMode) {
		// Get available fullscreen/hardware modes
		SDL_Rect **modes = SDL_ListModes(NULL, display->flags);

		// Check is there are any modes available.
		if (modes == NULL)
		    || modes == reinterpret_cast<SDL_Rect **>(-1)) { // should not happen, since the first try was rejected...
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

	SetVideoMode(w, h, display->format->BitsPerPixel, display->flags);
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

static void SVidRestartMixer()
{
	if (Mix_OpenAudio(22050, AUDIO_S16LSB, 2, 1024) < 0) {
		SDL_Log("%s", Mix_GetError());
	}
	Mix_AllocateChannels(25);
	Mix_ReserveChannels(1);
}
#if !SDL_VERSION_ATLEAST(2, 0, 4)
struct AudioQueueItem {
	unsigned char *data;
	unsigned long len;
	const unsigned char *pos;
};

class AudioQueue {
public:
	static void Callback(void *userdata, Uint8 *out, int out_len)
	{
		static_cast<AudioQueue *>(userdata)->Dequeue(out, out_len);
	}

	void Subscribe(SDL_AudioSpec *spec)
	{
		spec->userdata = this;
		spec->callback = AudioQueue::Callback;
	}

	void Enqueue(const unsigned char *data, unsigned long len)
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
	void EnqueueUnsafe(const unsigned char *data, unsigned long len)
	{
		AudioQueueItem item;
		item.data = new unsigned char[len];
		memcpy(item.data, data, len * sizeof(item.data[0]));
		item.len = len;
		item.pos = item.data;
		queue_.push(item);
	}

	void Dequeue(Uint8 *out, int out_len)
	{
		SDL_memset(out, 0, sizeof(out[0]) * out_len);
		AudioQueueItem *item;
		while ((item = Next()) != NULL) {
			if (static_cast<unsigned long>(out_len) <= item->len) {
				SDL_MixAudio(out, item->pos, out_len, SDL_MIX_MAXVOLUME);
				item->pos += out_len;
				item->len -= out_len;
				return;
			}

			SDL_MixAudio(out, item->pos, item->len, SDL_MIX_MAXVOLUME);
			out += item->len;
			out_len -= item->len;
			Pop();
		}
	}

	AudioQueueItem *Next()
	{
		while (!queue_.empty() && queue_.front().len == 0)
			Pop();
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

static AudioQueue *sVidAudioQueue = new AudioQueue();
#endif
#endif // NOSOUND

HANDLE SVidPlayBegin(const char *filename, int flags)
{
	if (flags & (0x10000 | 0x20000000)) {
		return NULL;
	}

	SVidLoop = (flags & 0x40000) != 0;
	bool enableVideo = !(flags & 0x100000);
	bool enableAudio = !(flags & 0x1000000);
	//0x8 // Non-interlaced
	//0x200, 0x800 // Upscale video
	//0x80000 // Center horizontally
	//0x800000 // Edge detection
	//0x200800 // Clear FB

	HANDLE videoFile = SFileOpenFile(filename);

	DWORD bytestoread = SFileGetFileSize(videoFile);
	SVidBuffer = DiabloAllocPtr(bytestoread);
	SFileReadFile(videoFile, SVidBuffer, bytestoread, NULL);
	SFileCloseFile(videoFile);

	SVidSMK = smk_open_memory(SVidBuffer, bytestoread);
	if (SVidSMK == NULL) {
		return NULL;
	}

#ifndef NOSOUND
	if (enableAudio) {
		unsigned char channels[7], depth[7];
		unsigned long rate[7];
		smk_info_audio(SVidSMK, NULL, channels, depth, rate);
		if (depth[0] != 0) {
			SVidAudioDepth = depth[0];
			SVidVolume = sound_get_sound_volume() - VOLUME_MIN;
			SVidVolume /= -VOLUME_MIN;

			smk_enable_audio(SVidSMK, 0, true);
			SDL_AudioSpec audioFormat;
			memset(&audioFormat, 0, sizeof(audioFormat));
			audioFormat.freq = rate[0];
			audioFormat.format = SVidAudioDepth == 16 ? AUDIO_S16SYS : AUDIO_U8;
			audioFormat.channels = channels[0];

			Mix_CloseAudio();

#if SDL_VERSION_ATLEAST(2, 0, 4)
			deviceId = SDL_OpenAudioDevice(NULL, 0, &audioFormat, NULL, 0);
			if (deviceId == 0) {
				ErrSdl();
			}

			SDL_PauseAudioDevice(deviceId, 0); /* start audio playing. */
#else
			sVidAudioQueue->Subscribe(&audioFormat);
			if (SDL_OpenAudio(&audioFormat, NULL) != 0) {
				ErrSdl();
			}
			SDL_PauseAudio(0);
#endif
		}
	}
#endif // NOSOUND

	smk_info_all(SVidSMK, NULL, NULL, &SVidFrameLength);
	smk_info_video(SVidSMK, &SVidWidth, &SVidHeight, NULL);

	smk_enable_video(SVidSMK, enableVideo);
	smk_first(SVidSMK); // Decode first frame
#ifndef USE_SDL1
	if (renderer != NULL) {
		SDL_DestroyTexture(renderer_texture);
		renderer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, SVidWidth, SVidHeight);
		if (renderer_texture == NULL) {
			ErrSdl();
		}
		if (SDL_RenderSetLogicalSize(renderer, SVidWidth, SVidHeight) < 0) {
			ErrSdl();
		}
	}
#else
	TrySetVideoModeToSVidForSDL1();
#endif
	memcpy(SVidPreviousPalette, orig_palette, sizeof(SVidPreviousPalette));

	// Copy frame to buffer
	SVidSurface = SDL_CreateRGBSurfaceWithFormatFrom(
	    (unsigned char *)smk_get_video(SVidSMK),
	    SVidWidth,
	    SVidHeight,
	    8,
	    SVidWidth,
	    SDL_PIXELFORMAT_INDEX8);
	if (SVidSurface == NULL) {
		ErrSdl();
	}

	SVidPalette = SDL_AllocPalette(256);
	if (SVidPalette == NULL) {
		ErrSdl();
	}
#ifndef USE_SDL1
	if (SDL_SetSurfacePalette(SVidSurface, SVidPalette) < 0) {
		ErrSdl();
	}
#else
	if (SDLC_SetSurfaceColors(SVidSurface, SVidPalette) < 0) {
		ErrSdl();
	}
#endif

	SVidFrameEnd = SDL_GetTicks() * 1000.0 + SVidFrameLength;
	SDL_FillRect(GetOutputSurface(), NULL, 0x000000);
	return SVidSMK;
}

static bool SVidLoadNextFrame()
{
	SVidFrameEnd += SVidFrameLength;

	if (smk_next(SVidSMK) == SMK_DONE) {
		if (!SVidLoop) {
			return false;
		}

		smk_first(SVidSMK);
	}

	return true;
}

static BYTE *SVidApplyVolume(const BYTE *raw, unsigned long rawLen)
{
	BYTE *scaled = DiabloAllocPtr(rawLen);

	// TODO: use SDL_MixAudio(Format) instead?
	//SVidVolume = MIX_MAX_VOLUME - MIX_MAX_VOLUME * sound_get_sound_volume() / VOLUME_MIN;
	//SDL_MixAudio(scaled, raw, rawLen, SVidVolume);
	//SDL_MixAudioFormat(scaled, raw, audioFormat, rawLen, SVidVolume);
	if (SVidAudioDepth == 16) {
		for (unsigned long i = 0; i < rawLen / 2; i++)
			((Sint16 *)scaled)[i] = ((Sint16 *)raw)[i] * SVidVolume;
	} else {
		for (unsigned long i = 0; i < rawLen; i++)
			scaled[i] = raw[i] * SVidVolume;
	}

	return scaled;
}

bool SVidPlayContinue()
{
	if (smk_palette_updated(SVidSMK)) {
		SDL_Color colors[256];
		const unsigned char *paletteData = smk_get_palette(SVidSMK);

		for (int i = 0; i < 256; i++) {
			colors[i].r = paletteData[i * 3 + 0];
			colors[i].g = paletteData[i * 3 + 1];
			colors[i].b = paletteData[i * 3 + 2];
#ifndef USE_SDL1
			colors[i].a = SDL_ALPHA_OPAQUE;
#endif

			orig_palette[i].r = paletteData[i * 3 + 0];
			orig_palette[i].g = paletteData[i * 3 + 1];
			orig_palette[i].b = paletteData[i * 3 + 2];
		}
		memcpy(logical_palette, orig_palette, sizeof(logical_palette));

		if (SDLC_SetSurfaceAndPaletteColors(SVidSurface, SVidPalette, colors, 0, 256) < 0) {
			SDL_Log("%s", SDL_GetError());
			return false;
		}
	}

	if (SDL_GetTicks() * 1000.0 >= SVidFrameEnd) {
		return SVidLoadNextFrame(); // Skip video and audio if the system is to slow
	}
#ifndef NOSOUND
	if (HaveAudio()) {
		unsigned long len = smk_get_audio_size(SVidSMK, 0);
		BYTE *audio = SVidApplyVolume(smk_get_audio(SVidSMK, 0), len);
#if SDL_VERSION_ATLEAST(2, 0, 4)
		if (SDL_QueueAudio(deviceId, audio, len) < 0) {
			SDL_Log("%s", SDL_GetError());
			return false;
		}
#else
		sVidAudioQueue->Enqueue(audio, len);
#endif
		mem_free_dbg(audio);
	}
#endif // NOSOUND
	if (SDL_GetTicks() * 1000.0 >= SVidFrameEnd) {
		return SVidLoadNextFrame(); // Skip video if the system is to slow
	}

#ifndef USE_SDL1
	if (renderer != NULL) {
		if (SDL_BlitSurface(SVidSurface, NULL, GetOutputSurface(), NULL) < 0) {
			SDL_Log("%s", SDL_GetError());
			return false;
		}
	} else
#endif
	{
		SDL_Surface *outputSurface = GetOutputSurface();
#ifdef USE_SDL1
		const bool isIndexedOutputFormat = SDLBackport_IsPixelFormatIndexed(outputSurface->format);
#else
		const Uint32 wndFormat = SDL_GetWindowPixelFormat(ghMainWnd);
		const bool isIndexedOutputFormat = SDL_ISPIXELFORMAT_INDEXED(wndFormat);
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
		outputRect.x = (outputSurface->w - outputRect.w) / 2;
		outputRect.y = (outputSurface->h - outputRect.h) / 2;

		if (isIndexedOutputFormat
		    || outputSurface->w == static_cast<int>(SVidWidth)
		    || outputSurface->h == static_cast<int>(SVidHeight)) {
			if (SDL_BlitSurface(SVidSurface, NULL, outputSurface, &outputRect) < 0) {
				ErrSdl();
			}
		} else {
			// The source surface is always 8-bit, and the output surface is never 8-bit in this branch.
			// We must convert to the output format before calling SDL_BlitScaled.
#ifdef USE_SDL1
			SDL_Surface *tmp = SDL_ConvertSurface(SVidSurface, ghMainWnd->format, 0);
#else
			SDL_Surface *tmp = SDL_ConvertSurfaceFormat(SVidSurface, wndFormat, 0);
#endif
			if (SDL_BlitScaled(tmp, NULL, outputSurface, &outputRect) < 0) {
				SDL_Log("%s", SDL_GetError());
				return false;
			}
			SDL_FreeSurface(tmp);
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
		SVidRestartMixer();
	}
#endif // NOSOUND
	if (SVidSMK != NULL)
		smk_close(SVidSMK);

	MemFreeDbg(SVidBuffer);

	SDL_FreePalette(SVidPalette);
	SVidPalette = NULL;

	SDL_FreeSurface(SVidSurface);
	SVidSurface = NULL;

	memcpy(orig_palette, SVidPreviousPalette, sizeof(orig_palette));
#ifndef USE_SDL1
	if (renderer != NULL) {
		SDL_DestroyTexture(renderer_texture);
		renderer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
		if (renderer_texture == NULL) {
			ErrSdl();
		}
		if (SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT) < 0) {
			ErrSdl();
		}
	}
#else
	if (IsSVidVideoMode) {
		SetVideoModeToPrimary(IsFullScreen(), SCREEN_WIDTH, SCREEN_HEIGHT);
		IsSVidVideoMode = false;
	}
#endif
}

DEVILUTION_END_NAMESPACE
