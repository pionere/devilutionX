/**
 * @file cel_render.cpp
 *
 * CEL rendering.
 */
#include "cel_render.h"

#include "all.h"

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
	int i;
	int8_t width;
	const BYTE *src, *end;
	BYTE* dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
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
	int i;
	int8_t width;
	const BYTE *src, *end;
	BYTE* dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;
	//assert(tbl != NULL);

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
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
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 */
void CelDraw(int sx, int sy, const CelImageBuf* pCelBuff, int nCel)
{
	int nDataSize;
	const BYTE* pRLEBytes;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrame((const BYTE*)pCelBuff, nCel, &nDataSize);

	CelBlit(&gpBuffer[BUFFERXY(sx, sy)], pRLEBytes, nDataSize, pCelBuff->ciWidth);
}

/**
 * @brief Blit CEL sprite, and apply trn, to the back buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
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

	pRLEBytes = CelGetFrame((const BYTE*)pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	CelBlitTrnTbl(pDecodeTo, pRLEBytes, nDataSize, pCelBuff->ciWidth, tbl);
}

/**
 * @brief Blit CEL sprite, and apply a given lighting/trn, to the given buffer at the given coordinates
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 * @param pCelBuff Cel data
 * @param nCel CEL frame number
 * @param nWidth Width of sprite
 */
void CelClippedDraw(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);

	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize);
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

DEVILUTION_END_NAMESPACE
