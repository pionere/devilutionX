/**
 * @file capture.cpp
 *
 * Implementation of the screenshot function.
 */
#include <cstdio>

#include "all.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

#define PCX_COLORS 256

/**
 * @brief Write the PCX-file header
 * @param width Image width
 * @param height Image height
 * @param out File stream to write to
 * @return True on success
 */
static bool CaptureHdr(uint16_t width, uint16_t height, FILE* out)
{
	FilePcxHeader Buffer;

	memset(&Buffer, 0, sizeof(Buffer));
	Buffer.Manufacturer = 0x0A;
	Buffer.Version = 5;
	Buffer.Encoding = 1;
	Buffer.BitsPerPixel = 8;
	Buffer.Xmax = static_cast<uint16_t>(width - 1);
	Buffer.Ymax = static_cast<uint16_t>(height - 1);
	Buffer.HDpi = width;
	Buffer.VDpi = height;
	Buffer.NPlanes = 1;
	Buffer.BytesPerLine = width;

	return WriteFile(&Buffer, sizeof(Buffer), out);
}

/**
 * @brief Write the current in-game palette to the PCX file
 * @param palette Current palette
 * @param out File stream for the PCX file.
 * @return True if successful, else false
 */
static bool CapturePal(SDL_Color (&palette)[NUM_COLORS], FILE* out)
{
	BYTE pcx_palette[1 + PCX_COLORS * 3];
	int i;
	static_assert(NUM_COLORS == PCX_COLORS, "Mismatching PCX and game palette.");
	pcx_palette[0] = 0x0C;
	for (i = 0; i < PCX_COLORS; i++) {
		pcx_palette[1 + 3 * i + 0] = palette[i].r;
		pcx_palette[1 + 3 * i + 1] = palette[i].g;
		pcx_palette[1 + 3 * i + 2] = palette[i].b;
	}

	return WriteFile(pcx_palette, sizeof(pcx_palette), out);
}

/**
 * @brief RLE compress the pixel data
 * @param src Raw pixel buffer
 * @param dst Output buffer
 * @param width Width of pixel buffer

 * @return Output buffer
 */
static BYTE* CaptureEnc(BYTE* src, BYTE* dst, int width)
{
	BYTE rleLength;
	BYTE rlePixel;

	while (width != 0) {
		rlePixel = *src;
		src++;
		rleLength = 1;

		width--;

		while (width != 0 && rlePixel == *src) {
			if (rleLength >= 0x3F)
				break;
			rleLength++;

			src++;
			width--;
		}

		if (rleLength > 1 || rlePixel > 0xBF) {
			*dst = rleLength | 0xC0;
			dst++;
		}

		*dst = rlePixel;
		dst++;
	}

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
static bool CapturePix(uint16_t width, uint16_t height, uint16_t stride, BYTE* pixels, FILE* out)
{
	int i;
	size_t writeSize;
	BYTE *pBuffer, *pBufferEnd;

	pBuffer = (BYTE*)DiabloAllocPtr(2 * width);
	for (i = height; i > 0; i--) {
		pBufferEnd = CaptureEnc(pixels, pBuffer, width);
		pixels += stride;
		writeSize = (size_t)pBufferEnd - (size_t)pBuffer;
		if (!WriteFile(pBuffer, writeSize, out))
			break;
	}
	mem_free_dbg(pBuffer);
	return i == 0;
}

static FILE* CaptureFile(std::string* dst_path)
{
	char filename[sizeof("screen00.PCX")];
	for (int i = 0; i <= 99; ++i) {
		snprintf(filename, sizeof(filename), "screen%02d.PCX", i);
		*dst_path = GetPrefPath();
		*dst_path += filename;
		if (!FileExists(dst_path->c_str())) {
			return FileOpen(dst_path->c_str(), "wb");
		}
	}
	return NULL;
}

void CaptureScreen()
{
	std::string FileName;
	bool success;

	FILE* out = CaptureFile(&FileName);
	if (out == NULL)
		return;

	lock_buf(2);
	success = CaptureHdr(SCREEN_WIDTH, SCREEN_HEIGHT, out);
	if (success) {
		success = CapturePix(SCREEN_WIDTH, SCREEN_HEIGHT, BUFFER_WIDTH, &gpBuffer[SCREENXY(0, 0)], out);
	}
	if (success) {
		success = CapturePal(system_palette, out);
	}
	unlock_buf(2);
	std::fclose(out);

	if (success) {
		DoLog("Screenshot saved at %s", FileName.c_str());
		EventPlrMsg("%s is created", FileName.c_str());
	} else {
		DoLog("Failed to save screenshot at %s", FileName.c_str());
		RemoveFile(FileName.c_str());
	}
}

DEVILUTION_END_NAMESPACE
