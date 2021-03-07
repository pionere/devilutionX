/**
 * @file engine.h
 *
 *  of basic engine helper functions:
 * - Sprite blitting
 * - Drawing
 * - Angle calculation
 * - RNG
 * - Memory allocation
 * - File loading
 * - Video playback
 */
#ifndef __ENGINE_H__
#define __ENGINE_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

inline BYTE *CelGetFrameStart(BYTE *pCelBuff, int nCel)
{
	DWORD *pFrameTable;

	pFrameTable = (DWORD *)pCelBuff;

	return pCelBuff + SwapLE32(pFrameTable[nCel]);
}

#define LOAD_LE32(b) (((DWORD)(b)[3] << 24) | ((DWORD)(b)[2] << 16) | ((DWORD)(b)[1] << 8) | (DWORD)(b)[0])
inline BYTE *CelGetFrame(BYTE *pCelBuff, int nCel, int *nDataSize)
{
	DWORD nCellStart;

	nCellStart = LOAD_LE32(&pCelBuff[nCel * 4]);
	*nDataSize = LOAD_LE32(&pCelBuff[(nCel + 1) * 4]) - nCellStart;
	return pCelBuff + nCellStart;
}

inline BYTE *CelGetFrameClipped(BYTE *pCelBuff, int nCel, int *nDataSize)
{
	DWORD nDataStart;
	BYTE *pRLEBytes = CelGetFrame(pCelBuff, nCel, nDataSize);

	nDataStart = pRLEBytes[1] << 8 | pRLEBytes[0];
	*nDataSize -= nDataStart;

	return pRLEBytes + nDataStart;
}

void CelDraw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelClippedDraw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelDrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, BYTE *tbl);
void CelClippedDrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelClippedDrawLightTrans(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelDrawLightRed(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelClippedDrawSafe(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelDrawLightRedSafe(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void CelDrawOutline(BYTE col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void ENG_set_pixel(int sx, int sy, BYTE col);
#ifdef _DEBUG
void engine_debug_trap(bool show_cursor);
void engine_draw_pixel(int sx, int sy);
#endif
void DrawLine(int x0, int y0, int x1, int y1, BYTE col);
int GetDirection(int x1, int y1, int x2, int y2);
void SetRndSeed(int s);
int GetRndSeed();
int random_(BYTE idx, int v);
BYTE *DiabloAllocPtr(DWORD dwBytes);
void mem_free_dbg(void *p);
BYTE *LoadFileInMem(const char *pszName, DWORD *pdwFileLen);
DWORD LoadFileWithMem(const char *pszName, BYTE *p);
void Cl2ApplyTrans(BYTE *p, BYTE *ttbl, int nCel);
void Cl2Draw(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void Cl2DrawOutline(BYTE col, int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);
void Cl2DrawLightTbl(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth, char light);
void Cl2DrawLight(int sx, int sy, BYTE *pCelBuff, int nCel, int nWidth);

BYTE* CelMerge(BYTE* celA, int nDataSizeA, BYTE* celB, int nDataSizeB);

void PlayInGameMovie(const char *pszMovie);

inline int RandRange(int minVal, int maxVal)
{
	return minVal + random_(0, maxVal - minVal + 1);
}

#ifdef __cplusplus
}
#endif

// BUGFIX: TODO DISABLE/ENABLE_WARNING macros are not tested for GNUC/clang
#if defined(_MSC_VER)
#define DIAG_PRAGMA(x) __pragma(warning(x))
#define DISABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) DIAG_PRAGMA(push) DIAG_PRAGMA(disable:##msvc_errorcode)
#define ENABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) DIAG_PRAGMA(pop)
//#define DISABLE_WARNING(gcc_unused,clang_unused,msvc_errorcode) __pragma(warning(suppress: msvc_errorcode))
//#define ENABLE_WARNING(gcc_unused,clang_unused,msvc_unused) ((void)0)
#else
#define DIAG_STR(s) #s
#define DIAG_JOINSTR(x,y) DIAG_STR(x ## y)
#define DIAG_PRAGMA(compiler,x) _Pragma(compiler diagnostic x)
#if defined(__clang__)
# define DISABLE_WARNING(gcc_unused,clang_option,msvc_unused) DIAG_PRAGMA(clang,push) DIAG_PRAGMA(clang,ignored DIAG_JOINSTR(-W,clang_option))
# define ENABLE_WARNING(gcc_unused,clang_option,msvc_unused) DIAG_PRAGMA(clang,pop)
#elif defined(__GNUC__)
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
# define DISABLE_WARNING(gcc_option,clang_unused,msvc_unused) DIAG_PRAGMA(GCC,push) DIAG_PRAGMA(GCC,ignored DIAG_JOINSTR(-W,gcc_option))
# define ENABLE_WARNING(gcc_option,clang_unused,msvc_unused) DIAG_PRAGMA(GCC,pop)
#else
# define DISABLE_WARNING(gcc_option,clang_unused,msvc_unused) DIAG_PRAGMA(GCC,ignored DIAG_JOINSTR(-W,gcc_option))
# define ENABLE_WARNING(gcc_option,clang_option,msvc_unused) DIAG_PRAGMA(GCC,warning DIAG_JOINSTR(-W,gcc_option))
#endif
#else
#define DISABLE_WARNING(gcc_unused,clang_unused,msvc_unused) ;
#define ENABLE_WARNING(gcc_unused,clang_unused,msvc_unused) ;
#endif
#endif
/*
 * Copy string from src to dest.
 * The NULL terminated content of src is copied to dest.
 */
template<DWORD N1, DWORD N2>
inline void copy_str(char (&dest)[N1], char (&src)[N2])
{
	static_assert(N1 >= N2, "String does not fit the destination.");
	DISABLE_WARNING(gcc_option, clang_option, 4996)
	strcpy(dest, src);
	ENABLE_WARNING(gcc_option, clang_option, 4996)
}

/*
 * Copy constant string from src to dest.
 * The whole (padded) length of the src array is copied.
 */
template<DWORD N1, DWORD N2>
inline void copy_cstr(char (&dest)[N1], const char (&src)[N2])
{
	static_assert(N1 >= N2, "String does not fit the destination.");
	memcpy(dest, src, std::min(N1, ((N2 + sizeof(int) - 1) / sizeof(int)) * sizeof(int)));
}

/*
 * Copy POD (Plain-Old-Data) from source to dest.
 */
template<class T>
inline void copy_pod(T &dest, T &src)
{
	memcpy(&dest, &src, sizeof(T));
}

/*
 * Copy POD (Plain-Old-Data) from constant source to dest.
 */
template<class T>
inline void copy_pod(T &dest, const T &src)
{
	memcpy(&dest, &src, sizeof(T));
}

/*
 * Concatenate strings using the provided format with the help of pos as a cursor.
 */
template<int N>
inline void cat_str(char (&dest)[N], int &pos, const char* fmt, ...)
{
	int n;
	va_list va;

	va_start(va, fmt);
	
	n = N - pos;
	pos += std::min(vsnprintf(&dest[pos], n, fmt, va), n - 1);

	va_end(va);
}

/*
 * Concatenate constant string to the dest with the help of pos as a cursor.
 */
template<int N1, int N2>
inline void cat_cstr(char (&dest)[N1], int &pos, const char (&src)[N2])
{
	int n;
	n = N1 - pos;
	if (n >= N2) {
		memcpy(&dest[pos], src, N2);
		pos += N2 - 1;
	}
	else {
		n--;
		memcpy(&dest[pos], src, n);
		pos += n;
		dest[pos] = '\0';
	}
}

/*
 * Helper function to simplify 'sizeof(list) / sizeof(list[0])' expressions.
 */
template<class T, int N>
inline constexpr int lengthof(T (&array)[N])
{
	return N;
}

DEVILUTION_END_NAMESPACE

#endif /* __ENGINE_H__ */
