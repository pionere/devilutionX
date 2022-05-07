/*
 * Utility functions to manipulate Diablo Level CEL files. Its main features are:
 *  PNG2Min: Generate CEL and meta-files based on PNG-quads
 *  PatchMin: Patch the given SOL and TMI files using a TXT file.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <png.h>

typedef enum micro_flag {
	MET_SQUARE,
	MET_TRANSPARENT,
	MET_LTRIANGLE,
	MET_RTRIANGLE,
	MET_LTRAPEZOID,
	MET_RTRAPEZOID
} micro_flag;

typedef enum piece_flag {
	PFLAG_BLOCK_PATH       = 1 << 0,
	PFLAG_BLOCK_LIGHT      = 1 << 1,
	PFLAG_BLOCK_MISSILE    = 1 << 2,
	PFLAG_TRANSPARENT      = 1 << 3,
	PFLAG_TRANS_MASK_LEFT  = 1 << 4,
	PFLAG_TRANS_MASK_RIGHT = 1 << 5,
	// PFLAG_TRANS_MASK_NONE  = 1 << 6, - unused. masked the LEFT/RIGHT flags in vanilla.
	PFLAG_TRAP_SOURCE      = 1 << 7,
} piece_flag;

typedef enum piece_micro_flag {
	TMIF_WALL_TRANS = 1 << 0,
	TMIF_LEFT_REDRAW = 1 << 1,
	TMIF_LEFT_FOLIAGE = 1 << 2,
	TMIF_LEFT_WALL_TRANS = 1 << 3,
	TMIF_RIGHT_REDRAW = 1 << 4,
	TMIF_RIGHT_FOLIAGE = 1 << 5,
	TMIF_RIGHT_WALL_TRANS = 1 << 6,
} piece_micro_flag;

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
} png_image_data;

typedef struct MegaMetaData {
	int pieces[4];
} MegaMetaData;

typedef struct MicroMetaData {
	bool hasSet;
	bool isFloor;
	int MicroType;
	int MicroIdx;
	uint32_t dataAddr;
	BYTE* celData;
	uint32_t celLength;
} MicroMetaData;

typedef struct MetaFlags {
	bool secondDraw;
	bool foliageDraw;
	bool trans;
} MetaFlags;

typedef struct PieceMetaData {
	bool transAbove;
	bool blockPath;
	bool blockLight;
	bool blockMissile;
	bool trapSource;
	int height;
	const char* fileName;
	MicroMetaData micros[16];
	MetaFlags left;
	MetaFlags right;
	std::set<unsigned>* megas;
} PieceMetaData;

static void assert(bool check)
{
	check = check;
}

static int max(int x1, int x2)
{
	return x1 > x2 ? x1 : x2;
}

#define TILE_HEIGHT 32
#define TILE_WIDTH 64
#ifndef  ASSUME_UNREACHABLE
#define ASSUME_UNREACHABLE assert(0);
#endif // ! ASSUME_UNREACHABLE

template<class T, int N>
constexpr int lengthof(T (&arr)[N])
{
	return N;
}

static BYTE* LoadPal(const char* palFile)
{
	BYTE* result = (BYTE*)malloc(256 * 3);

	FILE* f = fopen(palFile, "rb");

	fread(result, 1, 256 * 3, f);

	fclose(f);

	return result;
}

static BYTE GetPalColor(RGBA &data, BYTE *palette, int numcolors, int offset)
{
	int res = -1;
	//int best = abs(data.r - 0) + 
	//		   abs(data.g - 0) + 
	//		   abs(data.b - 0)
	int best = (data.r - 0) * (data.r - 0) + 
			   (data.g - 0) * (data.g - 0) + 
			   (data.b - 0) * (data.b - 0);

	for (int i = 0; i < numcolors; i++, palette += 3) {
		//int dist = abs(data.r - palette[0]) + 
		//		   abs(data.g - palette[1]) + 
		//		   abs(data.b - palette[2]);
		int dist = (data.r - palette[0]) * (data.r - palette[0]) + 
				   (data.g - palette[1]) * (data.g - palette[1]) + 
				   (data.b - palette[2]) * (data.b - palette[2]);
		if (dist < best) {
			best = dist;
			res = i;
		}
	}
	if (res == -1)
		res = 0;
	else
		res += offset;
	return res;
}

static bool ReadPNG(const char *pngname, png_image_data &data)
{
	const int alphaByte = 255;
	FILE* fp;

	fp = fopen(pngname, "rb");
	if (fp == NULL)
		return false;
	/*int number = 0;
	png_const_bytep header;
	const int number = 8;

	if (fread(&header, 1, number, fp) != number) {
       return 0;
    }

	if (!png_sig_cmp(header, 0, number)) {
		return 0;
	}*/

	png_structp png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_infop end_info;
		png_destroy_read_struct(&png_ptr, &info_ptr,
			&end_info);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	//png_set_sig_bytes(png_ptr, number);

	png_read_info(png_ptr, info_ptr);

	
	png_int_32 bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_get_IHDR(png_ptr, info_ptr, &data.width, &data.height,
       &bit_depth, &color_type, &interlace_type,
       &compression_type, &filter_method);

	// tell libpng to strip 16 bit/color files down to 8 bits/color
	png_set_strip_16(png_ptr);

	/* Expand data to 24-bit RGB, or 8-bit grayscale, with alpha if available. */
	if (color_type & PNG_COLOR_MASK_PALETTE)
	{
		png_set_expand(png_ptr);
	}

	/* Expand the grayscale to 24-bit RGB if necessary. */
	if (!(color_type & PNG_COLOR_MASK_COLOR))
	{
		png_set_gray_to_rgb(png_ptr);
	}

	// expand paletted or RGB images with transparency to full alpha channels
	// so the data will be available as RGBA quartets
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}

	// if there is no alpha information, fill with alphaByte
	if (!(color_type & PNG_COLOR_MASK_ALPHA))
	{
		png_set_filler(png_ptr, alphaByte, PNG_FILLER_AFTER );
	}

	// expand pictures with less than 8bpp to 8bpp
	if (bit_depth < 8) {
		png_set_packing(png_ptr);
	}

	png_set_interlace_handling(png_ptr);

	// update structure with the above settings
	png_read_update_info(png_ptr, info_ptr);

	png_bytep buffer = (png_bytep)png_malloc(png_ptr, data.height * data.width * sizeof(RGBA));

	png_bytep *row_pointers = (png_bytep*)malloc(data.height * sizeof(void*));
	for (int i = 0; i < data.height; i++)
		row_pointers[i] = buffer + i * data.width * sizeof(RGBA);

	png_read_image(png_ptr, row_pointers);

	png_read_end(png_ptr, (png_infop)NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr,
       (png_infopp)NULL);

	fclose(fp);
	data.row_pointers = row_pointers;
	data.data_ptr = buffer;
	return true;
}

/** Specifies the draw masks used to render transparency of the right side of tiles. */
static uint32_t RightMask[TILE_HEIGHT] = {
	0xEAAAAAAA, 0xF5555555,
	0xFEAAAAAA, 0xFF555555,
	0xFFEAAAAA, 0xFFF55555,
	0xFFFEAAAA, 0xFFFF5555,
	0xFFFFEAAA, 0xFFFFF555,
	0xFFFFFEAA, 0xFFFFFF55,
	0xFFFFFFEA, 0xFFFFFFF5,
	0xFFFFFFFE, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of the left side of tiles. */
static uint32_t LeftMask[TILE_HEIGHT] = {
	0xAAAAAAAB, 0x5555555F,
	0xAAAAAABF, 0x555555FF,
	0xAAAAABFF, 0x55555FFF,
	0xAAAABFFF, 0x5555FFFF,
	0xAAABFFFF, 0x555FFFFF,
	0xAABFFFFF, 0x55FFFFFF,
	0xABFFFFFF, 0x5FFFFFFF,
	0xBFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};
/** Specifies the draw masks used to render transparency of wall tiles. */
static uint32_t WallMask[TILE_HEIGHT] = {
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555,
	0xAAAAAAAA, 0x55555555
};

static uint32_t SolidMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF
};

static uint32_t RightFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0x3FFFFFFF,
	0x0FFFFFFF, 0x03FFFFFF,
	0x00FFFFFF, 0x003FFFFF,
	0x000FFFFF, 0x0003FFFF,
	0x0000FFFF, 0x00003FFF,
	0x00000FFF, 0x000003FF,
	0x000000FF, 0x0000003F,
	0x0000000F, 0x00000003,
	0x00000000, 0x00000003,
	0x0000000F, 0x0000003F,
	0x000000FF, 0x000003FF,
	0x00000FFF, 0x00003FFF,
	0x0000FFFF, 0x0003FFFF,
	0x000FFFFF, 0x003FFFFF,
	0x00FFFFFF, 0x03FFFFFF,
	0x0FFFFFFF, 0x3FFFFFFF,
};

static uint32_t LeftFoliageMask[TILE_HEIGHT] = {
	0xFFFFFFFF, 0xFFFFFFFC,
	0xFFFFFFF0, 0xFFFFFFC0,
	0xFFFFFF00, 0xFFFFFC00,
	0xFFFFF000, 0xFFFFC000,
	0xFFFF0000, 0xFFFC0000,
	0xFFF00000, 0xFFC00000,
	0xFF000000, 0xFC000000,
	0xF0000000, 0xC0000000,
	0x00000000, 0xC0000000,
	0xF0000000, 0xFC000000,
	0xFF000000, 0xFFC00000,
	0xFFF00000, 0xFFFC0000,
	0xFFFF0000, 0xFFFFC000,
	0xFFFFF000, 0xFFFFFC00,
	0xFFFFFF00, 0xFFFFFFC0,
	0xFFFFFFF0, 0xFFFFFFFC,
};

static bool hasFoliageLine(int n, uint32_t mask)
{
	int i = ((sizeof(uint32_t) * CHAR_BIT) - n);

	// Clear the lower bits of the mask to avoid testing i < n in the loops.
	mask = (mask >> i) << i;
	return mask != 0;
}

static bool hasFoliageBit(int type, bool left, BYTE* src)
{
	int i, j;
	char v;
	uint32_t m, *mask;

	if (left) {
		mask = &LeftFoliageMask[TILE_HEIGHT - 1];
	} else {
		mask = &RightFoliageMask[TILE_HEIGHT - 1];
	}	

	switch (type) {
	case MET_SQUARE:
		return true;
	case MET_TRANSPARENT:
		for (i = TILE_HEIGHT; i != 0; i--, mask--) {
			m = *mask;
			static_assert(TILE_WIDTH / 2 <= sizeof(m) * CHAR_BIT, "Undefined left-shift behavior.");
			for (j = TILE_WIDTH / 2; j != 0; j -= v, m <<= v) {
				v = *src++;
				if (v > 0) {
					if (hasFoliageLine(v, m))
						return true;
					src += v;
				} else {
					v = -v;
				}
			}
		}
		break;
	case MET_LTRIANGLE:
	case MET_RTRIANGLE:
		return false;
	case MET_LTRAPEZOID:
	case MET_RTRAPEZOID:
		return true;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return false;
}

static void CleanupImageData(png_image_data* imagedata, int numimages)
{
	for (int n = 0; n < numimages; n++) {
		free(imagedata[n].data_ptr);
		free(imagedata[n].row_pointers);
	}
	free(imagedata);
}

static bool InitFloorFlags(MetaFlags* mFlags, MicroMetaData* floorMeta, MicroMetaData* lvlMeta,
	bool left)
{
	bool inTown = false;
	switch (floorMeta->MicroType) {
	//case MET_SQUARE:
	case MET_TRANSPARENT:
		mFlags->secondDraw = hasFoliageBit(floorMeta->MicroType, left, floorMeta->celData);
		mFlags->foliageDraw = mFlags->secondDraw & (inTown || !lvlMeta->hasSet);
		mFlags->trans = false; //lvlIdx == DLV_HELL1 && lvlMeta->hasSet;
		return mFlags->trans;
	case MET_LTRIANGLE:
		assert(left);
		mFlags->secondDraw = false;
		mFlags->foliageDraw = false;
		mFlags->trans = false;
		break;
	case MET_RTRIANGLE:
		assert(!left);
		mFlags->secondDraw = false;
		mFlags->foliageDraw = false;
		mFlags->trans = false;
		break;
	case MET_LTRAPEZOID:
		assert(left);
		mFlags->secondDraw = inTown ? false : true;
		mFlags->foliageDraw = false;
		mFlags->trans = true;
		return lvlMeta->hasSet;
	case MET_RTRAPEZOID:
		assert(!left);
		mFlags->secondDraw = inTown ? false : true;
		mFlags->foliageDraw = false;
		mFlags->trans = true;
		return lvlMeta->hasSet;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return false;
}

static BYTE* ReEncodeMicro(BYTE* celData, int type, uint32_t* length)
{
	BYTE* tmpData;
	BYTE* pSrc = celData;
	BYTE* pDst;
	int i, j;

	switch (type) {
	case MET_LTRIANGLE:
		*length = TILE_WIDTH * TILE_HEIGHT / 4 + TILE_HEIGHT;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2) {
			if (((char)*pSrc) == -i) {
				// skip transparent pixels
				++pSrc;
			} else if (i != 0)
				break;
			if (*pSrc != (TILE_WIDTH / 2 - i))
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			for (j = 0; j < TILE_WIDTH / 2 - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i >= 0)
			break;

		for (i = 2; i != TILE_HEIGHT; i += 2) {
			if (((char)*pSrc) != -i)
				break;
			++pSrc;
			if (*pSrc != (TILE_WIDTH / 2 - i))
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			for (j = 0; j < TILE_WIDTH / 2 - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i == TILE_HEIGHT) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	case MET_RTRIANGLE:
		*length = TILE_WIDTH * TILE_HEIGHT / 4 + TILE_HEIGHT;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2) {
			if (*pSrc != (TILE_WIDTH / 2 - i))
				break;
			++pSrc;
			for (j = 0; j < TILE_WIDTH / 2 - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			if (i == 0)
				continue;
			// skip transparent pixels
			if (((char)*pSrc) != -i)
				break;
			++pSrc;
		}
		if (i >= 0)
			break;

		for (i = 2; i != TILE_HEIGHT; i += 2) {
			if (*pSrc != (TILE_WIDTH / 2 - i))
				break;
			++pSrc;
			for (j = 0; j < TILE_WIDTH / 2 - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
			if (((char)*pSrc) != -i)
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
		}
		if (i == TILE_HEIGHT) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	case MET_LTRAPEZOID:
		*length = (TILE_WIDTH * TILE_HEIGHT) / 4 + TILE_HEIGHT * (2 + TILE_HEIGHT) / 4 + TILE_HEIGHT / 2;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2) {
			if (((char)*pSrc) == -i) {
				// skip transparent pixels
				++pSrc;
			} else if (i != 0)
				break;
			if (*pSrc != (TILE_WIDTH / 2 - i))
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			for (j = 0; j < TILE_WIDTH / 2 - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i >= 0)
			break;

		for (i = TILE_HEIGHT / 2; i != 0; i--) {
			if (*pSrc != TILE_WIDTH / 2)
				break;
			++pSrc;
			for (j = 0; j < TILE_WIDTH / 2; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i == 0) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	case MET_RTRAPEZOID:
		*length = (TILE_WIDTH * TILE_HEIGHT) / 4 + TILE_HEIGHT * (2 + TILE_HEIGHT) / 4 + TILE_HEIGHT / 2;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = TILE_HEIGHT - 2; i >= 0; i -= 2) {
			if (*pSrc != (TILE_WIDTH / 2 - i))
				break;
			++pSrc;
			for (j = 0; j < TILE_WIDTH / 2 - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			if (i == 0)
				continue;
			// skip transparent pixels
			if (((char)*pSrc) != -i)
				break;
			++pSrc;
		}
		if (i >= 0)
			break;

		for (i = TILE_HEIGHT / 2; i != 0; i--) {
			if (*pSrc != TILE_WIDTH / 2)
				break;
			++pSrc;
			for (j = 0; j < TILE_WIDTH / 2; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i == 0) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	case MET_SQUARE:
		*length = TILE_WIDTH * TILE_HEIGHT / 2;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = TILE_HEIGHT; i != 0; i--) {
			if (*pSrc != TILE_WIDTH / 2)
				break;
			++pSrc;
			for (j = 0; j < TILE_WIDTH / 2; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i == 0) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	default:
		ASSUME_UNREACHABLE;
	}
	free(tmpData);
	return NULL;
}

static void fput_int32(FILE* f0, int32_t val)
{
	fputc(val & 0xFF, f0);
	fputc((val >> 8) & 0xFF, f0);
	fputc((val >> 16) & 0xFF, f0);
	fputc((val >> 24) & 0xFF, f0);
}

static void fput_int16(FILE* f0, int16_t val)
{
	fputc(val & 0xFF, f0);
	fputc((val >> 8) & 0xFF, f0);
}

static void EncodeMicro(png_image_data* imagedata, int sy, bool left, MicroMetaData* mmd,
	BYTE* palette, int numcolors, int coloroffset)
{
	BYTE* celData = (BYTE*)malloc(TILE_WIDTH * TILE_HEIGHT);
	memset(celData, 0, TILE_WIDTH * TILE_HEIGHT);

	bool hasAlpha = false;
	bool hasColor = false;
	BYTE* pHead = celData;
	BYTE* pBuf = celData + 1;
	for (int i = sy; i >= max(0, sy - (TILE_HEIGHT - 1)); i--) {
		RGBA* data = (RGBA*)imagedata->row_pointers[i];
		if (!left)
			data += TILE_WIDTH / 2;
		bool alpha = false;
		for (int j = 0; j < TILE_WIDTH / 2; j++) {
			if (data[j].a != 255) {
				// add transparent pixel
				if ((char)(*pHead) > 0) {
					pHead = pBuf;
					pBuf++;
				}
				--*pHead;
				alpha = true;
				hasAlpha = true;
			} else {
				// add opaque pixel
				if (alpha) {
					alpha = false;
					pHead = pBuf;
					pBuf++;
				}
				*pBuf = GetPalColor(data[j], palette, numcolors, coloroffset);
				pBuf++;
				++*pHead;
				hasColor = true;
			}
		}
		pHead = pBuf;
		pBuf++;
	}
	if (!hasColor) {
		free(celData);
		return;
	}

	// add transparent rows
	for (int i = 0; i < -(sy - (TILE_HEIGHT - 1)); i++) {
		*pBuf = -(TILE_WIDTH / 2);
		pBuf++;
		hasAlpha = true;
	}

	// find the best encoding
	mmd->hasSet = true;
	if (!hasAlpha) {
		mmd->celData = ReEncodeMicro(celData, MET_SQUARE, &mmd->celLength);
		mmd->MicroType = MET_SQUARE;
		free(celData);
		return;
	}

	uint32_t bestLen = pBuf - celData;
	if (!hasFoliageBit(MET_TRANSPARENT, left, celData)) {
		if (bestLen >= TILE_WIDTH * TILE_HEIGHT / 4 + TILE_HEIGHT) {
			uint32_t newLen;
			BYTE* remData = ReEncodeMicro(celData, left ? MET_LTRIANGLE : MET_RTRIANGLE, &newLen);
			if (remData != NULL) {
				mmd->celData = remData;
				mmd->celLength = newLen;
				mmd->MicroType = left ? MET_LTRIANGLE : MET_RTRIANGLE;
				free(celData);
				return;
			}
		}
	} else {
		uint32_t newLen;
		BYTE* remData = ReEncodeMicro(celData, left ? MET_LTRAPEZOID : MET_RTRAPEZOID, &newLen);
		if (remData != NULL) {
			mmd->celData = remData;
			mmd->celLength = newLen;
			mmd->MicroType = left ? MET_LTRAPEZOID : MET_RTRAPEZOID;
			free(celData);
			return;
		}
	}
	mmd->celData = celData;
	mmd->celLength = bestLen;
	mmd->MicroType = MET_TRANSPARENT;
}

/*
 * Generate CEL and meta-files based on PNG-quads.
 * - Sample usage:
 *     const char* filenames[][4] = {
 *		 { "f:\\l1_min_sub-tile0000.png",
 *		   "f:\\l1_min_sub-tile0001.png",
 *		   "f:\\l1_min_sub-tile0002.png",
 *		   "f:\\l1_min_sub-tile0003.png" },
 *		 { "f:\\l1_min_sub-tile0004.png",
 *		   "f:\\l1_min_sub-tile0005.png",
 *		   "f:\\l1_min_sub-tile0006.png",
 *		   "f:\\l1_min_sub-tile0003.png" } 
 *		};
 *     BYTE* pal = LoadPal("d:\\L1_1.PAL");
 *     PNG2Min(filenames, 2, 10, "d:\\out\\", "L1", pal, 256, 0);
 *   output: TIL, CEL, MIN, TMI, SOL, TXT files
 *
 * @param megatiles: PNG-quads, images of tiles for each megatile
 * @param nummegas: the number of PNG-quads
 * @param blocks: the number of micros per tile. Must be an even number. 10 or 16 in vanilla diablo.
 * @param outfolder: the path to the folder where the output files are placed
 * @param prefix: added to the generated filenames.
 * @param palette: the palette to be used to generate the CEL file
 * @param numcolors: the number of colors in the palette
 * @param coloroffset: added to the color-values which are selected from the palette
 * @return 0 if the function succeeds
 */
int PNG2Min(const char* megatiles[][4], int nummegas, int blocks,
	const char* outfolder, const char* prefix,
	BYTE* palette, int numcolors, int coloroffset)
{
	// read the png images
	png_image_data* imagedata = (png_image_data*)malloc(sizeof(png_image_data) * (nummegas * 4));
	std::map<const char*, std::pair<int, png_image_data*>> imagemap;
	int pn = 0;
	for (int n = 0; n < nummegas; n++) {
		for (int i = 0; i < 4; i++) {
			if (imagemap.find(megatiles[n][i]) != imagemap.end())
				continue;
			if (ReadPNG(megatiles[n][i], imagedata[pn])) {
				imagemap[megatiles[n][i]] = std::pair<int, png_image_data*>(pn, &imagedata[pn]);
				pn++;
				continue;
			}
			CleanupImageData(imagedata, pn);
			return 1; // out-of-memory
		}
	}

	// validate the images
	for (int n = 0; n < pn; n++) {
		png_image_data* imgdata = &imagedata[n];
		if (imgdata->width != TILE_WIDTH) {
			CleanupImageData(imagedata, pn);
			return 2; // invalid image (width)
		}
		if (imgdata->height > TILE_HEIGHT * blocks / 2) {
			CleanupImageData(imagedata, pn);
			return 3; // invalid image (height)
		}
	}

	// create megas
	MegaMetaData* megaData = (MegaMetaData*)malloc(sizeof(MegaMetaData) * nummegas);
	for (int n = 0; n < nummegas; n++) {
		MegaMetaData* cmd = &megaData[n];
		for (int i = 0; i < 4; i++) {
			cmd->pieces[i] = imagemap[megatiles[n][i]].first;
		}
	}

	// create pieces
	PieceMetaData* pieceData = (PieceMetaData*)malloc(sizeof(PieceMetaData) * pn);
	assert(blocks <= lengthof(pieceData->micros));
	for (auto it = imagemap.cbegin(); it != imagemap.cend(); ++it) {
		PieceMetaData* cpd = &pieceData[it->second.first];
		cpd->fileName = it->first;
		png_image_data* imgdata = it->second.second;

		memset(cpd->micros, 0, sizeof(cpd->micros));
		int n = 0;
		for (int i = imgdata->height - 1; i >= 0; i -= TILE_HEIGHT) {
			EncodeMicro(imgdata, i, true, &cpd->micros[n++], palette, numcolors, coloroffset);
			EncodeMicro(imgdata, i, false, &cpd->micros[n++], palette, numcolors, coloroffset);
		}

		cpd->micros[0].isFloor = true;
		cpd->micros[1].isFloor = true;

		cpd->megas = new std::set<unsigned>();
		for (int n = 0; n < nummegas; n++) {
			MegaMetaData* cmd = &megaData[n];
			for (int i = 0; i < 4; i++) {
				if (cmd->pieces[i] == it->second.first)
					cpd->megas->insert(n);
			}
		}
	}

	// initialize the micro-indices
	int midx = 1;
	MicroMetaData** microData = (MicroMetaData**)malloc(sizeof(MicroMetaData*) * (pn * blocks + 1));
	for (int n = 0; n < pn; n++) {
		PieceMetaData* cpd = &pieceData[n];
		for (int j = 0; j < blocks; j += 2) {
			if (cpd->micros[j].hasSet) {
				microData[midx] = &cpd->micros[j];
				cpd->micros[j].MicroIdx = midx++;
			}
			if (cpd->micros[j + 1].hasSet) {
				microData[midx] = &cpd->micros[j + 1];
				cpd->micros[j + 1].MicroIdx = midx++;
			}
		}
	}

	// initialize meta-flags
	for (int n = 0; n < pn; n++) {
		PieceMetaData* mData = &pieceData[n];
		mData->blockLight = (mData->micros[2].hasSet && mData->micros[2].MicroType != MET_TRANSPARENT)
			|| (mData->micros[3].hasSet && mData->micros[3].MicroType == MET_TRANSPARENT);
		mData->blockMissile = mData->micros[2].hasSet || mData->micros[3].hasSet;
		mData->blockPath = mData->micros[2].hasSet || mData->micros[3].hasSet
			|| (mData->micros[0].hasSet && mData->micros[0].MicroType == MET_LTRAPEZOID)
			|| (mData->micros[1].hasSet && mData->micros[1].MicroType == MET_RTRAPEZOID);
		mData->trapSource = 
			(mData->micros[2].hasSet && mData->micros[2].MicroType == MET_SQUARE
		  && mData->micros[4].hasSet && mData->micros[4].MicroType == MET_SQUARE)
		 || (mData->micros[3].hasSet && mData->micros[3].MicroType == MET_SQUARE
		 && mData->micros[5].hasSet && mData->micros[5].MicroType == MET_SQUARE);

		int height = blocks - 1;
		while (height > 0 && !mData->micros[height].hasSet)
			height--;
		mData->height = height / 2 + 1;
		if (!mData->micros[0].hasSet) {
			if (!mData->micros[1].hasSet) {
				// no floor
				mData->left.secondDraw = false;
				mData->left.foliageDraw = false;
				mData->left.trans = false;

				mData->right.secondDraw = false;
				mData->right.foliageDraw = false;
				mData->right.trans = false;

				mData->transAbove = false;
				continue;
			}
			// right floor only
			mData->left.secondDraw = false;
			mData->left.foliageDraw = false;
			mData->left.trans = false;

			mData->transAbove = InitFloorFlags(&mData->right, &mData->micros[1], &mData->micros[3],
				false);
			continue;
		}
		if (!mData->micros[1].hasSet) {
			// left floor only
			mData->right.secondDraw = false;
			mData->right.foliageDraw = false;
			mData->right.trans = false;

			mData->transAbove = InitFloorFlags(&mData->left, &mData->micros[0], &mData->micros[2],
				true);
			continue;
		}
		// both floors are set
		mData->transAbove = InitFloorFlags(&mData->left, &mData->micros[0], &mData->micros[2],
				true);
		mData->transAbove |= InitFloorFlags(&mData->right, &mData->micros[1], &mData->micros[3],
				false);
	}

	// create output files
	char filename[256];
	FILE* f0;
	// create TIL
	snprintf(filename, sizeof(filename), "%s%s.TIL", outfolder, prefix);
	f0 = fopen(filename, "wb");
	for (int n = 0; n < nummegas; n++) {
		for (int i = 0; i < 4; i++) {
			fput_int16(f0, megaData[n].pieces[i]);
		}
	}
	fclose(f0);

	// create CEL
	snprintf(filename, sizeof(filename), "%s%s.CEL", outfolder, prefix);
	f0 = fopen(filename, "wb");
	fput_int32(f0, midx - 1);
	uint32_t addr = 4 * (midx + 1);
	for (int n = 1; n < midx; n++) {
		MicroMetaData* mmd = microData[n];
		fput_int32(f0, addr);
		addr += mmd->celLength;
		mmd->dataAddr = addr;
	}
	fput_int32(f0, addr);

	for (int n = 1; n < midx; n++) {
		MicroMetaData* mmd = microData[n];
		fwrite(mmd->celData, 1, mmd->celLength, f0);
	}
	fclose(f0);

	// create MIN
	snprintf(filename, sizeof(filename), "%s%s.MIN", outfolder, prefix);
	f0 = fopen(filename, "wb");
	for (int n = 0; n < pn; n++) {
		PieceMetaData* cpd = &pieceData[n];
		for (int i = blocks - 2; i >= 0; i -= 2) {
			uint16_t idx = 0;
			if (cpd->micros[i].hasSet)
				idx = (cpd->micros[i].MicroType << 12) |  cpd->micros[i].MicroIdx;
			fput_int16(f0, idx);
			idx = 0;
			if (cpd->micros[i + 1].hasSet)
				idx = (cpd->micros[i + 1].MicroType << 12) |  cpd->micros[i + 1].MicroIdx;
			fput_int16(f0, idx);
		}
	}
	fclose(f0);

	// create TMI
	snprintf(filename, sizeof(filename), "%s%s.TMI", outfolder, prefix);
	f0 = fopen(filename, "wb");
	fputc(0, f0);
	for (int n = 0; n < pn; n++) {
		PieceMetaData* cpd = &pieceData[n];
		BYTE bv = 0;
		bv |= (cpd->transAbove ? 1 : 0) << 0;
		bv |= (cpd->left.secondDraw ? 1 : 0) << 1;
		bv |= (cpd->left.foliageDraw ? 1 : 0) << 2;
		bv |= (cpd->left.trans ? 1 : 0) << 3;
		bv |= (cpd->right.secondDraw ? 1 : 0) << 4;
		bv |= (cpd->right.foliageDraw ? 1 : 0) << 5;
		bv |= (cpd->right.trans ? 1 : 0) << 6;
		fputc(bv, f0);
	}
	fclose(f0);

	// create SOL
	snprintf(filename, sizeof(filename), "%s%s.SOL", outfolder, prefix);
	f0 = fopen(filename, "wb");
	fputc(0, f0);
	for (int n = 0; n < pn; n++) {
		PieceMetaData* cpd = &pieceData[n];
		BYTE bv = 0;

		bv |= (cpd->blockPath ? 1 : 0) << 0;
		bv |= (cpd->blockLight ? 1 : 0) << 1;
		bv |= (cpd->blockMissile ? 1 : 0) << 2;

		bv |= (cpd->transAbove ? 1 : 0) << 3;
		bv |= (cpd->left.trans ? 1 : 0) << 4;
		bv |= (cpd->right.trans ? 1 : 0) << 5;

		bv |= (cpd->trapSource ? 1 : 0) << 7;
		fputc(bv, f0);
	}
	fclose(f0);

	// create TXT
	snprintf(filename, sizeof(filename), "%s%s.TXT", outfolder, prefix);
	f0 = fopen(filename, "wt");
	for (int n = 1; n <= pn; n++) {
		PieceMetaData* mData = &pieceData[n - 1];

		snprintf(filename, 256, "; Path: %s\n", mData->fileName);
		fputs(filename, f0);

		snprintf(filename, 256, "; Megas:", mData->megas->size());
		fputs(filename, f0);
		for (auto it = mData->megas->cbegin(); it != mData->megas->cend(); ++it) {
			snprintf(filename, 256, " %d,", (*it) + 1);
			fputs(filename, f0);
		}
		if (!mData->megas->empty())
			fseek(f0, -1, SEEK_CUR);
		fputs("\n", f0);
		snprintf(filename, 256, "; Micros:\n");
		fputs(filename, f0);
		for (int j = 0; j < mData->height; j++) {
			fputs(";\t", f0);
			MicroMetaData* mcData = &mData->micros[j * 2];
			if (mcData->hasSet) {
				snprintf(filename, 256, "[Idx:%4d, Type:%d, Size:%4d Addr:%8d], ", mcData->MicroIdx, mcData->MicroType, mcData->celLength, mcData->dataAddr);
			} else {
				snprintf(filename, 256, "[-]                                        , ");
			}
			fputs(filename, f0);

			mcData = &mData->micros[j * 2 + 1];
			if (mcData->hasSet) {
				snprintf(filename, 256, "[Idx:%4d, Type:%d, Size:%4d Addr:%8d]", mcData->MicroIdx, mcData->MicroType, mcData->celLength, mcData->dataAddr);
			} else {
				snprintf(filename, 256, "[-]");
			}
			fputs(filename, f0);
			fputs("\n", f0);
		}
		fputs(";\n", f0);
		snprintf(filename, 256, "[Tile%d]\n", n);
		fputs(filename, f0);
		snprintf(filename, 256, "BlocksPath=%d\n", mData->blockPath);
		fputs(filename, f0);
		snprintf(filename, 256, "BlocksLight=%d\n", mData->blockLight);
		fputs(filename, f0);
		snprintf(filename, 256, "BlocksMissile=%d\n", mData->blockMissile);
		fputs(filename, f0);
		snprintf(filename, 256, "TransparentWall=%d\n", mData->transAbove);
		fputs(filename, f0);
		snprintf(filename, 256, "TransparentLFloor=%d\n", mData->left.trans);
		fputs(filename, f0);
		snprintf(filename, 256, "TransparentRFloor=%d\n", mData->right.trans);
		fputs(filename, f0);
		snprintf(filename, 256, "TrapSource=%d\n", mData->trapSource);
		fputs(filename, f0);
		fputs("\n", f0);

		snprintf(filename, 256, "RedrawLeft=%d\n", mData->left.secondDraw);
		fputs(filename, f0);
		snprintf(filename, 256, "FoliageDrawLeft=%d\n", mData->left.foliageDraw);
		fputs(filename, f0);
		snprintf(filename, 256, "RedrawRight=%d\n", mData->right.secondDraw);
		fputs(filename, f0);
		snprintf(filename, 256, "FoliageDrawRight=%d\n", mData->right.foliageDraw);
		fputs(filename, f0);

		fputs("\n\n", f0);
	}
	fclose(f0);

	// cleanup memory
	CleanupImageData(imagedata, pn);
	for (int n = 0; n < pn; n++) {
		PieceMetaData* mData = &pieceData[n];
		for (int i = 0; i < blocks; i++) {
			if (mData->micros[i].hasSet)
				free(mData->micros[i].celData);
		}
		free(mData->megas);
	}
	free(pieceData);
	free(megaData);
	free(microData);
	return 0;
}

static void PatchFile(FILE* file, int idx, int flag, int flagPos, int value)
{
	BYTE bv;
	assert(flag == (1 << flagPos));
	if (file == NULL)
		return;
	fseek(file, idx, SEEK_SET);
	fread(&bv, 1, 1, file);
	fseek(file, -1, SEEK_CUR);
	bv = (bv & ~flag) | ((value != 0) << flagPos);
	fputc(bv, file);
}

/*
 * Patch the given SOL and TMI files using a TXT file.
 *
 * @param patchFileName: path of a TXT file containing Tile-properties
 * @param solFileName: path of a SOL file (optional)
 * @param tmiFileName: path of a TMI file (optional)
 */
void PatchMin(const char* patchFileName, const char* solFileName, const char* tmiFileName)
{
	std::ifstream stream(patchFileName);
	if (!stream.is_open())
		return;
	FILE* solFile = solFileName == NULL ? NULL : fopen(solFileName, "r+b");
	FILE* tmiFile = tmiFileName == NULL ? NULL : fopen(tmiFileName, "r+b");

	const char* prefix = "Tile";
	std::string buffer;
	int idx = -1; BYTE bv;
	while (std::getline(stream, buffer)) {
		if (buffer[0] == ';' || buffer[0] == '#') continue;
		if (buffer[0] == '[') {
			auto endPos = buffer.find(']');
			if (endPos == std::string::npos || endPos <= sizeof("Tile"))
				continue;
			buffer = buffer.substr(sizeof("Tile"), endPos - sizeof("Tile"));
			idx = atoi(buffer.c_str());
		} else if (idx != -1) {
			auto equalsPosition = buffer.find('=');
			if (equalsPosition == std::string::npos)
				continue;
			std::string nameOfElement = buffer.substr(0, equalsPosition);
			std::string valueOfElement = buffer.substr(equalsPosition + 1, buffer.size());

			int value = atoi(valueOfElement.c_str());
			if (nameOfElement == "BlocksPath") {
				PatchFile(solFile, idx, PFLAG_BLOCK_PATH, 0, value);
			} else if (nameOfElement == "BlocksLight") {
				PatchFile(solFile, idx, PFLAG_BLOCK_LIGHT, 1, value);
			} else if (nameOfElement == "BlocksMissile") {
				PatchFile(solFile, idx, PFLAG_BLOCK_MISSILE, 2, value);
			} else if (nameOfElement == "TransparentWall") {
				PatchFile(solFile, idx, PFLAG_TRANSPARENT, 3, value);
				PatchFile(tmiFile, idx, TMIF_WALL_TRANS, 0, value);
			} else if (nameOfElement == "TransparentLFloor") {
				PatchFile(solFile, idx, PFLAG_TRANS_MASK_LEFT, 4, value);
				PatchFile(tmiFile, idx, TMIF_LEFT_WALL_TRANS, 3, value);
			} else if (nameOfElement == "TransparentRFloor") {
				PatchFile(solFile, idx, PFLAG_TRANS_MASK_RIGHT, 5, value);
				PatchFile(tmiFile, idx, TMIF_RIGHT_WALL_TRANS, 6, value);
			} else if (nameOfElement == "TrapSource") {
				PatchFile(solFile, idx, PFLAG_TRAP_SOURCE, 7, value);
			} else if (nameOfElement == "RedrawLeft") {
				PatchFile(tmiFile, idx, TMIF_LEFT_REDRAW, 1, value);
			} else if (nameOfElement == "FoliageDrawLeft") {
				PatchFile(tmiFile, idx, TMIF_LEFT_FOLIAGE, 2, value);
			} else if (nameOfElement == "RedrawRight") {
				PatchFile(tmiFile, idx, TMIF_RIGHT_REDRAW, 4, value);
			} else if (nameOfElement == "FoliageDrawRight") {
				PatchFile(tmiFile, idx, TMIF_RIGHT_FOLIAGE, 5, value);
			}
		}
	}

	fclose(solFile);
	fclose(tmiFile);
}
