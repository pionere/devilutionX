/**
 * @file capture.cpp
 *
 * Implementation of the screenshot function.
 */
#include <fstream>

#include "all.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

#define PCX_COLORS	256

/**
 * @brief Write the PCX-file header
 * @param width Image width
 * @param height Image height
 * @param out File stream to write to
 * @return True on success
 */
static bool CaptureHdr(uint16_t width, uint16_t height, std::ofstream *out)
{
	FilePcxHeader Buffer;

	memset(&Buffer, 0, sizeof(Buffer));
	Buffer.Manufacturer = 10;
	Buffer.Version = 5;
	Buffer.Encoding = 1;
	Buffer.BitsPerPixel = 8;
	Buffer.Xmax = static_cast<uint16_t>(width - 1);
	Buffer.Ymax = static_cast<uint16_t>(height - 1);
	Buffer.HDpi = width;
	Buffer.VDpi = height;
	Buffer.NPlanes = 1;
	Buffer.BytesPerLine = width;

	out->write(reinterpret_cast<const char *>(&Buffer), sizeof(Buffer));
	return !out->fail();
}

/**
 * @brief Write the current in-game palette to the PCX file
 * @param palette Current palette
 * @param out File stream for the PCX file.
 * @return True if successful, else false
 */
static bool CapturePal(SDL_Color (&palette)[NUM_COLORS], std::ofstream *out)
{
	BYTE pcx_palette[1 + PCX_COLORS * 3];
	int i;
	static_assert(NUM_COLORS == PCX_COLORS, "Mismatching PCX and game palette.");
	pcx_palette[0] = 12;
	for (i = 0; i < PCX_COLORS; i++) {
		pcx_palette[1 + 3 * i + 0] = palette[i].r;
		pcx_palette[1 + 3 * i + 1] = palette[i].g;
		pcx_palette[1 + 3 * i + 2] = palette[i].b;
	}

	out->write(reinterpret_cast<const char *>(pcx_palette), sizeof(pcx_palette));
	return !out->fail();
}

/**
 * @brief RLE compress the pixel data
 * @param src Raw pixel buffer
 * @param dst Output buffer
 * @param width Width of pixel buffer

 * @return Output buffer
 */
static BYTE *CaptureEnc(BYTE *src, BYTE *dst, int width)
{
	int rleLength;

	do {
		BYTE rlePixel = *src;
		src++;
		rleLength = 1;

		width--;

		while (rlePixel == *src) {
			if (rleLength >= 63)
				break;
			if (width == 0)
				break;
			rleLength++;

			width--;
			src++;
		}

		if (rleLength > 1 || rlePixel > 0xBF) {
			*dst = rleLength | 0xC0;
			dst++;
		}

		*dst = rlePixel;
		dst++;
	} while (width);

	return dst;
}

/**
 * @brief Write the pixel data to the PCX file
 * @param width Image width
 * @param height Image height
 * @param stride Buffer width
 * @param pixels Raw pixel buffer
 * @return True if successful, else false
 */
static bool CapturePix(uint16_t width, uint16_t height, uint16_t stride, BYTE *pixels, std::ofstream *out)
{
	int i, writeSize;
	BYTE *pBuffer, *pBufferEnd;

	pBuffer = (BYTE *)DiabloAllocPtr(2 * width);
	for (i = height; i > 0; i--) {
		pBufferEnd = CaptureEnc(pixels, pBuffer, width);
		pixels += stride;
		writeSize = pBufferEnd - pBuffer;
		out->write(reinterpret_cast<const char *>(pBuffer), writeSize);
		if (out->fail())
			break;
	}
	mem_free_dbg(pBuffer);
	return i == 0;
}

/**
 * Returns a pointer because in GCC < 5 ofstream itself is not moveable due to a bug.
 */
static std::ofstream *CaptureFile(std::string *dst_path)
{
	char filename[sizeof("screen00.PCX")];
	for (int i = 0; i <= 99; ++i) {
		snprintf(filename, sizeof(filename), "screen%02d.PCX", i);
		*dst_path = GetPrefPath();
		*dst_path += filename;
		if (!FileExists(dst_path->c_str())) {
			return new std::ofstream(*dst_path, std::ios::binary | std::ios::trunc);
		}
	}
	return NULL;
}

/**
 * @brief Make a red version of the given palette and apply it to the screen.
 */
static void RedPalette()
{
	for (int i = 0; i < NUM_COLORS; i++) {
		system_palette[i].g = 0;
		system_palette[i].b = 0;
	}
	palette_update();
	BltFast();
	RenderPresent();
}

void CaptureScreen()
{
	SDL_Color bkp_palette[lengthof(system_palette)];
	std::string FileName;
	bool success;

	std::ofstream *out = CaptureFile(&FileName);
	if (out == NULL)
		return;
	scrollrt_draw_game();
	memcpy(bkp_palette, system_palette, sizeof(bkp_palette));
	RedPalette();

	lock_buf(2);
	success = CaptureHdr(SCREEN_WIDTH, SCREEN_HEIGHT, out);
	if (success) {
		success = CapturePix(SCREEN_WIDTH, SCREEN_HEIGHT, BUFFER_WIDTH, &gpBuffer[SCREENXY(0, 0)], out);
	}
	if (success) {
		success = CapturePal(bkp_palette, out);
	}
	unlock_buf(2);
	out->close();

	if (!success) {
		DoLog("Failed to save screenshot at %s", FileName.c_str());
		RemoveFile(FileName.c_str());
	} else {
		DoLog("Screenshot saved at %s", FileName.c_str());
	}
	SDL_Delay(300);
	memcpy(system_palette, bkp_palette, sizeof(bkp_palette));
	palette_update();
	gbRedrawFlags = REDRAW_ALL;
	delete out;
}

DEVILUTION_END_NAMESPACE
