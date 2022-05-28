#pragma once

#include <fstream>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <png.h>

template<class T, int N>
constexpr int lengthof(T (&arr)[N])
{
	return N;
}

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef unsigned char BYTE;

typedef struct RGBA {
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE a;
} RGBA;

typedef struct png_image_data {
	png_uint_32 width;
	png_uint_32 height;
	png_bytep *row_pointers;
	png_bytep data_ptr;
	BYTE* fixColorMask;
} png_image_data;

inline WORD SwapLE16(WORD w)
{
	WORD v = 1;
	if (((BYTE*)&v)[1] == 0)
		return w;
	return ((w >> 8) & 0x00FF) | ((w << 8) & 0xFF00);
}

inline DWORD SwapLE32(DWORD dw)
{
	DWORD v = 1;
	if (((BYTE*)&v)[3] == 0)
		return dw;
	return ((dw >> 24) & 0xFF) | ((dw << 24) & 0xFF000000) | ((dw >> 8) & 0x00FF00) | ((dw << 8) & 0xFF0000);
}

extern size_t GetFileSize(const char* filename);
extern BYTE* LoadPal(const char* palFile);
extern BYTE GetPalColor(RGBA &data, BYTE *palette, int numcolors, int offset, int numfixcolors);
extern RGBA GetPNGColor(BYTE col, BYTE *palette, int coloroffset, int numfixcolors);
extern bool ReadPNG(const char *pngname, png_image_data &data);
extern bool WritePNG(const char *pngname, png_image_data &data);
extern void UpscalePNGImages(png_image_data* imagedata, int numimage, int multiplier, BYTE* palette, int numcolors, int coloroffset, int numfixcolors, int antiAliasingMode = 0);
extern void CleanupImageData(png_image_data* imagedata, int numimages);

#ifdef __cplusplus
}
#endif
