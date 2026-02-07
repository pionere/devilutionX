/**
 * @file engine.h
 *
 *  of basic engine helper functions:
 * - Angle calculation
 * - RNG
 * - Memory allocation
 * - File loading
 * - Video playback
 */
#ifndef __ENGINE_H__
#define __ENGINE_H__

DEVILUTION_BEGIN_NAMESPACE

/* Calculate direction (DIR_) from (x1;y1) to (x2;y2) */
int GetDirection(int x1, int y1, int x2, int y2);
/* Set the current RNG seed */
void SetRndSeed(int32_t s);
/* Retrieve the current RNG seed */
int32_t GetRndSeed();
/* Get the next RNG seed */
int32_t NextRndSeed();
/* Retrieve the next pseudo-random number in the range of 0 <= x < v, where v is a non-negative (32bit) integer. The result is zero if v is negative. */
int random_(BYTE idx, int v);
/* Retrieve the next pseudo-random number in the range of 0 <= x < v, where v is a positive integer and less than or equal to 0x7FFF. */
int random_low(BYTE idx, int v);
/**
 * @brief Multithreaded safe malloc
 * @param dwBytes Byte size to allocate
 */
BYTE* DiabloAllocPtr(size_t dwBytes);
/**
 * @brief Multithreaded safe memfree
 * @param p Memory pointer to free
 */
void mem_free_dbg(void* p);
#define MemFreeDbg(p)       \
	{                       \
		void* p__p;         \
		p__p = p;           \
		p = NULL;           \
		mem_free_dbg(p__p); \
	}
#define MemFreeConst(p)     \
	{                       \
		void* p__p;         \
		p__p = (void*)p;    \
		p = NULL;           \
		mem_free_dbg(p__p); \
	}
#define MemFreeTxtFile(p)      \
	{                          \
		char** p__p;           \
		p__p = p;              \
		p = NULL;              \
		mem_free_dbg(p__p[0]); \
		mem_free_dbg(p__p);    \
	}
/**
 * @brief Load an asset in to a buffer
 * @param name path/name of the asset
 * @param pdwFileLen Will be set to the size of the asset if non-NULL
 * @return Buffer with content of the asset
 */
BYTE* LoadFileInMem(const char* name, size_t* pdwFileLen = NULL);
/**
 * @brief Load an asset in to the given buffer
 * @param name path/name of the asset
 * @param p Target buffer
 */
void LoadFileWithMem(const char* name, BYTE* p);
/*
 * @brief Load a text-asset with line-breaks
 * @param name path/name of the asset
 * @param lines number of lines in the text-asset
 * @return address of the content in memory
 */
char** LoadTxtFile(const char* name, int lines);

void PlayInGameMovie(const char* pszMovie);
/* Retrieve the next pseudo-random number in the range of minVal <= x <= maxVal, where (maxVal - minVal) is a non-negative (32bit) integer. The result is minVal if (maxVal - minVal) is negative. */
inline int RandRange(int minVal, int maxVal)
{
	return minVal + random_(0, maxVal - minVal + 1);
}
/* Retrieve the next pseudo-random number in the range of minVal <= x <= maxVal, where (maxVal - minVal) is a non-negative integer and less than 0x7FFF. */
inline int RandRangeLow(int minVal, int maxVal)
{
	return minVal + random_low(0, maxVal - minVal + 1);
}

/*
 * Copy string from src to dest.
 * The NULL terminated content of src is copied to dest.
 */
template <size_t N1, size_t N2>
inline void copy_str(char (&dest)[N1], const char (&src)[N2])
{
	static_assert(N1 >= N2, "String does not fit the destination.");
	DISABLE_WARNING(deprecated-declarations, deprecated-declarations, 4996)
	strcpy(dest, src);
	ENABLE_WARNING(deprecated-declarations, deprecated-declarations, 4996)
}

/*
 * Copy constant string from src to dest.
 * The whole (padded) length of the src array is copied.
 */
template <size_t N1, size_t N2>
inline void copy_cstr(char (&dest)[N1], const char (&src)[N2])
{
	static_assert(N1 >= N2, "String does not fit the destination.");
	constexpr size_t src_len = ((N2 + sizeof(int) - 1) / sizeof(int)) * sizeof(int);
	constexpr size_t len = N1 >= src_len ? src_len : N1;
	memcpy(dest, src, len);
}

/*
 * Copy POD (Plain-Old-Data) from source to dest.
 */
template <class T>
inline void copy_pod(T& dest, T& src)
{
	memcpy(&dest, &src, sizeof(T));
}

/*
 * Copy POD (Plain-Old-Data) from constant source to dest.
 */
template <class T>
inline void copy_pod(T& dest, const T& src)
{
	memcpy(&dest, &src, sizeof(T));
}

/*
 * Concatenate strings using the provided format with the help of pos as a cursor.
 */
template <int N>
inline void cat_str(char (&dest)[N], int& pos, const char* fmt, ...)
{
	int n, res;
	va_list va;

	va_start(va, fmt);

	n = N - pos;
	res = vsnprintf(&dest[pos], n, fmt, va);
	pos += (res <= n - 1) ? res : n - 1;

	va_end(va);
}

/*
 * Concatenate constant string to the dest with the help of pos as a cursor.
 */
template <int N1, int N2>
inline void cat_cstr(char (&dest)[N1], int& pos, const char (&src)[N2])
{
	int n;
	n = N1 - pos;
	if (n >= N2) {
		memcpy(&dest[pos], src, N2);
		pos += N2 - 1;
	} else {
		n--;
		memcpy(&dest[pos], src, n);
		pos += n;
		dest[pos] = '\0';
	}
}

/*
 * Helper function to simplify 'sizeof(list) / sizeof(list[0])' expressions.
 */
template <class T, int N>
inline constexpr int lengthof(T (&array)[N])
{
	return N;
}

typedef struct CCritSect {
	SDL_mutex* m_critsect;

	CCritSect();
	~CCritSect();
	void Enter();
	void Leave();
} CCritSect;

DEVILUTION_END_NAMESPACE

#endif /* __ENGINE_H__ */
