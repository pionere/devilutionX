/**
 * @file cl2_render.cpp
 *
 * CL2 rendering.
 */
#include "cl2_render.h"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Blit CL2 sprite to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CL2 pixel stream (run-length encoded)
 * @param nDataSize Size of CL2 in bytes
 * @param nWidth Width of sprite
 */
static void Cl2Blit(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	int i;
	int8_t width;
	BYTE fill, *dst;
	const BYTE *src, *end;

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
					//if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							*dst = fill;
							dst++;
							width--;
						}
						continue;
					//}
				} else {
					//if (dst < gpBufEnd && dst >= gpBufStart) {
						i -= width;
						while (width != 0) {
							*dst = *src;
							src++;
							dst++;
							width--;
						}
						continue;
					//} else {
					//	src += width;
					//}
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
	int i;
	int8_t width;
	BYTE fill, *dst;
	const BYTE *src, *end;

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
