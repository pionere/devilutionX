/*
 * Utility functions to manipulate Diablo Level CEL files. Its main features are:
 *  Pal2PalUniq: Change palette colors to ensure each color is unique.
 *  Min2PNG: Convert a MIN file to PNG(s)
 *  PNG2Min: Generate CEL and meta-files based on PNG-quads
 *  UpscaleMin: (integer) upscale a MIN file (+ its corresponding CEL)
 *  PatchMin: Patch the given SOL and TMI files using a TXT file.
 */
#include "cmtools_c.h"

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

typedef enum _draw_mask_type {
	DMT_NONE,
	DMT_TWALL,
	DMT_LTFLOOR,
	DMT_RTFLOOR,
	DMT_LFLOOR,
	DMT_RFLOOR,
} _draw_mask_type;

typedef enum dungeon_type {
	DTYPE_TOWN,
	DTYPE_CATHEDRAL,
	DTYPE_CATACOMBS,
	DTYPE_CAVES,
	DTYPE_HELL,
	DTYPE_CRYPT,
	DTYPE_NEST,
	DTYPE_NONE = 0xFF,
} dungeon_type;

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

#define TILE_HEIGHT 32
#define TILE_WIDTH 64
#define MICRO_WIDTH 32
#define MICRO_HEIGHT 32
#ifndef  ASSUME_UNREACHABLE
#define ASSUME_UNREACHABLE assert(0);
#endif // ! ASSUME_UNREACHABLE

static void DumpPalette(const char* palFile, BYTE* palette, int numcolors)
{
	FILE* f = fopen(palFile, "wb");

	if (f == NULL) {
		return;
	}
	for (int i = 0; i < numcolors; i++) {
		fputc(palette[0], f);
		fputc(palette[1], f);
		fputc(palette[2], f);
		palette += 3;
	}
	fclose(f);
}

static bool PalColorUnique(BYTE* palCol, BYTE* palette, int numcolors)
{
	for (int i = 0; i < numcolors; i++, palette += 3) {
		if (palCol[0] != palette[0] || palCol[1] != palette[1] || palCol[2] != palette[2])
			continue;
		if (palCol == palette)
			continue;
		return false;
	}
	return true;
}

static bool NextPalColor(BYTE* palCol, int i)
{
	int sgn = i & 1;
	int dc = i >> 1;
	int dr, dg, db;

	switch (dc % 8) {
	case 0: dr = 0; dg = 0; db = 0; break;
	case 1: dr = 1; dg = 0; db = 0; break;
	case 2: dr = 0; dg = 1; db = 0; break;
	case 3: dr = 0; dg = 0; db = 1; break;
	case 4: dr = 1; dg = 1; db = 0; break;
	case 5: dr = 1; dg = 0; db = 1; break;
	case 6: dr = 0; dg = 1; db = 1; break;
	case 7: dr = 1; dg = 1; db = 1; break;
	}

	dc /= 8;
	dr += dc;
	dg += dc;
	db += dc;
	if (sgn) {
		dr = -dr;
		dg = -dg;
		db = -db;
	}

	if (dr + palCol[0] < 0 || dr + palCol[0] > UCHAR_MAX)
		return false;
	if (dg + palCol[1] < 0 || dg + palCol[2] > UCHAR_MAX)
		return false;
	if (db + palCol[2] < 0 || db + palCol[2] > UCHAR_MAX)
		return false;

	palCol[0] += dr;
	palCol[1] += dg;
	palCol[2] += db;
	return true;
}

/**
 * Change palette colors to ensure each color is unique. Necessary to preserve information while transforming data between  * PNG and CEL formats.
 * 
 * @param palName: the path to the palette
 * @param numcolors: the number of colors in the palette
 * @param outPalName: the result of the change
 * @return true if the change is successful and necessary
 */
bool Pal2PalUniq(const char* palName, int numcolors, const char* outPalName)
{
	int result;
	BYTE* pal = LoadPal(palName);
	BYTE* palCol;

	if (pal == NULL) {
		return false;
	}

	result = 0;
	for (int i = 1; i < numcolors; i++) {
		palCol = &pal[i * 3];
		if (PalColorUnique(palCol, pal, numcolors))
			continue;

		RGBA col;
		col.r = palCol[0];
		col.g = palCol[1];
		col.b = palCol[2];
		for (int j = 2; j < 512; j++) {
			NextPalColor(palCol, j);
			if (PalColorUnique(palCol, pal, numcolors))
				break;
			palCol[0] = col.r;
			palCol[1] = col.g;
			palCol[2] = col.b;
		}
		result++;
	}

	DumpPalette(outPalName, pal, numcolors);

	free(pal);
	return result != 0;
}

/** Specifies the draw masks used to render transparency of the right side of tiles. */
static uint32_t RightMask[MICRO_HEIGHT] = {
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
static uint32_t LeftMask[MICRO_HEIGHT] = {
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
static uint32_t WallMask[MICRO_HEIGHT] = {
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

static uint32_t SolidMask[MICRO_HEIGHT] = {
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

static uint32_t RightFoliageMask[MICRO_HEIGHT] = {
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

static uint32_t LeftFoliageMask[MICRO_HEIGHT] = {
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
		mask = &LeftFoliageMask[MICRO_HEIGHT - 1];
	} else {
		mask = &RightFoliageMask[MICRO_HEIGHT - 1];
	}	

	switch (type) {
	case MET_SQUARE:
		return true;
	case MET_TRANSPARENT:
		for (i = MICRO_HEIGHT; i != 0; i--, mask--) {
			m = *mask;
			static_assert(MICRO_WIDTH <= sizeof(m) * CHAR_BIT, "Undefined left-shift behavior.");
			for (j = MICRO_WIDTH; j != 0; j -= v, m <<= v) {
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

inline static void RenderLine(RGBA* dst, BYTE* src, int n, uint32_t mask, BYTE* palette, int coloroffset, int numfixcolors)
{
//#ifdef NO_OVERDRAW
//	if (dst >= gpBufStart && dst <= gpBufEnd)
//#endif
	{
		assert(n != 0);
		int i = ((sizeof(uint32_t) * CHAR_BIT) - n);
		BYTE* tbl;
		// Add the lower bits about we don't care.
		mask |= (1 << i) - 1;
		if (mask == 0xFFFFFFFF) {
			for (i = 0; i < n; i++) {
				dst[i] = GetPNGColor(src[i], palette, coloroffset, numfixcolors);
			}
		} else {
			// Clear the lower bits of the mask to avoid testing i < n in the loops.
			mask = (mask >> i) << i;
			for (i = 0; mask != 0; i++, mask <<= 1) {
				if (mask & 0x80000000) {
					dst[i] = GetPNGColor(src[i], palette, coloroffset, numfixcolors);
				}
			}
		}
	/*} else {
		if (dst < gpBufStart) {
			int xy = gpBufStart - dst;
			int y = -(xy / BUFFER_WIDTH);
			extern int aminy;
			if (aminy > y)
				aminy = y;
		} else {
			int xy = dst - gpBufEnd;
			int y = (gbZoomInFlag ? SCREEN_HEIGHT / 2 : SCREEN_HEIGHT) + (xy / BUFFER_WIDTH);
			extern int amaxy;
			if (amaxy < y)
				amaxy = y;
		}*/
	}

	//(*src) += n;
	//(*dst) += n;
}

void RenderMicro(RGBA* pBuff, int bufferWidth, uint16_t levelCelBlock, int maskType, BYTE* srcCels, bool upscaled, BYTE* palette, int coloroffset, int numfixcolors)
{
	int i, j, light = 0;
	char v, encoding;
	BYTE *src;
	RGBA* dst;
	uint32_t m, *mask, *pFrameTable;

	int BUFFER_WIDTH = bufferWidth;

	dst = pBuff;
	pFrameTable = (uint32_t *)srcCels;
	if (levelCelBlock == 0)
		return;
	if (upscaled) {
		if (pFrameTable[0] < levelCelBlock)
			return;
		src = &srcCels[SwapLE32(pFrameTable[levelCelBlock])];
		encoding = *src;
		src++;
	} else {
		if (pFrameTable[0] < (levelCelBlock & 0xFFF))
			return;
		src = &srcCels[SwapLE32(pFrameTable[levelCelBlock & 0xFFF])];
		encoding = (levelCelBlock /*& 0x7000*/) >> 12;
	}

	//mask = &SolidMask[MICRO_HEIGHT - 1];
	switch (maskType) {
	case DMT_NONE:
		mask = &SolidMask[MICRO_HEIGHT - 1];
		break;
	case DMT_TWALL:
		mask = &WallMask[MICRO_HEIGHT - 1];
		break;
	case DMT_LTFLOOR:
		mask = encoding != MET_LTRIANGLE ? &LeftMask[MICRO_HEIGHT - 1] : &SolidMask[MICRO_HEIGHT - 1];
		break;
	case DMT_RTFLOOR:
		mask = encoding != MET_RTRIANGLE ? &RightMask[MICRO_HEIGHT - 1] : &SolidMask[MICRO_HEIGHT - 1];
		break;
	case DMT_LFLOOR:
		//if (encoding != MET_TRANSPARENT)
		//	return;
		mask = &LeftFoliageMask[MICRO_HEIGHT - 1];
		break;
	case DMT_RFLOOR:
		//if (encoding != MET_TRANSPARENT)
		//	return;
		mask = &RightFoliageMask[MICRO_HEIGHT - 1];
		break;
	default:
		ASSUME_UNREACHABLE;
	}

	static_assert(MICRO_HEIGHT - 2 < MICRO_WIDTH, "Line with negative or zero width.");
	static_assert(MICRO_WIDTH <= sizeof(*mask) * CHAR_BIT, "Mask is too small to cover the tile.");
	switch (encoding) {
	case MET_SQUARE:
		for (i = MICRO_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH;
			dst += MICRO_WIDTH;
		}
		break;
	case MET_TRANSPARENT:
		for (i = MICRO_HEIGHT; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			m = *mask;
			static_assert(MICRO_WIDTH <= sizeof(m) * CHAR_BIT, "Undefined left-shift behavior.");
			for (j = MICRO_WIDTH; j != 0; j -= v, m <<= v) {
				v = *src++;
				if (v > 0) {
					RenderLine(dst, src, v, m, palette, coloroffset, numfixcolors);
					src += v;
					dst += v;
				} else {
					v = -v;
					dst += v;
				}
			}
		}
		break;
	case MET_LTRIANGLE:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
		}
		for (i = 2; i != MICRO_WIDTH; i += 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
		}
		break;
	case MET_RTRIANGLE:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
			src += i & 2;
			dst += i;
		}
		for (i = 2; i != MICRO_HEIGHT; i += 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
			src += i & 2;
			dst += i;
		}
		break;
	case MET_LTRAPEZOID:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			src += i & 2;
			dst += i;
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
		}
		for (i = MICRO_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH;
			dst += MICRO_WIDTH;
		}
		break;
	case MET_RTRAPEZOID:
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH - i, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH - i;
			dst += MICRO_WIDTH - i;
			src += i & 2;
			dst += i;
		}
		for (i = MICRO_HEIGHT / 2; i != 0; i--, dst -= BUFFER_WIDTH + MICRO_WIDTH, mask--) {
			RenderLine(dst, src, MICRO_WIDTH, *mask, palette, coloroffset, numfixcolors);
			src += MICRO_WIDTH;
			dst += MICRO_WIDTH;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

typedef struct min_image_data {
	uint16_t* levelBlocks;
} min_image_data;
min_image_data* ReadMinData(const char* minname, int &columns, int &rows, const char* celname,
	const char* tilname, int* oNumtiles, BYTE** oCelBuf, BYTE** oMinBuf)
{
	// read the CEL file into memory
	FILE* f = fopen(celname, "rb");
	if (f == NULL)
		return NULL;

	size_t fileSize = GetFileSize(celname);
	if (fileSize == 0)
		return NULL;
	BYTE *celBuf = (BYTE *)malloc(fileSize);

	fread(celBuf, 1, fileSize, f);
	fclose(f);

	fileSize = GetFileSize(minname);
	if (columns == 0)
		columns = 2;

	if (rows == 0) {
		if (fileSize % (columns * 5 * 2) == 0) {
			if (fileSize % (columns * 8 * 2) == 0) {
				fileSize = fileSize;
			}
			rows = 5;
		} else if (fileSize % (columns * 8 * 2) == 0) {
			rows = 8;
		} else {
			// invalid MIN file
			free(celBuf);
			return NULL;
		}
	} else {
		if (fileSize % (columns * rows * 2) != 0) {
			// MIN file does not match the input celpertile
			free(celBuf);
			return NULL;
		}
	}

	// read MIN file to memory
	f = fopen(minname, "rb");
	if (f == NULL) {
		free(celBuf);
		return NULL;
	}

	BYTE *minBuf = (BYTE *)malloc(fileSize);
	fread(minBuf, 1, fileSize, f);
	fclose(f);

	int numtiles;
	min_image_data* mindata;
	if (tilname != NULL) {
		// print the tiles specified by the TIL file
		f = fopen(tilname, "rb");
		if (f == NULL) {
			free(celBuf);
			free(minBuf);
			return NULL;
		}
		size_t tilSize = GetFileSize(tilname);
		if (tilSize % (4 * 2) != 0) {
			// invalid TIL file
			free(celBuf);
			free(minBuf);
			return NULL;
		}
		numtiles = tilSize / 8;
		numtiles *= 4;
		mindata = (min_image_data*)malloc(sizeof(min_image_data) * numtiles);
		for (int i = 0; i < numtiles; i++) {
			int n;
			fread(&n, 1, 2, f);
			n = SwapLE16(n);
			mindata[i].levelBlocks = &((WORD*)&minBuf[0])[n * (rows * columns)];
			if (n * (rows * columns) > fileSize - (rows * columns)) {
				// MIN file does not contain enough entries
				free(celBuf);
				free(minBuf);
				return NULL;
			}
		}

		fclose(f);
	} else {
		// print the content of MIN sequentially
		numtiles = fileSize / (columns * rows * 2);
		mindata = (min_image_data*)malloc(sizeof(min_image_data) * numtiles);
		for (int i = 0; i < numtiles; i++) {
			mindata[i].levelBlocks = &((WORD*)&minBuf[0])[i * (rows * columns)];
		}
	}

	*oNumtiles = numtiles;
	*oCelBuf = celBuf;
	*oMinBuf = minBuf;
	return mindata;
}

/*
 * Convert a MIN file to PNG(s)
 * @param minname: the path of the MIN file
 * @param columns: the number of columns in one tile. It is set to 2 by default.
 * @param rows: the number of rows in one tile. It is set to 5 or 8 by default depending on the size of the MIN file.
 * @param celname: the path to the CEL file containing the images of the sub-tiles
 * @param tilname: the path of the TIL file to select which tiles should be converted. The whole content of the MIN file is converted if it is set to NULL.
 * @param destFolder: the output folder
 * @param palette: the palette to be used
 * @param coloroffset: the offset to be applied when selecting a color from the palette
 * @return true if the function succeeds
 */
bool Min2PNG(const char* minname, int columns, int rows, const char* celname,
	const char* tilname, const char* destFolder, BYTE* palette, int coloroffset)
{
	int numtiles;
	BYTE *celBuf, *minBuf;
	min_image_data* mindata = ReadMinData(minname, columns, rows, celname, tilname, &numtiles, &celBuf, &minBuf);
	if (mindata == NULL)
		return false;

	// write the png(s)
	png_image_data imagedata;
	for (int i = 0; i < numtiles; i++) {
		imagedata.width = columns * MICRO_WIDTH;
		imagedata.height = rows * MICRO_HEIGHT;
		RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata.height * imagedata.width);
		// make the background transparent
		memset(imagerows, 0, sizeof(RGBA) * imagedata.height * imagedata.width);
		imagedata.row_pointers = (png_bytep*)malloc(imagedata.height * sizeof(void*));
		for (int n = 0; n < imagedata.height; n++) {
			imagedata.row_pointers[n] = (png_bytep)&imagerows[imagedata.width * n];
		}
		imagedata.data_ptr = (png_bytep)imagerows;

		uint16_t* src = mindata[i].levelBlocks;
		if (columns != 2) {
			// upscaled min
			RGBA* dst = (RGBA*)imagedata.row_pointers[rows * MICRO_HEIGHT - 1];
			for (int y = 0; y < rows; y++) {
				for (int x = 0; x < columns; x++, src++) {
					uint16_t levelCelBlock = SwapLE16(*src);
					RenderMicro(dst, columns * MICRO_WIDTH, levelCelBlock, DMT_NONE, celBuf, true, palette, coloroffset, 0);
					dst += MICRO_WIDTH;
				}
				dst -= columns * MICRO_WIDTH * (MICRO_HEIGHT + 1);
			}
		} else {
			// standard min
			RGBA* dst = (RGBA*)imagedata.row_pointers[MICRO_HEIGHT - 1];
			for (int y = 0; y < rows; y++) {
				for (int x = 0; x < columns; x++, src++) {
					uint16_t levelCelBlock = SwapLE16(*src);
					RenderMicro(dst, columns * MICRO_WIDTH, levelCelBlock, DMT_NONE, celBuf, false, palette, coloroffset, 0);
					dst += MICRO_WIDTH;
				}
				dst += columns * MICRO_WIDTH * (MICRO_HEIGHT - 1);
			}
		}
		// imagedata.fixColorMask = NULL;

		// write a single png
		char destFile[256];
		int idx = strlen(celname) - 1;
		while (idx > 0 && celname[idx] != '\\' && celname[idx] != '/')
			idx--;
		int fnc = snprintf(destFile, 236, "%s%s", destFolder, &celname[idx + 1]);
		snprintf(&destFile[fnc - 4], 20, "_tile%04d.png", i);

		if (!WritePNG(destFile, imagedata)) {
			free(imagedata.row_pointers);
			free(imagerows);
			free(celBuf);
			free(minBuf);
			free(mindata);
			return false;
		}
		free(imagedata.row_pointers);
		free(imagerows);
	}

	free(celBuf);
	free(minBuf);
	free(mindata);
	return true;
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
		*length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2) {
			if (((char)*pSrc) == -i) {
				// skip transparent pixels
				++pSrc;
			} else if (i != 0)
				break;
			if (*pSrc != (MICRO_WIDTH - i))
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			for (j = 0; j < MICRO_WIDTH - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i >= 0)
			break;

		for (i = 2; i != MICRO_HEIGHT; i += 2) {
			if (((char)*pSrc) != -i)
				break;
			++pSrc;
			if (*pSrc != (MICRO_WIDTH - i))
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			for (j = 0; j < MICRO_WIDTH - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i == MICRO_HEIGHT) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	case MET_RTRIANGLE:
		*length = MICRO_WIDTH * MICRO_HEIGHT / 2 + MICRO_HEIGHT;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2) {
			if (*pSrc != (MICRO_WIDTH - i))
				break;
			++pSrc;
			for (j = 0; j < MICRO_WIDTH - i; ++j) {
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

		for (i = 2; i != MICRO_HEIGHT; i += 2) {
			if (*pSrc != (MICRO_WIDTH - i))
				break;
			++pSrc;
			for (j = 0; j < MICRO_WIDTH - i; ++j) {
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
		if (i == MICRO_HEIGHT) {
			assert(pDst - tmpData == *length);
			return tmpData;
		}
		break;
	case MET_LTRAPEZOID:
		*length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2) {
			if (((char)*pSrc) == -i) {
				// skip transparent pixels
				++pSrc;
			} else if (i != 0)
				break;
			if (*pSrc != (MICRO_WIDTH - i))
				break;
			++pSrc;
			//if (i & 2)
			//	++pDst;
			pDst += i & 2;
			for (j = 0; j < MICRO_WIDTH - i; ++j) {
				*pDst = *pSrc;
				++pDst;
				++pSrc;
			}
		}
		if (i >= 0)
			break;

		for (i = MICRO_HEIGHT / 2; i != 0; i--) {
			if (*pSrc != MICRO_WIDTH)
				break;
			++pSrc;
			for (j = 0; j < MICRO_WIDTH; ++j) {
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
		*length = (MICRO_WIDTH * MICRO_HEIGHT) / 2 + MICRO_HEIGHT * (2 + MICRO_HEIGHT) / 4 + MICRO_HEIGHT / 2;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = MICRO_HEIGHT - 2; i >= 0; i -= 2) {
			if (*pSrc != (MICRO_WIDTH - i))
				break;
			++pSrc;
			for (j = 0; j < MICRO_WIDTH - i; ++j) {
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

		for (i = MICRO_HEIGHT / 2; i != 0; i--) {
			if (*pSrc != MICRO_WIDTH)
				break;
			++pSrc;
			for (j = 0; j < MICRO_WIDTH; ++j) {
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
		*length = MICRO_WIDTH * MICRO_HEIGHT;
		tmpData = (BYTE*)malloc(*length);
		memset(tmpData, 0, *length);
		pDst = tmpData;
		for (i = MICRO_HEIGHT; i != 0; i--) {
			if (*pSrc != MICRO_WIDTH)
				break;
			++pSrc;
			for (j = 0; j < MICRO_WIDTH; ++j) {
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

static void EncodeMicro(png_image_data* imagedata, int sx, int sy, MicroMetaData* mmd,
	BYTE* palette, int numcolors, int coloroffset, int numfixcolors)
{
	BYTE* celData = (BYTE*)malloc(2 * MICRO_WIDTH * MICRO_HEIGHT);
	memset(celData, 0, 2 * MICRO_WIDTH * MICRO_HEIGHT);

	bool hasAlpha = false;
	bool hasColor = false;
	BYTE* pHead = celData;
	BYTE* pBuf = celData + 1;
	for (int i = sy; i >= std::max(0, sy - (MICRO_HEIGHT - 1)); i--) {
		RGBA* data = (RGBA*)imagedata->row_pointers[i];
		data += sx;
		bool alpha = false;
		for (int j = 0; j < MICRO_WIDTH; j++) {
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
				if (imagedata->fixColorMask != NULL && imagedata->fixColorMask[i * imagedata->width + sx + j] != 0)
					*pBuf = imagedata->fixColorMask[i * imagedata->width + sx + j];
				else
					*pBuf = GetPalColor(data[j], palette, numcolors, coloroffset, numfixcolors);
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
	for (int i = 0; i < -(sy - (MICRO_HEIGHT - 1)); i++) {
		*pBuf = -(MICRO_WIDTH);
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
	// list of encodings sorted by size
	static const int encodings[4] = { MET_LTRIANGLE, MET_RTRIANGLE, MET_LTRAPEZOID, MET_RTRAPEZOID };
	for (int i = 0; i < lengthof(encodings); i++) {
		int type = encodings[i];
		uint32_t newLen;
		BYTE* remData = ReEncodeMicro(celData, type, &newLen);
		if (remData != NULL) {
			mmd->celData = remData;
			mmd->celLength = newLen;
			mmd->MicroType = type;
			free(celData);
			return;
		}
	}
	mmd->celData = celData;
	mmd->celLength = bestLen;
	mmd->MicroType = MET_TRANSPARENT;
}

void WritePNG2Min(png_image_data* imagedata, int numtiles, min_image_data* mindata, const char* destFolder, const char* prefix, BYTE* palette, int numcolors, int coloroffset, int numfixcolors)
{
	if (numtiles == 0) {
		CleanupImageData(imagedata, numtiles);
		return;
	}

	// convert PNG to CEL-micros
	int nummicros = imagedata[0].width / (MICRO_WIDTH);
	nummicros *= imagedata[0].height / MICRO_HEIGHT;

	int upscale = imagedata[0].width / (2 * MICRO_WIDTH);

	MicroMetaData* microData = (MicroMetaData*)malloc(sizeof(MicroMetaData) * numtiles * nummicros);
	memset(microData, 0, sizeof(MicroMetaData) * numtiles * nummicros);
	int n = 0;
	for (int i = 0; i < numtiles; i++) {
		png_image_data* img_data = &imagedata[i];
		if (upscale > 1) {
			for (int y = img_data->height - 1; y > 0; y -= MICRO_HEIGHT) {
				for (int x = 0; x < img_data->width; x += MICRO_WIDTH, n++) {
					EncodeMicro(img_data, x, y, &microData[n], palette, numcolors, coloroffset, numfixcolors);
				}
			}
		} else {
			for (int y = MICRO_HEIGHT - 1; y < img_data->height; y += MICRO_HEIGHT) {
				for (int x = 0; x < img_data->width; x += MICRO_WIDTH, n++) {
					EncodeMicro(img_data, x, y, &microData[n], palette, numcolors, coloroffset, numfixcolors);
				}
			}
		}
		//CleanupImageData(img_data, 1);
		free(img_data->data_ptr);
		free(img_data->row_pointers);
	}
	free(imagedata);

	// create output files
	char filename[256];
	FILE* f;
	// create MIN
	snprintf(filename, sizeof(filename), "%s%s.MIN", destFolder, prefix);
	f = fopen(filename, "wb");

	n = 0;
	std::vector<MicroMetaData*> uniqMicros;
	for (int i = 0; i < numtiles; i++) {
		for (int j = 0; j < nummicros; j++, n++) {
			MicroMetaData* mmd = &microData[n];
			if (mmd->celData == NULL) {
				// blank
				fput_int16(f, 0);
				continue;
			}
			auto it = uniqMicros.cbegin();
			for ( ; it != uniqMicros.cend(); ++it) {
				if ((*it)->celLength != mmd->celLength || (*it)->MicroType != mmd->MicroType)
					continue;
				if (memcmp((*it)->celData, mmd->celData, mmd->celLength) == 0)
					break;
			}
			uint16_t idx = ((it - uniqMicros.cbegin()) + 1);
			if (it == uniqMicros.cend()) {
				// add new micro
				uniqMicros.push_back(mmd);
			}
			if (upscale == 1)
				idx |= mmd->MicroType << 12;
			fput_int16(f, idx);
		}
		if (upscale > 1) {
			// add blanks to ensure 16 blocks
			for (int j = nummicros; j < 16 * upscale * upscale; j++) {
				// blank
				fput_int16(f, 0);
			}
		}
	}
	fclose(f);

	// create CEL
	snprintf(filename, sizeof(filename), "%s%s.CEL", destFolder, prefix);
	f = fopen(filename, "wb");

	fput_int32(f, uniqMicros.size());
	uint32_t addr = 4 + 4 * (uniqMicros.size() + 1);
	for (int n = 0; n < uniqMicros.size(); n++) {
		MicroMetaData* mmd = uniqMicros[n];
		fput_int32(f, addr);
		addr += mmd->celLength;
		if (upscale > 1)
			addr++;
		mmd->dataAddr = addr;
	}
	fput_int32(f, addr);

	for (int n = 0; n < uniqMicros.size(); n++) {
		MicroMetaData* mmd = uniqMicros[n];
		if (upscale > 1) // (prefix celdata with the type of the micro)
			fputc(mmd->MicroType, f);
		fwrite(mmd->celData, 1, mmd->celLength, f);
	}
	fclose(f);

	if (upscale > 1) {
		// create standard CEL to view in "Diablo 1 Graphics Tool"
		snprintf(filename, sizeof(filename), "%s%s_orig.CEL", destFolder, prefix);
		f = fopen(filename, "wb");

		fput_int32(f, uniqMicros.size());
		uint32_t addr = 4 + 4 * (uniqMicros.size() + 1);
		for (int n = 0; n < uniqMicros.size(); n++) {
			MicroMetaData* mmd = uniqMicros[n];
			fput_int32(f, addr);
			addr += mmd->celLength;
			mmd->dataAddr = addr;
		}
		fput_int32(f, addr);

		for (int n = 0; n < uniqMicros.size(); n++) {
			MicroMetaData* mmd = uniqMicros[n];
			fwrite(mmd->celData, 1, mmd->celLength, f);
		}
		fclose(f);
	}

	// create TMI
	/*snprintf(filename, sizeof(filename), "%s%s.TMI", destFolder, prefix);
	f = fopen(filename, "wb");
	fputc(0, f);
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
		fputc(bv, f);
	}
	fclose(f);

	// create SOL
	snprintf(filename, sizeof(filename), "%s%s.SOL", destFolder, prefix);
	f = fopen(filename, "wb");
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
		fputc(bv, f);
	}
	fclose(f);*/

	// cleanup memory
	for (int i = 0; i < nummicros; i++)
		free(microData[i].celData);
	free(microData);
}

static int Block2MicroTile(int i, int microTileLen)
{
	int idx = microTileLen - i;

	idx = idx - 2 * ((idx + 1) & 1);
	return idx;
}

#define Blk2Mcr(n, x) mindata[n - 1].levelBlocks[Block2MicroTile(x, microTileLen)] = 0;
void PatchMinData(int dunType, int microTileLen, min_image_data* mindata, int numtiles)
{
	switch (dunType) {
	case DTYPE_TOWN:
		// patch dMiniTiles - Town.MIN
		// pointless tree micros (re-drawn by dSpecial)
		Blk2Mcr(117, 3);
		Blk2Mcr(117, 5);
		Blk2Mcr(128, 2);
		Blk2Mcr(128, 3);
		Blk2Mcr(128, 4);
		Blk2Mcr(128, 5);
		Blk2Mcr(128, 6);
		Blk2Mcr(128, 7);
		Blk2Mcr(129, 3);
		Blk2Mcr(129, 5);
		Blk2Mcr(129, 7);
		Blk2Mcr(130, 2);
		Blk2Mcr(130, 4);
		Blk2Mcr(130, 6);
		Blk2Mcr(156, 2);
		Blk2Mcr(156, 3);
		Blk2Mcr(156, 4);
		Blk2Mcr(156, 5);
		Blk2Mcr(156, 6);
		Blk2Mcr(156, 7);
		Blk2Mcr(156, 8);
		Blk2Mcr(156, 9);
		Blk2Mcr(156, 10);
		Blk2Mcr(156, 11);
		Blk2Mcr(157, 3);
		Blk2Mcr(157, 5);
		Blk2Mcr(157, 7);
		Blk2Mcr(157, 9);
		Blk2Mcr(157, 11);
		Blk2Mcr(158, 2);
		Blk2Mcr(158, 4);
		Blk2Mcr(160, 2);
		Blk2Mcr(160, 3);
		Blk2Mcr(160, 4);
		Blk2Mcr(160, 5);
		Blk2Mcr(160, 6);
		Blk2Mcr(160, 7);
		Blk2Mcr(160, 8);
		Blk2Mcr(160, 9);
		Blk2Mcr(162, 2);
		Blk2Mcr(162, 4);
		Blk2Mcr(162, 6);
		Blk2Mcr(162, 8);
		Blk2Mcr(162, 10);
		Blk2Mcr(212, 3);
		Blk2Mcr(212, 4);
		Blk2Mcr(212, 5);
		Blk2Mcr(212, 6);
		Blk2Mcr(212, 7);
		Blk2Mcr(212, 8);
		Blk2Mcr(212, 9);
		Blk2Mcr(212, 10);
		Blk2Mcr(212, 11);
		//Blk2Mcr(214, 4);
		//Blk2Mcr(214, 6);
		Blk2Mcr(216, 2);
		Blk2Mcr(216, 4);
		Blk2Mcr(216, 6);
		//Blk2Mcr(217, 4);
		//Blk2Mcr(217, 6);
		//Blk2Mcr(217, 8);
		//Blk2Mcr(358, 4);
		//Blk2Mcr(358, 5);
		//Blk2Mcr(358, 6);
		//Blk2Mcr(358, 7);
		//Blk2Mcr(358, 8);
		//Blk2Mcr(358, 9);
		//Blk2Mcr(358, 10);
		//Blk2Mcr(358, 11);
		//Blk2Mcr(358, 12);
		//Blk2Mcr(358, 13);
		//Blk2Mcr(360, 4);
		//Blk2Mcr(360, 6);
		//Blk2Mcr(360, 8);
		//Blk2Mcr(360, 10);
		// fix bad artifact
		Blk2Mcr(233, 6);
		// useless black micros
		Blk2Mcr(426, 1);
		Blk2Mcr(427, 0);
		Blk2Mcr(427, 1);
		Blk2Mcr(429, 1);
		// fix bad artifacts
		Blk2Mcr(828, 12);
		Blk2Mcr(828, 13);
		Blk2Mcr(1018, 2);
		// useless black micros
		Blk2Mcr(1143, 0);
		Blk2Mcr(1145, 0);
		Blk2Mcr(1145, 1);
		Blk2Mcr(1146, 0);
		Blk2Mcr(1153, 0);
		Blk2Mcr(1155, 1);
		Blk2Mcr(1156, 0);
		Blk2Mcr(1169, 1);
		Blk2Mcr(1170, 0);
		Blk2Mcr(1170, 1);
		Blk2Mcr(1172, 1);
		Blk2Mcr(1176, 1);
		Blk2Mcr(1199, 1);
		Blk2Mcr(1200, 0);
		Blk2Mcr(1200, 1);
		Blk2Mcr(1202, 1);
		Blk2Mcr(1203, 1);
		Blk2Mcr(1205, 1);
		Blk2Mcr(1212, 0);
		Blk2Mcr(1219, 0);
		if (numtiles > 1258)
			//#ifdef HELLFIRE
			// fix bad artifact
			Blk2Mcr(1273, 7);
		break;
	case DTYPE_CATHEDRAL:
		// patch dMiniTiles - L1.MIN
		// useless black micros
		Blk2Mcr(107, 0);
		Blk2Mcr(107, 1);
		Blk2Mcr(109, 1);
		Blk2Mcr(137, 1);
		Blk2Mcr(138, 0);
		Blk2Mcr(138, 1);
		Blk2Mcr(140, 1);
		break;
	case DTYPE_CATACOMBS:
		break;
	case DTYPE_CAVES:
		// patch dMiniTiles - L3.MIN
		// fix bad artifact
		Blk2Mcr(82, 4);
		break;
	case DTYPE_HELL:
		break;
	case DTYPE_NEST:
		// patch dMiniTiles - L6.MIN
		// useless black micros
		Blk2Mcr(21, 0);
		Blk2Mcr(21, 1);
		// fix bad artifacts
		Blk2Mcr(132, 7);
		Blk2Mcr(366, 1);
		break;
	case DTYPE_CRYPT:
		// patch dMiniTiles - L5.MIN
		// useless black micros
		Blk2Mcr(130, 0);
		Blk2Mcr(130, 1);
		Blk2Mcr(132, 1);
		Blk2Mcr(134, 0);
		Blk2Mcr(134, 1);
		Blk2Mcr(149, 0);
		Blk2Mcr(149, 1);
		Blk2Mcr(149, 2);
		Blk2Mcr(150, 0);
		Blk2Mcr(150, 1);
		Blk2Mcr(150, 2);
		Blk2Mcr(150, 4);
		Blk2Mcr(151, 0);
		Blk2Mcr(151, 1);
		Blk2Mcr(151, 3);
		Blk2Mcr(152, 0);
		Blk2Mcr(152, 1);
		Blk2Mcr(152, 3);
		Blk2Mcr(152, 5);
		Blk2Mcr(153, 0);
		Blk2Mcr(153, 1);
		// fix bad artifact
		Blk2Mcr(156, 2);
		// useless black micros
		Blk2Mcr(172, 0);
		Blk2Mcr(172, 1);
		Blk2Mcr(172, 2);
		Blk2Mcr(173, 0);
		Blk2Mcr(173, 1);
		Blk2Mcr(174, 0);
		Blk2Mcr(174, 1);
		Blk2Mcr(174, 2);
		Blk2Mcr(174, 4);
		Blk2Mcr(175, 0);
		Blk2Mcr(175, 1);
		Blk2Mcr(176, 0);
		Blk2Mcr(176, 1);
		Blk2Mcr(176, 3);
		Blk2Mcr(177, 0);
		Blk2Mcr(177, 1);
		Blk2Mcr(177, 3);
		Blk2Mcr(177, 5);
		Blk2Mcr(178, 0);
		Blk2Mcr(178, 1);
		Blk2Mcr(179, 0);
		Blk2Mcr(179, 1);
		break;
	}
}

/*
 * (integer) upscale a MIN file (+ its corresponding CEL)
 * @param minname: the path of the MIN file
 * @param multiplier: the extent of the upscale
 * @param celname: the path of the CEL file
 * @param dunType: reset useless micros based on the type of the dungeon. Set to DTYPE_NONE to skip.
 * @param palette: the palette to be used
 * @param numcolors: the number of colors in the palette
 * @param coloroffset: the offset to be applied when selecting a color from the palette
 * @param destFolder: the output folder
 * @param prefix: the base name of the generated output files
 */
void UpscaleMin(const char* minname, int multiplier, const char* celname, int dunType,
	BYTE* palette, int numcolors, int coloroffset, int numfixcolors,
	const char* destFolder, const char* prefix)
{
	int numtiles, columns = 0, rows = 0;
	BYTE *celBuf, *minBuf;
	min_image_data* mindata = ReadMinData(minname, columns, rows, celname, NULL, &numtiles, &celBuf, &minBuf);
	if (mindata == NULL)
		return;

	// reset useless micros
	PatchMinData(dunType, columns * rows, mindata, numtiles);

	// write the png(s)
	png_image_data* imagedata = (png_image_data*)malloc(sizeof(png_image_data) * numtiles);
	for (int i = 0; i < numtiles; i++) {
		// prepare pngdata
		imagedata[i].width = columns * MICRO_WIDTH;
		imagedata[i].height = rows * MICRO_HEIGHT;
		RGBA *imagerows = (RGBA *)malloc(sizeof(RGBA) * imagedata[i].height * imagedata[i].width);
		// make the background transparent
		memset(imagerows, 0, sizeof(RGBA) * imagedata[i].height * imagedata[i].width);
		imagedata[i].row_pointers = (png_bytep*)malloc(imagedata[i].height * sizeof(void*));
		for (int n = 0; n < imagedata[i].height; n++) {
			imagedata[i].row_pointers[n] = (png_bytep)&imagerows[imagedata[i].width * n];
		}
		imagedata[i].data_ptr = (png_bytep)imagerows;

		//lastLine += imagedata.width * (imagedata.height - 1);
		// CelBlitSafe(lastLine, celdata[i].data, celdata[i].dataSize, imagedata.width, imagedata.width, palette, coloroffset, 0);

		RGBA* dst = (RGBA*)imagedata[i].row_pointers[MICRO_HEIGHT - 1];
		uint16_t* src = mindata[i].levelBlocks;
		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < columns; x++, src++) {
				uint16_t levelCelBlock = *src;
				RenderMicro(dst, imagedata[i].width, levelCelBlock, DMT_NONE, celBuf, false, palette, coloroffset, numfixcolors);
				dst += MICRO_WIDTH;
			}
			dst += imagedata[i].width * MICRO_HEIGHT - columns * MICRO_WIDTH;
		}

		// prepare meta-info
		dst -= imagedata[i].width * MICRO_HEIGHT;
		imagedata[i].fixColorMask = (BYTE*)malloc(sizeof(BYTE) * imagedata[i].height * imagedata[i].width);
		memset(imagedata[i].fixColorMask, 0, sizeof(BYTE) * imagedata[i].height * imagedata[i].width);
		BYTE* dstB = &imagedata[i].fixColorMask[(imagedata[i].height - 1) * imagedata[i].width];
		for (int y = 0; y < imagedata[i].height; y++) {
			for (int x = 0; x < imagedata[i].width; x++) {
				if (dst[x].a != 0 && dst[x].a != 255) {
					dstB[x] = dst[x].a;
					dst[x].a = 255;
				}
			}
			dstB -= imagedata[i].width;
			dst -= imagedata[i].width;
		}
	}

	free(minBuf);
	free(celBuf);

	// upscale the png data
	UpscalePNGImages(imagedata, numtiles, multiplier, palette, numcolors, coloroffset, numfixcolors, 2);

	// convert pngs back to min/cel
	WritePNG2Min(imagedata, numtiles, mindata, destFolder, prefix, palette, numcolors, coloroffset, numfixcolors);
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
 * @param destFolder: the output folder
 * @param prefix: the base name of the generated output files
 * @param palette: the palette to be used to generate the CEL file
 * @param numcolors: the number of colors in the palette
 * @param coloroffset: added to the color-values which are selected from the palette
 * @return 0 if the function succeeds
 */
int PNG2Min(const char* megatiles[][4], int nummegas, int blocks,
	const char* destFolder, const char* prefix,
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
		if (imgdata->width != 2 * MICRO_WIDTH) {
			CleanupImageData(imagedata, pn);
			return 2; // invalid image (width)
		}
		if (imgdata->height > MICRO_HEIGHT * blocks / 2) {
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
		for (int i = imgdata->height - 1; i >= 0; i -= MICRO_HEIGHT) {
			EncodeMicro(imgdata, 0, i, &cpd->micros[n++], palette, numcolors, coloroffset, 0);
			EncodeMicro(imgdata, MICRO_WIDTH, i, &cpd->micros[n++], palette, numcolors, coloroffset, 0);
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
	snprintf(filename, sizeof(filename), "%s%s.TIL", destFolder, prefix);
	f0 = fopen(filename, "wb");
	for (int n = 0; n < nummegas; n++) {
		for (int i = 0; i < 4; i++) {
			fput_int16(f0, megaData[n].pieces[i]);
		}
	}
	fclose(f0);

	// create CEL
	snprintf(filename, sizeof(filename), "%s%s.CEL", destFolder, prefix);
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
	snprintf(filename, sizeof(filename), "%s%s.MIN", destFolder, prefix);
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
	snprintf(filename, sizeof(filename), "%s%s.TMI", destFolder, prefix);
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
	snprintf(filename, sizeof(filename), "%s%s.SOL", destFolder, prefix);
	f0 = fopen(filename, "wb");
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
	snprintf(filename, sizeof(filename), "%s%s.TXT", destFolder, prefix);
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

int main()
{
	/*{ // upscale tiles of the levels (fails if the output-folder structure is not prepared)
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\TownData\\Town.PAL");
		UpscaleMin("f:\\MPQE\\Work\\Levels\\TownData\\Town.MIN", 2, "f:\\MPQE\\Work\\Levels\\TownData\\Town.CEL", DTYPE_TOWN, pal, 128, 0, 0,
			"f:\\outmin\\Levels\\TownData\\", "Town");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\L1Data\\L1_1.PAL");
		UpscaleMin("f:\\MPQE\\Work\\Levels\\L1Data\\L1.MIN", 2, "f:\\MPQE\\Work\\Levels\\L1Data\\L1.CEL", DTYPE_CATHEDRAL, pal, 128, 0, 0,
			"f:\\outmin\\Levels\\L1Data\\", "L1");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\L2Data\\L2_1.PAL");
		UpscaleMin("f:\\MPQE\\Work\\Levels\\L2Data\\L2.MIN", 2, "f:\\MPQE\\Work\\Levels\\L2Data\\L2.CEL", DTYPE_CATACOMBS, pal, 128, 0, 0,
			"f:\\outmin\\Levels\\L2Data\\", "L2");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\L3Data\\L3_1.PAL");
		UpscaleMin("f:\\MPQE\\Work\\Levels\\L3Data\\L3.MIN", 2, "f:\\MPQE\\Work\\Levels\\L3Data\\L3.CEL", DTYPE_CAVES, pal, 128, 0, 32, 
			"f:\\outmin\\Levels\\L3Data\\", "L3");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\L4Data\\L4_1.PAL");
		UpscaleMin("f:\\MPQE\\Work\\Levels\\L4Data\\L4.MIN", 2, "f:\\MPQE\\Work\\Levels\\L4Data\\L4.CEL", DTYPE_HELL, pal, 128, 0, 32,
			"f:\\outmin\\Levels\\L4Data\\", "L4");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\TownData\\Town.PAL");
		UpscaleMin("f:\\MPQE\\Work\\NLevels\\TownData\\Town.MIN", 2, "f:\\MPQE\\Work\\NLevels\\TownData\\Town.CEL", DTYPE_TOWN, pal, 256, 0, 0,
			"f:\\outmin\\NLevels\\TownData\\", "Town");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\NLevels\\L5Data\\L5Base.PAL");
		UpscaleMin("f:\\MPQE\\Work\\NLevels\\L5Data\\L5.MIN", 2, "f:\\MPQE\\Work\\NLevels\\L5Data\\L5.CEL", DTYPE_CRYPT, pal, 128, 0, 32,
			"f:\\outmin\\NLevels\\L5Data\\", "L5");
		free(pal);
	}
	{
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\NLevels\\L6Data\\L6Base1.PAL");
		UpscaleMin("f:\\MPQE\\Work\\NLevels\\L6Data\\L6.MIN", 2, "f:\\MPQE\\Work\\NLevels\\L6Data\\L6.CEL", DTYPE_NEST, pal, 128, 0, 32, 
			"f:\\outmin\\NLevels\\L6Data\\", "L6");
		free(pal);
	}*/
	/*{ // sample code to convert TIL, CEL, MIN, PAL to PNGs (fails if the output-folder structure is not prepared)
		BYTE* pal = LoadPal("f:\\MPQE\\Work\\Levels\\TownData\\Town.PAL");
		Min2PNG("f:\\MPQE\\Work\\Levels\\TownData\\Town.MIN", 0, 0,
			"f:\\MPQE\\Work\\Levels\\TownData\\Town.CEL",
			"f:\\MPQE\\Work\\Levels\\TownData\\Town.TIL", "f:\\outmin\\TownData\\", pal, 0);
		free(pal);
	}*/
	/*{ // sample code to convert TIL, CEL, MIN, PAL to PNGs where the original colors of the palette are not unique (fails if the output-folder structure is not prepared)
		Pal2PalUniq("f:\\MPQE\\Work\\Levels\\L3Data\\L3_1.PAL", 256, "f:\\outmin\\Levels\\L3Data\\L3_1_uniq.PAL");
		BYTE* pal = LoadPal("f:\\outmin\\Levels\\L3Data\\L3_1_uniq.PAL");
		Min2PNG("f:\\MPQE\\Work\\Levels\\TownData\\Town.MIN", 0, 0,
			"f:\\MPQE\\Work\\Levels\\TownData\\Town.CEL",
			"f:\\MPQE\\Work\\Levels\\TownData\\Town.TIL", "f:\\outmin\\Levels\\L3Data\\", pal, 0);
		free(pal);
	}*/
	/*{ // sample code to convert upscaled(2x) MIN, CEL (TIL, PAL) to PNGs
		BYTE* pal = LoadPal("f:\\outmin\\Levels\\L3Data\\L3_1_uniq.PAL");
		Min2PNG("f:\\outmin\\Levels\\L3Data\\L3.MIN", 2 * 2, 8 * 2,
			"f:\\outmin\\Levels\\L3Data\\L3.CEL",
			"f:\\MPQE\\Work\\Levels\\L3Data\\L3.TIL", "f:\\outmin\\Levels\\L3Data\\", pal, 0);
		free(pal);
	}*/
}