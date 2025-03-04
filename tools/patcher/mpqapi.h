/**
 * @file mpqapi.h
 *
 * Interface of functions for creating and editing MPQ files.
 */
#ifndef __MPQAPI_H__
#define __MPQAPI_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void mpqapi_remove_entry(const char* pszName);
bool mpqapi_write_entry(const char* pszName, BYTE* pbData, DWORD dwLen);
void mpqapi_rename_entry(char* pszOld, char* pszNew);
bool mpqapi_has_entry(const char* pszName);
bool OpenMPQ(const char* pszArchive);
bool CreateMPQ(const char* pszArchive, int hashCount, int blockCount);
void mpqapi_flush_and_close(bool bFree);
void mpqapi_close();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MPQAPI_H__ */
