/**
 * @file cl2_render.cpp
 *
 * CL2 rendering.
 */
#include "cl2_render.h"

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE
bool doDebugg = false;

static void LogErrorFF(const char* msg, ...)
{
	char tmp[256];

	const char* paths[2] = { GetBasePath(), GetPrefPath() };
	FILE* f0 = NULL;
	for (int i = 0; f0 == NULL && i < lengthof(paths); i++) {
		std::string filepath = paths[i];
		filepath += "logdebug0.txt";
		f0 = std::fopen(filepath.c_str(), "a+");
	}

	va_list va;

	va_start(va, msg);

	vsnprintf(tmp, sizeof(tmp), msg, va);

	va_end(va);

	fputs(tmp, f0);

	fputc('\n', f0);

	fclose(f0);
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
if (doDebugg) {
	LogErrorFF("Cl2Blit fill w%d c%d", width, fill);
}
						i -= width;
						while (width != 0) {
							*dst = fill;
							dst++;
							width--;
						}
						continue;
				} else {
if (doDebugg) {
	LogErrorFF("Cl2Blit colors w%d", width);
}
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
if (doDebugg) {
	LogErrorFF("Cl2Blit last transp w%d", width);
}
					dst += width;
					i -= width;
					break;
				} else {
if (doDebugg) {
	LogErrorFF("Cl2Blit i transp w%d", i);
}
					dst += i;
					width -= i;
					i = nWidth;
					dst -= BUFFER_WIDTH + nWidth;
				}
			}
		}
if (doDebugg) {
	LogErrorFF("Cl2Blit new line");
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
 */
void Cl2Draw(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth)
{
	int nDataSize;
	const BYTE* pRLEBytes;
	BYTE* pDecodeTo;

	assert(gpBuffer != NULL);
	assert(pCelBuff != NULL);
	assert(nCel > 0);
if (doDebugg) {
	LogErrorFF("Cl2Draw 0 to%d:%d", sx, sy);
}
	pRLEBytes = CelGetFrameClipped(pCelBuff, nCel, &nDataSize, &sy);
if (doDebugg) {
	LogErrorFF("Cl2Draw 1 to%d:%d size:%d offset%d in buffer %d", sx, sy, nDataSize, (size_t)pRLEBytes - (size_t)pCelBuff, BUFFERXY(sx, sy));
}
	pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];

	Cl2Blit(pDecodeTo, pRLEBytes, nDataSize, nWidth);
}

DEVILUTION_END_NAMESPACE
