/**
 * @file cel_render.cpp
 *
 * CEL rendering.
 */
#include "cel_render.h"

#include "render.h"

DEVILUTION_BEGIN_NAMESPACE

/**
 * @brief Blit CEL sprite to the given buffer, checks for drawing outside the buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
static void CelBlit(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	const BYTE *src, *end;
	BYTE* dst;
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
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst >= gpBufStart) {
					memcpy(dst, src, width);
				}
				src += width;
				dst += width;
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief apply trn to the CEL sprite and blit to the given buffer
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 * @param tbl Palette translation table
 */
static void CelBlitTrnTbl(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth, const BYTE* tbl)
{
	const BYTE *src, *end;
	BYTE* dst;
	int i, width;

	// assert(gpBuffer != NULL);
	// assert(pDecodeTo != NULL);
	// assert(pRLEBytes != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;
	//assert(tbl != NULL);
	//if (tbl == NULL)
	//	tbl = ColorTrns[light_trn_index];

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = (int8_t)*src++;
			if (width >= 0) {
				i -= width;
				if (dst < gpBufEnd && dst >= gpBufStart) {
					if (width & 1) {
						dst[0] = tbl[src[0]];
						src++;
						dst++;
					}
					width >>= 1;
					if (width & 1) {
						dst[0] = tbl[src[0]];
						dst[1] = tbl[src[1]];
						src += 2;
						dst += 2;
					}
					width >>= 1;
					for ( ; width != 0; width--) {
						dst[0] = tbl[src[0]];
						dst[1] = tbl[src[1]];
						dst[2] = tbl[src[2]];
						dst[3] = tbl[src[3]];
						src += 4;
						dst += 4;
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief Blit CEL sprite to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 */
void CelDraw(int sx, int sy, const CelImageBuf* pCelBuff, int nCel)
{
	int nDataSize;
	BYTE* pDecodeTo;
	const BYTE* pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrame((const BYTE*)pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	CelBlit(pDecodeTo, pRLEBytes, nDataSize, pCelBuff->ciWidth);
}

/**
 * @brief Blit CEL sprite, and apply trn, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 * @param tbl Palette translation table
 */
void CelDrawTrnTbl(int sx, int sy, const CelImageBuf* pCelBuff, int nCel, const BYTE* tbl)
{
	int nDataSize;
	BYTE* pDecodeTo;
	const BYTE* pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrame((const BYTE*)pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	/*if (tbl == NULL) {
		if (light_trn_index == 0) {
			CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
			return;
		}
		tbl = ColorTrns[light_trn_index];
	}*/
	CelBlitTrnTbl(pDecodeTo, pRLEBytes, nDataSize, pCelBuff->ciWidth, tbl);
}

/**
 * @brief apply light trn to the CEL sprite and blit to the given buffer using stippled transparency
 * @param pDecodeTo The output buffer
 * @param pRLEBytes CEL pixel stream (run-length encoded)
 * @param nDataSize Size of CEL in bytes
 * @param nWidth Width of sprite
 */
static void CelBlitLightTrans(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	const BYTE *tbl, *src, *end;
	BYTE* dst;
	int i, width;
	BOOLEAN shift;

	// assert(gpBuffer != NULL);
	// assert(pDecodeTo != NULL);
	// assert(pRLEBytes != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;
	tbl = ColorTrns[light_trn_index];
	shift = (BYTE)(size_t)dst & 1;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth, shift = 1 - shift) {
		for (i = nWidth; i != 0; ) {
			width = (int8_t)*src++;
			if (width >= 0) {
				i -= width;
					if (((BYTE)(size_t)dst & 1) == shift) {
						if (!(width & 1)) {
							goto L_ODD;
						} else {
							src++;
							dst++;
						L_EVEN:
							width >>= 1;
							if (width & 1) {
								dst[0] = tbl[src[0]];
								src += 2;
								dst += 2;
							}
							width >>= 1;
							for ( ; width != 0; width--) {
								dst[0] = tbl[src[0]];
								dst[2] = tbl[src[2]];
								src += 4;
								dst += 4;
							}
						}
					} else {
						if (!(width & 1)) {
							goto L_EVEN;
						} else {
							dst[0] = tbl[src[0]];
							src++;
							dst++;
						L_ODD:
							width >>= 1;
							if (width & 1) {
								dst[1] = tbl[src[1]];
								src += 2;
								dst += 2;
							}
							width >>= 1;
							for ( ; width != 0; width--) {
								dst[1] = tbl[src[1]];
								dst[3] = tbl[src[3]];
								src += 4;
								dst += 4;
							}
						}
					}
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief Blit CEL sprite, and optionally use stippled-transparency or light trn, to the given buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawLightTrans(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize, &sy);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	if (gbCelTransparencyActive)
		CelBlitLightTrans(pDecodeTo, pRLEBytes, nDataSize, nWidth);
	else // if (light_trn_index != 0) - do not bypass the light translations
		CelBlitTrnTbl(pDecodeTo, pRLEBytes, nDataSize, nWidth, ColorTrns[light_trn_index]);
	// else
	//	CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

/**
 * @brief Blit CEL sprite, and apply a given lighting/trn, to the given buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 * @param light index of the light shade/trn to use
 */
void CelClippedDrawLightTbl(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth, BYTE light)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize, &sy);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	if (light != 0)
		CelBlitTrnTbl(pDecodeTo, pRLEBytes, nDataSize, nWidth, ColorTrns[light]);
	else
		CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

static void CelBlitOutline(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth, BYTE col)
{
	const BYTE *src, *end;
	BYTE* dst;
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
			if (width >= 0) {
				i -= width;
						while (width != 0) {
							if (*src++) {
								dst[-BUFFER_WIDTH] = col;
								dst[-1] = col;
								dst[1] = col;
								dst[BUFFER_WIDTH] = col;
							}
							dst++;
							width--;
						}
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

/**
 * @brief Blit CEL sprite with an outline one pixel larger then the given sprite shape to the target buffer at the given coordinates
 * @param col color of the sprite and the outline (Color index from current palette)
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDrawOutline(BYTE col, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize, &sy);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	// gpBufStart += BUFFER_WIDTH;
	// gpBufEnd -= BUFFER_WIDTH;
	CelBlitOutline(pDecodeTo, pRLEBytes, nDataSize, nWidth, col);
	// gpBufStart -= BUFFER_WIDTH;
	// gpBufEnd += BUFFER_WIDTH;
}

/**
 * @brief Blit a slice of a CEL sprite to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 * @param fy the first (lowest) line of the sprite to blit
 * @param ny the number of lines to blit
 */
void CelClippedDrawSlice(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth, unsigned fy, unsigned ny)
{
	int nDataSize;
	const BYTE *pRLEBytes;
	BYTE* pDecodeTo;

	unsigned blocks = fy / CEL_BLOCK_HEIGHT;

	const BYTE *src;
	BYTE* dst;
	int i, width;

	pRLEBytes = CelGetFrameClippedAt(pCelBuff, nCel, blocks, &nDataSize);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy - fy)];

	src = pRLEBytes;

	fy = fy % CEL_BLOCK_HEIGHT; // -= blocks * CEL_BLOCK_HEIGHT;
	while (fy-- != 0) {
		for (i = nWidth; i != 0; ) {
			width = (int8_t)*src++;
			if (width >= 0) {
				i -= width;
				src += width;
			} else {
				i += width;
			}
		}
	}

	dst = pDecodeTo;
	for ( ; ny-- != 0; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = (int8_t)*src++;
			if (width >= 0) {
				i -= width;
				memcpy(dst, src, width);
				src += width;
				dst += width;
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

unsigned CelClippedWidth(const BYTE* pCelBuff)
{
	int nDataSize;
	const BYTE *pRLEBytes;

	pRLEBytes = CelGetFrameClippedAt(pCelBuff, 1, 0, &nDataSize);

	const BYTE *src, *end;
	int width;

	src = pRLEBytes;

	end = CelGetFrameClippedAt(pCelBuff, 1, 1, &nDataSize);

	unsigned n = 0;
	while (src < end) {
		width = (int8_t)*src++;
		if (width >= 0) {
			n += width;
			src += width;
		} else {
			n -= width;
		}
	}

	return n / CEL_BLOCK_HEIGHT;
}

DEVILUTION_END_NAMESPACE
