/**
 * @file cl2_render.cpp
 *
 * CL2 rendering.
 */
#include "cl2_render.h"

#include "render.h"

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
	const BYTE *src, *end;
	BYTE fill, *dst;
	int i, width;

	// assert(gpBuffer != NULL);
	// assert(pDecodeTo != NULL);
	// assert(pRLEBytes != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = (int8_t)*src++;
			if (width < 0) {
				width = -width;
				if (width > 65) {
					width -= 65;
					fill = *src++;
						i -= width;
						while (width != 0) {
							*dst = fill;
							dst++;
							width--;
						}
						continue;
				} else {
						i -= width;
						while (width != 0) {
							*dst = *src;
							src++;
							dst++;
							width--;
						}
						continue;
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
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @param nCel CL2 frame number
 * @param nWidth Width of sprite
 */
void Cl2Draw(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize, &sy);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	Cl2Blit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

DEVILUTION_END_NAMESPACE
