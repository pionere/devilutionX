/**
 * @file cl2_render.cpp
 *
 * CL2 rendering.
 */
#include "cl2_render.h"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Apply the color swaps to a CL2 sprite
 * @param p CL2 buffer
 * @param ttbl Palette translation table
 * @param nCel number of frames in the CL2 file
 */
void Cl2ApplyTrans(BYTE* p, const BYTE* ttbl, int nCel)
{
	int i, nDataSize;
	int8_t width;
	BYTE* dst;
	const BYTE* end;

	assert(p != NULL);
	assert(ttbl != NULL);

	for (i = 1; i <= nCel; i++) {
		dst = const_cast<BYTE*>(CelGetFrameClippedAt(p, i, 0, &nDataSize));
		end = &dst[nDataSize];
		while (dst != end) {
			width = *dst++;
			assert(dst <= end);
			if (width < 0) {
				width = -width;
				if (width > 65) {
					*dst = ttbl[*dst];
					dst++;
					assert(dst <= end);
				} else {
					assert(dst + width <= end);
					while (width--) {
						*dst = ttbl[*dst];
						dst++;
					}
				}
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 */
static void Cl2Blit(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	const BYTE *src, *end;
	BYTE fill, *dst;
	int i;
	int8_t width;

	// assert(gpBuffer != NULL);
	// assert(pDecodeTo != NULL);
	// assert(pRLEBytes != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width < 0) {
				width = -width;
				if (width > 65) {
					width -= 65;
					fill = *src++;
					if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							*dst = fill;
							dst++;
							width--;
						}
						continue;
					}
				} else {
					if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							*dst = *src;
							src++;
							dst++;
							width--;
						}
						continue;
					} else {
						src += width;
					}
				}
			}
			while (true) {
				if (width <= i) {
					dst += width;
					i -= width;
					break;
				} else {
					dst += i;
					width -= i;
					i = nWidth;
					dst -= BUFFER_WIDTH + nWidth;
				}
			}
		}
	}
}

/**
 * @brief Blit a solid colder shape one pixel larger then the given sprite shape, to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 * @param col Color index from current palette
 */
static void Cl2BlitOutline(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth, BYTE col)
{
	const BYTE *src, *end;
	BYTE* dst;
	int i;
	int8_t width;

	// assert(gpBuffer != NULL);
	// assert(pDecodeTo != NULL);
	// assert(pRLEBytes != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width < 0) {
				width = -width;
				if (width > 65) {
					width -= 65;
					if (*src++ != 0 && dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						dst[-1] = col;
						dst[width] = col;
						while (width != 0) {
							dst[-BUFFER_WIDTH] = col;
							dst[BUFFER_WIDTH] = col;
							dst++;
							width--;
						}
						continue;
					}
				} else {
					if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							if (*src++ != 0) {
								dst[-1] = col;
								dst[1] = col;
								dst[-BUFFER_WIDTH] = col;
								// BUGFIX: only set `if (dst+BUFFER_WIDTH < gpBufEnd)`
								dst[BUFFER_WIDTH] = col;
							}
							dst++;
							width--;
						}
						continue;
					} else {
						src += width;
					}
				}
			}
			while (true) {
				if (width <= i) {
					dst += width;
					i -= width;
					break;
				} else {
					dst += i;
					width -= i;
					i = nWidth;
					dst -= BUFFER_WIDTH + nWidth;
				}
			}
		}
	}
}

/**
 * @brief Blit CL2 sprite, and apply lighting, to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth With of CL2 sprite
 * @param pTable Light color table
 */
static void Cl2BlitLight(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth, const BYTE* pTable)
{
	const BYTE *src, *end;
	BYTE fill, *dst;
	int i;
	int8_t width;

	// assert(gpBuffer != NULL);
	// assert(pDecodeTo != NULL);
	// assert(pRLEBytes != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width < 0) {
				width = -width;
				if (width > 65) {
					width -= 65;
					fill = pTable[*src++];
					if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							*dst = fill;
							dst++;
							width--;
						}
						continue;
					}
				} else {
					if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							*dst = pTable[*src];
							src++;
							dst++;
							width--;
						}
						continue;
					} else {
						src += width;
					}
				}
			}
			while (true) {
				if (width <= i) {
					dst += width;
					i -= width;
					break;
				} else {
					dst += i;
					width -= i;
					i = nWidth;
					dst -= BUFFER_WIDTH + nWidth;
				}
			}
		}
	}
}

/**
 * @brief Blit an outline one pixel larger then the given sprite shape to the back buffer at the given coordinates
 * @param col Color index from current palette
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2DrawOutline(BYTE col, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	gpBufEnd -= BUFFER_WIDTH;
	Cl2BlitOutline(pDecodeTo, pRLEBytes, nDataSize, nWidth, col);
	gpBufEnd += BUFFER_WIDTH;
}

/**
 * @brief Blit CL2 sprite, and apply a given lighting, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 * @param light index of the light shade to use
 */
void Cl2DrawLightTbl(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth, BYTE light)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[sx + BUFFER_WIDTH * sy];

	if (light != 0)
		Cl2BlitLight(pDecodeTo, pRLEBytes, nDataSize, nWidth, ColorTrns[light]);
	else
		Cl2Blit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

DEVILUTION_END_NAMESPACE
