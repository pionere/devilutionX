#include "art.h"

#include "all.h"

#include "utils/display.h"
#include "utils/sdl_compat.h"
#include "storm/storm.h"
#include "engine.h"
#include "debug.h"

DEVILUTION_BEGIN_NAMESPACE

/*
 * Load an 8bpp PCX file to the given art entity.
 */
bool LoadArt(const char* pszFileName, Art* art, int frames, SDL_Color* pPalette)
{
	unsigned width, height, bpp;
	size_t dataSize;
	BYTE* fileBuffer;
	HANDLE hFile;

	assert(art != NULL);
	if (art->surface != NULL)
		return false;

	art->frames = frames;

#ifdef _DEBUG
	assert(pszFileName != NULL);
	dataSize = strlen(pszFileName);
	if (dataSize < 4 || strcasecmp(&pszFileName[dataSize - 4], ".pcx") != 0) {
		SDL_Log("Failed to load image meta");
		return false;
	}
#endif

	// check if the file exists
	hFile = SFileOpenFile(pszFileName);
	if (hFile == NULL) {
		return false;
	}

	// open/read the required data
	dataSize = SFileGetFileSize(hFile);

	fileBuffer = DiabloAllocPtr(dataSize);
	if (dataSize < sizeof(PCXHEADER) || !SFileReadFile(hFile, fileBuffer, dataSize)) {
		free(fileBuffer);
		SFileCloseFile(hFile);
		SDL_Log("Failed to load image meta");
		return false;
	}
	SFileCloseFile(hFile);

	// process the header
	PCXHEADER &pcxhdr = *(PCXHEADER*)fileBuffer;
	bpp = pcxhdr.BitsPerPixel;
	assert(bpp == 8);
	//if (bpp != 8)
	//	pPalette = NULL;
	width = SwapLE16(pcxhdr.Xmax) - SwapLE16(pcxhdr.Xmin) + 1;
	height = SwapLE16(pcxhdr.Ymax) - SwapLE16(pcxhdr.Ymin) + 1;

	// create the surface
	Uint32 format;
	//switch (bpp) {
	//case 8:
		format = SDL_PIXELFORMAT_INDEX8;
	/*	break;
	case 24:
		format = SDL_PIXELFORMAT_RGB888;
		break;
	case 32:
		format = SDL_PIXELFORMAT_RGBA8888;
		break;
	default:
		format = SDL_PIXELFORMAT_UNKNOWN;
		break;
	}*/
	SDL_Surface* artSurface = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, width, height, bpp, format);

	// process the body
	BYTE byte, *pSrc;
	BYTE* pDst = static_cast<BYTE*>(artSurface->pixels);
	size_t dwBufferSize = artSurface->pitch * artSurface->format->BytesPerPixel * height;
	// If the given buffer is larger than width * height, assume the extra data
	// is scanline padding.
	//
	// This is useful because in SDL the pitch size is often slightly larger
	// than image width for efficiency.
	const int xSkip = dwBufferSize / height - width;
	assert(xSkip >= 0);
	// width of PCX data is always even -> need to skip a bit if the width is odd
	const BYTE srcSkip = width % 2;
	assert(srcSkip == width - pcxhdr.BytesPerLine);
	const BYTE PCX_MAX_SINGLE_PIXEL = 0xBF;
	const BYTE PCX_RUNLENGTH_MASK = 0x3F;
	pSrc = fileBuffer + sizeof(PCXHEADER);
	for (unsigned y = 0; y < height; y++) {
		for (unsigned x = 0; x < width; pSrc++) {
			byte = *pSrc;
			if (byte <= PCX_MAX_SINGLE_PIXEL) {
				*pDst = byte;
				pDst++;
				x++;
				continue;
			}
			byte &= PCX_RUNLENGTH_MASK;
			pSrc++;
			memset(pDst, *pSrc, byte);
			pDst += byte;
			x += byte;
		}
		pSrc += srcSkip;
		// Skip the pitch padding.
		pDst += xSkip;
	}

	// process the palette at the end of the body
	if (pPalette != NULL) {
		palette_create_sdl_colors(pPalette, (BYTE (&)[256][3])*(fileBuffer + dataSize - (256 * 3)));
	}

	free(fileBuffer);

	art->surface = artSurface;
	art->logical_width = artSurface->w;
	art->frame_height = height / art->frames;
	//art->palette_version = back_surface_palette_version - 1;
	ScaleSurfaceToOutput(&art->surface);
	return true;
}

void LoadMaskedArt(const char* pszFile, Art* art, int frames, int mask)
{
	if (LoadArt(pszFile, art, frames))
		SDLC_SetColorKey(art->surface, mask);
}

DEVILUTION_END_NAMESPACE
