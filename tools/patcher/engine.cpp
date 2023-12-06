/**
 * @file engine.cpp
 *
 * Implementation of basic engine helper functions:
 * - Sprite blitting
 * - Drawing
 * - Angle calculation
 * - RNG
 * - Memory allocation
 * - File loading
 * - Video playback
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#if DEBUG_MODE
/** Number of times the current seed has been fetched */
int SeedCount;
#endif
/** Current game seed */
int32_t sglGameSeed;
#if __cplusplus <= 199711L
static CCritSect sgMemCrit;
#endif

/**
 * @brief Multithreaded safe malloc
 * @param dwBytes Byte size to allocate
 */
BYTE* DiabloAllocPtr(size_t dwBytes)
{
	BYTE* buf;
#if __cplusplus <= 199711L
	sgMemCrit.Enter();
	buf = (BYTE*)malloc(dwBytes);
	sgMemCrit.Leave();
#else
	buf = (BYTE*)malloc(dwBytes);
#endif

	if (buf == NULL)
		app_fatal("Out of memory");

	return buf;
}

/**
 * @brief Multithreaded safe memfree
 * @param p Memory pointer to free
 */
void mem_free_dbg(void* p)
{
	if (p != NULL) {
#if __cplusplus <= 199711L
		sgMemCrit.Enter();
		free(p);
		sgMemCrit.Leave();
#else
		free(p);
#endif
	}
}

/**
 * @brief Load a file in to a buffer
 * @param pszName Path of file
 * @param pdwFileLen Will be set to file size if non-NULL
 * @return Buffer with content of file
 */
BYTE* LoadFileInMem(const char* pszName, size_t* pdwFileLen)
{
	HANDLE file;
	BYTE* buf = NULL;
	size_t fileLen;

	file = SFileOpenFile(pszName);
	fileLen = SFileGetFileSize(file);

	if (pdwFileLen != NULL)
		*pdwFileLen = fileLen;

	if (fileLen != 0) {
		buf = (BYTE*)DiabloAllocPtr(fileLen);
		SFileReadFile(file, buf, fileLen);
	}

	SFileCloseFile(file);

	return buf;
}

/**
 * @brief Load a file in to the given buffer
 * @param pszName Path of file
 * @param p Target buffer
 */
void LoadFileWithMem(const char* pszName, BYTE* p)
{
	DWORD dwFileLen;
	HANDLE hsFile;

	assert(pszName != NULL);
	/*if (p == NULL) {
		app_fatal("LoadFileWithMem(NULL):\n%s", pszName);
	}*/

	hsFile = SFileOpenFile(pszName);

	dwFileLen = SFileGetFileSize(hsFile);
	if (dwFileLen != 0) {
		SFileReadFile(hsFile, p, dwFileLen);
	}

	SFileCloseFile(hsFile);
}

char** LoadTxtFile(const char* name, int lines)
{
	BYTE* textFile = LoadFileInMem(name, NULL);
	char** textLines = (char**)DiabloAllocPtr(sizeof(char*) * lines);

	for (int i = 0; i < lines; i++) {
		textLines[i] = (char*)textFile;

		while (*textFile != '\n') {
			assert(*textFile != '\0');
			textFile++;
		}
		*textFile = '\0';
		textFile++;
	}

	return textLines;
}

#define LOAD_LE32(b) (SwapLE32(*((DWORD*)(b))))
BYTE* CelMerge(BYTE* celA, size_t nDataSizeA, BYTE* celB, size_t nDataSizeB)
{
	size_t nDataSize;
	DWORD i, nCelA, nCelB, cData, nData;
	BYTE *cel, *pBuf;
	DWORD* pHead;

	nDataSize = nDataSizeA + nDataSizeB - 4 * 2;
	cel = DiabloAllocPtr(nDataSize);
	*(DWORD*)cel = 0;
	pBuf = cel;
	nCelA = LOAD_LE32(celA);
	nCelB = LOAD_LE32(celB);
	pHead = (DWORD*)pBuf;
	pHead++;
	pBuf += 4 * (nCelA + nCelB + 2);

	nData = LOAD_LE32(celA + 4);
	for (i = 1; i <= nCelA; i++) {
		cData = nData;
		nData = LOAD_LE32(celA + 4 * (i + 1));
		*pHead = SwapLE32(pBuf - cel);
		memcpy(pBuf, &celA[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	nData = LOAD_LE32(celB + 4);
	for (i = 1; i <= nCelB; i++) {
		cData = nData;
		nData = LOAD_LE32(celB + 4 * (i + 1));
		*pHead = SwapLE32(pBuf - cel);
		memcpy(pBuf, &celB[cData], nData - cData);
		pBuf += nData - cData;
		++*cel;
		pHead++;
	}

	*pHead = SwapLE32(pBuf - cel);
	// assert(*pHead == nDataSize);
	return cel;
}

DEVILUTION_END_NAMESPACE
