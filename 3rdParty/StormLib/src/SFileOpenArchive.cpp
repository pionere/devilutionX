/*****************************************************************************/
/* SFileOpenArchive.cpp                       Copyright Ladislav Zezula 1999 */
/*                                                                           */
/* Author : Ladislav Zezula                                                  */
/* E-mail : ladik@zezula.net                                                 */
/* WWW    : www.zezula.net                                                   */
/*---------------------------------------------------------------------------*/
/* Implementation of archive functions                                       */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* xx.xx.xx  1.00  Lad  Created                                              */
/* 19.11.03  1.01  Dan  Big endian handling                                  */
/*****************************************************************************/

#define __STORMLIB_SELF__
#include "StormLib.h"
#include "StormCommon.h"

#define HEADER_SEARCH_BUFFER_SIZE   0x1000

//-----------------------------------------------------------------------------
// Local functions
#ifdef FULL
static MTYPE CheckMapType(LPCTSTR szFileName, LPBYTE pbHeaderBuffer, size_t cbHeaderBuffer)
{
    LPDWORD HeaderInt32 = (LPDWORD)pbHeaderBuffer;
    LPCTSTR szExtension;

    // Don't do any checks if there is not at least 16 bytes
    if(cbHeaderBuffer > 0x10)
    {
        DWORD DwordValue0 = BSWAP_INT32_UNSIGNED(HeaderInt32[0]);
        DWORD DwordValue1 = BSWAP_INT32_UNSIGNED(HeaderInt32[1]);
        DWORD DwordValue2 = BSWAP_INT32_UNSIGNED(HeaderInt32[2]);
        DWORD DwordValue3 = BSWAP_INT32_UNSIGNED(HeaderInt32[3]);

        // Check maps by extension (Starcraft, Starcraft II). We must do this before
        // checking actual data, because the "NP_Protect" protector places
        // fake Warcraft III header into the Starcraft II maps
        if((szExtension = _tcsrchr(szFileName, _T('.'))) != NULL)
        {
            // Check for Starcraft II maps by extension
            if(!_tcsicmp(szExtension, _T(".s2ma")) || !_tcsicmp(szExtension, _T(".SC2Map")) || !_tcsicmp(szExtension, _T(".SC2Mod")))
            {
                return MapTypeStarcraft2;
            }

            // Check for Starcraft I maps by extension
            if(!_tcsicmp(szExtension, _T(".scm")) || !_tcsicmp(szExtension, _T(".scx")))
            {
                return MapTypeStarcraft;
            }
        }

        // Test for AVI files (Warcraft III cinematics) - 'RIFF', 'AVI ' or 'LIST'
        if(DwordValue0 == 0x46464952 && DwordValue2 == 0x20495641 && DwordValue3 == 0x5453494C)
            return MapTypeAviFile;

        // Check for Warcraft III maps
        if(DwordValue0 == 0x57334D48 && DwordValue1 == 0x00000000)
            return MapTypeWarcraft3;
    }

    // MIX files are DLL files that contain MPQ in overlay.
    // Only Warcraft III is able to load them, so we consider them Warcraft III maps
    if(cbHeaderBuffer > 0x200 && pbHeaderBuffer[0] == 'M' && pbHeaderBuffer[1] == 'Z')
    {
        // Check the value of IMAGE_DOS_HEADER::e_lfanew at offset 0x3C
        if(0 < HeaderInt32[0x0F] && HeaderInt32[0x0F] < 0x10000)
            return MapTypeWarcraft3;
    }

    // No special map type recognized
    return MapTypeNotRecognized;
}

static TMPQUserData * IsValidMpqUserData(ULONGLONG ByteOffset, ULONGLONG FileSize, void * pvUserData)
{
    TMPQUserData * pUserData;

    // BSWAP the source data and copy them to our buffer
    BSWAP_ARRAY32_UNSIGNED(pvUserData, sizeof(TMPQUserData));
    pUserData = (TMPQUserData *)pvUserData;

    // Check the sizes
    if(pUserData->cbUserDataHeader <= pUserData->cbUserDataSize && pUserData->cbUserDataSize <= pUserData->dwHeaderOffs)
    {
        // Move to the position given by the userdata
        ByteOffset += pUserData->dwHeaderOffs;

        // The MPQ header should be within range of the file size
        if((ByteOffset + MPQ_HEADER_SIZE_V1) < FileSize)
        {
            // Note: We should verify if there is the MPQ header.
            // However, the header could be at any position below that
            // that is multiplier of 0x200
            return (TMPQUserData *)pvUserData;
        }
    }

    return NULL;
}

// This function gets the right positions of the hash table and the block table.
static DWORD VerifyMpqTablePositions(TMPQArchive * ha, ULONGLONG FileSize)
{
    TMPQHeader * pHeader = &ha->pHeader;
#ifdef FULL
    ULONGLONG ByteOffset;
#else
    FILESIZE_T ByteOffset;
#endif
    //bool bMalformed = (ha->dwFlags & MPQ_FLAG_MALFORMED) ? true : false;
#ifdef FULL
    // Check the begin of HET table
    if(pHeader->HetTablePos64)
    {
        ByteOffset = ha->MpqPos + pHeader->HetTablePos64;
        if(ByteOffset > FileSize)
            return ERROR_BAD_FORMAT;
    }

    // Check the begin of BET table
    if(pHeader->BetTablePos64)
    {
        ByteOffset = ha->MpqPos + pHeader->BetTablePos64;
        if(ByteOffset > FileSize)
            return ERROR_BAD_FORMAT;
    }

    // Check the begin of hash table
    if(pHeader->wHashTablePosHi || pHeader->dwHashTablePos)
    {
        ByteOffset = FileOffsetFromMpqOffset(ha, MAKE_OFFSET64(pHeader->wHashTablePosHi, pHeader->dwHashTablePos));
        if(ByteOffset > FileSize)
            return ERROR_BAD_FORMAT;
    }

    // Check the begin of block table
    if(pHeader->wBlockTablePosHi || pHeader->dwBlockTablePos)
    {
        ByteOffset = FileOffsetFromMpqOffset(ha, MAKE_OFFSET64(pHeader->wBlockTablePosHi, pHeader->dwBlockTablePos));
        if(ByteOffset > FileSize)
            return ERROR_BAD_FORMAT;
    }

    // Check the begin of hi-block table
    //if(pHeader->HiBlockTablePos64 != 0)
    //{
    //    ByteOffset = ha->MpqPos + pHeader->HiBlockTablePos64;
    //    if(ByteOffset > FileSize)
    //        return ERROR_BAD_FORMAT;
    //}
#else
    // Check the begin of hash table
    if(pHeader->dwHashTablePos)
    {
        ByteOffset = FileOffsetFromMpqOffset(pHeader->dwHashTablePos);
        if(ByteOffset > FileSize)
            return ERROR_BAD_FORMAT;
    }

    // Check the begin of block table
    if(pHeader->dwBlockTablePos)
    {
        ByteOffset = FileOffsetFromMpqOffset(pHeader->dwBlockTablePos);
        if(ByteOffset > FileSize)
            return ERROR_BAD_FORMAT;
    }
#endif
    // All OK.
    return ERROR_SUCCESS;
}
#endif
//-----------------------------------------------------------------------------
// Support for alternate markers. Call before opening an archive
#ifdef FULL
#define SFILE_MARKERS_MIN_SIZE   (sizeof(DWORD) + sizeof(DWORD) + sizeof(const char *) + sizeof(const char *))

bool WINAPI SFileSetArchiveMarkers(PSFILE_MARKERS pMarkers)
{
    // Check structure minimum size
    if(pMarkers == NULL || pMarkers->dwSize < SFILE_MARKERS_MIN_SIZE)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    // Make sure that the MPQ cryptography is initialized at this time
    InitializeMpqCryptography();

    // Remember the marker for MPQ header
    if(pMarkers->dwSignature != 0)
        g_dwMpqSignature = pMarkers->dwSignature;

    // Remember the encryption key for hash table
    if(pMarkers->szHashTableKey != NULL)
        g_dwHashTableKey = HashString(pMarkers->szHashTableKey, MPQ_HASH_FILE_KEY);

    // Remember the encryption key for block table
    if(pMarkers->szBlockTableKey != NULL)
        g_dwBlockTableKey = HashString(pMarkers->szBlockTableKey, MPQ_HASH_FILE_KEY);

    // Succeeded
    return true;
}

//-----------------------------------------------------------------------------
// SFileGetLocale and SFileSetLocale
// Set the locale for all newly opened files

/*LCID WINAPI SFileGetLocale()
{
    return g_lcFileLocale;
}

LCID WINAPI SFileSetLocale(LCID lcNewLocale)
{
    g_lcFileLocale = lcNewLocale;
    return g_lcFileLocale;
}*/
#endif
//-----------------------------------------------------------------------------
// SFileOpenArchive
//
//   szFileName - MPQ archive file name to open
////   dwPriority - When SFileOpenFileEx called, this contains the search priority for searched archives
//   dwFlags    - See MPQ_OPEN_XXX in StormLib.h
//   phMpq      - Pointer to store open archive handle

HANDLE WINAPI SFileOpenArchive(
    const TCHAR * szMpqName,
    DWORD dwFlags)
{
#ifdef FULL
    TMPQUserData * pUserData;
    TFileStream * pStream = NULL;       // Open file stream
#endif
    TMPQArchive * ha = NULL;            // Archive handle
#ifdef FULL
    TFileEntry * pFileEntry;
    ULONGLONG FileSize = 0;             // Size of the file
    LPBYTE pbHeaderBuffer = NULL;       // Buffer for searching MPQ header
    DWORD dwStreamFlags = (dwFlags & STREAM_FLAGS_MASK);
    MTYPE MapType = MapTypeNotChecked;
#else
    DWORD dwStreamFlags = dwFlags;
#endif
    DWORD dwErrCode = ERROR_SUCCESS;
#ifdef FULL
    // Verify the parameters
    if(szMpqName == NULL || *szMpqName == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    // One time initialization of MPQ cryptography
    InitializeMpqCryptography();

    // If not forcing MPQ v 1.0, also use file bitmap
    dwStreamFlags |= (dwFlags & MPQ_OPEN_FORCE_MPQ_V1) ? 0 : STREAM_FLAG_USE_BITMAP;

    // Open the MPQ archive file
    pStream = FileStream_OpenFile(szMpqName, dwStreamFlags);
    if(pStream == NULL)
        return NULL;

    // Check the file size. There must be at least 0x20 bytes
    if(dwErrCode == ERROR_SUCCESS)
    {
        FileStream_GetSize(pStream, &FileSize);
        if(FileSize < MPQ_HEADER_SIZE_V1)
            dwErrCode = ERROR_BAD_FORMAT;
    }
#endif
    // Allocate the MPQhandle
    if(dwErrCode == ERROR_SUCCESS)
    {
        if((ha = STORM_ALLOC(TMPQArchive, 1)) == NULL)
#ifdef FULL
            dwErrCode = ERROR_NOT_ENOUGH_MEMORY;
#else
            dwErrCode = ERROR_SUCCESS + 1;
#endif
    }

    // Allocate buffer for searching MPQ header
    if(dwErrCode == ERROR_SUCCESS)
    {
#ifdef FULL
        pbHeaderBuffer = STORM_ALLOC(BYTE, HEADER_SEARCH_BUFFER_SIZE);
        if(pbHeaderBuffer == NULL)
            dwErrCode = ERROR_NOT_ENOUGH_MEMORY;
#else
        dwErrCode = FileStream_OpenFile(&ha->pStream, szMpqName, dwStreamFlags);
        if (dwErrCode != ERROR_SUCCESS) {
            STORM_FREE(ha);
            ha = NULL;
        }
#endif
    }
    // Find the position of MPQ header
    if(dwErrCode == ERROR_SUCCESS)
    {
#ifdef FULL
        ULONGLONG ByteOffset = 0;
        ULONGLONG EndOfSearch = FileSize;
        DWORD dwStrmFlags = 0;
        DWORD dwHeaderSize;
        DWORD dwHeaderID;
        bool bSearchComplete = false;

        memset(ha, 0, sizeof(TMPQArchive));

        ha->dwValidFileFlags = MPQ_FILE_VALID_FLAGS;
        ha->pfnHashString = HashStringSlash;
#else
        FILESIZE_T ByteOffset = 0;
        ha->pBlockTable = NULL;
        ha->pHashTable = NULL;
#endif
#ifdef FULL
        ha->pStream = pStream;
        pStream = NULL;

        // Set the archive read only if the stream is read-only
        FileStream_GetFlags(ha->pStream, &dwStrmFlags);
        ha->dwFlags |= (dwStrmFlags & STREAM_FLAG_READ_ONLY) ? MPQ_FLAG_READ_ONLY : 0;

        // Also remember if we shall check sector CRCs when reading file
        ha->dwFlags |= (dwFlags & MPQ_OPEN_CHECK_SECTOR_CRC) ? MPQ_FLAG_CHECK_SECTOR_CRC : 0;

        // Also remember if this MPQ is a patch
        ha->dwFlags |= (dwFlags & MPQ_OPEN_PATCH) ? MPQ_FLAG_PATCH : 0;

        // Limit the header searching to about 130 MB of data
        if(EndOfSearch > 0x08000000)
            EndOfSearch = 0x08000000;
        if(FileSize < HEADER_SEARCH_BUFFER_SIZE)
            memset(pbHeaderBuffer, 0, HEADER_SEARCH_BUFFER_SIZE);

        // Find the offset of MPQ header within the file
        while(bSearchComplete == false && ByteOffset < EndOfSearch)
        {
            // Always read at least 0x1000 bytes for performance.
            // This is what Storm.dll (2002) does.
            DWORD dwBytesAvailable = HEADER_SEARCH_BUFFER_SIZE;

            // Cut the bytes available, if needed
            if((FileSize - ByteOffset) < HEADER_SEARCH_BUFFER_SIZE)
                dwBytesAvailable = (DWORD)(FileSize - ByteOffset);

            // Read the eventual MPQ header
            if(!FileStream_Read(ha->pStream, &ByteOffset, pbHeaderBuffer, dwBytesAvailable))
            {
                dwErrCode = GetLastError();
                break;
            }

            // Check whether the file is AVI file or a Warcraft III/Starcraft II map
            if(MapType == MapTypeNotChecked)
            {
                // Do nothing if the file is an AVI file
                if((MapType = CheckMapType(szMpqName, pbHeaderBuffer, dwBytesAvailable)) == MapTypeAviFile)
                {
                    dwErrCode = ERROR_AVI_FILE;
                    break;
                }
            }

            // Search the header buffer
            for(DWORD dwInBufferOffset = 0; dwInBufferOffset < dwBytesAvailable; dwInBufferOffset += 0x200)
            {
                // Copy the data from the potential header buffer to the MPQ header
                memcpy(ha->HeaderData, pbHeaderBuffer + dwInBufferOffset, sizeof(ha->HeaderData));

                // If there is the MPQ user data, process it
                // Note that Warcraft III does not check for user data, which is abused by many map protectors
                dwHeaderID = BSWAP_INT32_UNSIGNED(ha->HeaderData[0]);
                if(MapType != MapTypeWarcraft3 && (dwFlags & MPQ_OPEN_FORCE_MPQ_V1) == 0)
                {
                    if(ha->pUserData == NULL && dwHeaderID == ID_MPQ_USERDATA)
                    {
                        // Verify if this looks like a valid user data
                        pUserData = IsValidMpqUserData(ByteOffset, FileSize, ha->HeaderData);
                        if(pUserData != NULL)
                        {
                            // Fill the user data header
                            ha->UserDataPos = ByteOffset;
                            ha->pUserData = &ha->UserData;
                            memcpy(ha->pUserData, pUserData, sizeof(TMPQUserData));

                            // Continue searching from that position
                            ByteOffset += ha->pUserData->dwHeaderOffs;
                            break;
                        }
                    }
                }

                // There must be MPQ header signature. Note that STORM.dll from Warcraft III actually
                // tests the MPQ header size. It must be at least 0x20 bytes in order to load it
                // Abused by Spazzler Map protector. Note that the size check is not present
                // in Storm.dll v 1.00, so Diablo I code would load the MPQ anyway.
                dwHeaderSize = BSWAP_INT32_UNSIGNED(ha->HeaderData[1]);
                if(dwHeaderID == g_dwMpqSignature && dwHeaderSize >= MPQ_HEADER_SIZE_V1)
                {
                    // Now convert the header to version 4
                    dwErrCode = ConvertMpqHeaderToFormat4(ha, ByteOffset, FileSize, dwFlags, MapType);
#else
                if(!FileStream_Read(&ha->pStream, ByteOffset, &ha->pHeader, sizeof(ha->pHeader)))
                {
                    dwErrCode = ERROR_SUCCESS + 1;
                } else {
                    ConvertMpqHeaderToFormat4(ha);
#endif // FULL
#ifdef FULL
                    if(dwErrCode != ERROR_FAKE_MPQ_HEADER)
                    {
                        bSearchComplete = true;
                        break;
                    }
                }

                // Check for MPK archives (Longwu Online - MPQ fork)
                if(MapType == MapTypeNotRecognized && (dwFlags & MPQ_OPEN_FORCE_MPQ_V1) == 0 && dwHeaderID == ID_MPK)
                {
                    // Now convert the MPK header to MPQ Header version 4
                    dwErrCode = ConvertMpkHeaderToFormat4(ha, FileSize, dwFlags);
                    bSearchComplete = true;
                    break;
                }

                // If searching for the MPQ header is disabled, return an error
                if(dwFlags & MPQ_OPEN_NO_HEADER_SEARCH)
                {
                    dwErrCode = ERROR_NOT_SUPPORTED;
                    bSearchComplete = true;
                    break;
                }

                // Move the pointers
                ByteOffset += 0x200;
            }
#endif // FULL
        }
#ifdef FULL
        // Did we identify one of the supported headers?
        if(dwErrCode == ERROR_SUCCESS)
        {
            // Set the user data position to the MPQ header, if none
            if(ha->pUserData == NULL)
                ha->UserDataPos = ByteOffset;

            // Set the position of the MPQ header
            ha->pHeader  = (TMPQHeader *)ha->HeaderData;

            ha->MpqPos   = ByteOffset;
            ha->FileSize = FileSize;

            // Sector size must be nonzero.
            if(ByteOffset >= FileSize || ha->pHeader->wSectorSize == 0)
                dwErrCode = ERROR_BAD_FORMAT;
        }
#else
        if(dwErrCode == ERROR_SUCCESS)
        {
            if((int)ha->pHeader.dwHashTableSize <= 0                     // required by GetFirstHashEntry ( != 0 by LoadAnyHashTable)
             || (ha->pHeader.dwHashTableSize & (ha->pHeader.dwHashTableSize - 1)) != 0)
                dwErrCode = ERROR_SUCCESS + 1;

            if(ha->pHeader.dwBlockTableSize == 0) // required by BuildFileTable
                dwErrCode = ERROR_SUCCESS + 1;
        }
#endif
    }
#ifdef FULL
    // Fix table positions according to format
    if(dwErrCode == ERROR_SUCCESS)
    {
        // Dump the header
//      DumpMpqHeader(ha->pHeader);

        // W3x Map Protectors use the fact that War3's Storm.dll ignores the MPQ user data,
        // and ignores the MPQ format version as well. The trick is to
        // fake MPQ format 2, with an improper hi-word position of hash table and block table
        // We can overcome such protectors by forcing opening the archive as MPQ v 1.0
        if(dwFlags & MPQ_OPEN_FORCE_MPQ_V1)
        {
            ha->pHeader->wFormatVersion = MPQ_FORMAT_VERSION_1;
            ha->pHeader->dwHeaderSize = MPQ_HEADER_SIZE_V1;
            ha->dwFlags |= MPQ_FLAG_READ_ONLY;
            ha->pUserData = NULL;
        }

        // Anti-overflow. If the hash table size in the header is
        // higher than 0x10000000, it would overflow in 32-bit version
        // Observed in the malformed Warcraft III maps
        // Example map: MPQ_2016_v1_ProtectedMap_TableSizeOverflow.w3x
        ha->pHeader->dwBlockTableSize = (ha->pHeader->dwBlockTableSize & BLOCK_INDEX_MASK);
        ha->pHeader->dwHashTableSize = (ha->pHeader->dwHashTableSize & BLOCK_INDEX_MASK);

        // Both MPQ_OPEN_NO_LISTFILE or MPQ_OPEN_NO_ATTRIBUTES trigger read only mode
        if(dwFlags & (MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_NO_ATTRIBUTES))
            ha->dwFlags |= MPQ_FLAG_READ_ONLY;

        // Check if the caller wants to force adding listfile
        if(dwFlags & MPQ_OPEN_FORCE_LISTFILE)
            ha->dwFlags |= MPQ_FLAG_LISTFILE_FORCE;

        // Maps from StarCraft and Warcraft III need special treatment
        switch(MapType)
        {
            case MapTypeStarcraft:
                ha->dwValidFileFlags = MPQ_FILE_VALID_FLAGS_SCX;
                ha->dwFlags |= MPQ_FLAG_STARCRAFT;
                break;

            case MapTypeWarcraft3:
                ha->dwValidFileFlags = MPQ_FILE_VALID_FLAGS_W3X;
                ha->dwFlags |= MPQ_FLAG_WAR3_MAP;
                break;
        }
        // Set the size of file sector
        ha->dwSectorSize = (0x200 << ha->pHeader->wSectorSize);

        // Verify if any of the tables doesn't start beyond the end of the file
        dwErrCode = VerifyMpqTablePositions(ha, FileSize);
    }
#endif
    // Read the hash table. Ignore the result, as hash table is no longer required
    // Read HET table. Ignore the result, as HET table is no longer required
    if(dwErrCode == ERROR_SUCCESS)
    {
        dwErrCode = LoadAnyHashTable(ha);
    }

    // Now, build the file table. It will be built by combining
    // the block table, BET table, hi-block table, (attributes) and (listfile).
    if(dwErrCode == ERROR_SUCCESS)
    {
        dwErrCode = BuildFileTable(ha);
    }

#ifdef FULL
    // Load the internal listfile and include it to the file table
    if(dwErrCode == ERROR_SUCCESS && (dwFlags & MPQ_OPEN_NO_LISTFILE) == 0)
    {
        // Quick check for (listfile)
        pFileEntry = GetFileEntryLocale(ha, LISTFILE_NAME, LANG_NEUTRAL);
        if(pFileEntry != NULL)
        {
            // Ignore result of the operation. (listfile) is optional.
            SFileAddListFile((HANDLE)ha, NULL);
            ha->dwFileFlags1 = pFileEntry->dwFlags;
        }
    }

    // Load the "(attributes)" file and merge it to the file table
    if(dwErrCode == ERROR_SUCCESS && (dwFlags & MPQ_OPEN_NO_ATTRIBUTES) == 0 && (ha->dwFlags & MPQ_FLAG_BLOCK_TABLE_CUT) == 0)
    {
        // Quick check for (attributes)
        pFileEntry = GetFileEntryLocale(ha, ATTRIBUTES_NAME, LANG_NEUTRAL);
        if(pFileEntry != NULL)
        {
            // Ignore result of the operation. (attributes) is optional.
            SAttrLoadAttributes(ha);
            ha->dwFileFlags2 = pFileEntry->dwFlags;
        }
    }

    // Remember whether the archive has weak signature. Only for MPQs format 1.0.
    if(dwErrCode == ERROR_SUCCESS)
    {
        // Quick check for (signature)
        pFileEntry = GetFileEntryLocale(ha, SIGNATURE_NAME, LANG_NEUTRAL);
        if(pFileEntry != NULL)
        {
            // Just remember that the archive is weak-signed
            assert((pFileEntry->dwFlags & MPQ_FILE_EXISTS) != 0);
            ha->dwFileFlags3 = pFileEntry->dwFlags;
        }
        // Finally, set the MPQ_FLAG_READ_ONLY if the MPQ was found malformed
        ha->dwFlags |= (ha->dwFlags & MPQ_FLAG_MALFORMED) ? MPQ_FLAG_READ_ONLY : 0;
    }
#endif
    // Cleanup and exit
    if(dwErrCode != ERROR_SUCCESS)
    {
#ifdef FULL
        FileStream_Close(pStream);
#endif
        FreeArchiveHandle(ha);
#ifdef FULL
        SetLastError(dwErrCode);
#endif
        ha = NULL;
    }
#ifdef FULL
    // Free the header buffer
    if(pbHeaderBuffer != NULL)
        STORM_FREE(pbHeaderBuffer);
#endif
    return ha;
}

#ifdef FULL
//-----------------------------------------------------------------------------
// bool WINAPI SFileSetDownloadCallback(HANDLE, SFILE_DOWNLOAD_CALLBACK, void *);
//
// Sets a callback that is called when content is downloaded from the master MPQ
//

bool WINAPI SFileSetDownloadCallback(HANDLE hMpq, SFILE_DOWNLOAD_CALLBACK DownloadCB, void * pvUserData)
{
    TMPQArchive * ha = (TMPQArchive *)hMpq;

    // Do nothing if 'hMpq' is bad parameter
    if(!IsValidMpqHandle(hMpq))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    return FileStream_SetCallback(ha->pStream, DownloadCB, pvUserData);
}

//-----------------------------------------------------------------------------
// bool SFileFlushArchive(HANDLE hMpq)
//
// Saves all dirty data into MPQ archive.
// Has similar effect like SFileCloseArchive, but the archive is not closed.
// Use on clients who keep MPQ archive open even for write operations,
// and terminating without calling SFileCloseArchive might corrupt the archive.
//

bool WINAPI SFileFlushArchive(HANDLE hMpq)
{
    TMPQArchive * ha;
    DWORD dwResultError = ERROR_SUCCESS;
    DWORD dwErrCode;

    // Do nothing if 'hMpq' is bad parameter
    if((ha = IsValidMpqHandle(hMpq)) == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    // Only if the MPQ was changed
    if(ha->dwFlags & MPQ_FLAG_CHANGED)
    {
        // Indicate that we are saving MPQ internal structures
        ha->dwFlags |= MPQ_FLAG_SAVING_TABLES;

        // Defragment the file table. This will allow us to put the internal files to the end
        DefragmentFileTable(ha);

        //
        // Create each internal file
        // Note that the (signature) file is usually before (listfile) in the file table
        //

        if(ha->dwFlags & MPQ_FLAG_SIGNATURE_NEW)
        {
            dwErrCode = SSignFileCreate(ha);
            if(dwErrCode != ERROR_SUCCESS)
                dwResultError = dwErrCode;
        }

        if(ha->dwFlags & (MPQ_FLAG_LISTFILE_NEW | MPQ_FLAG_LISTFILE_FORCE))
        {
            dwErrCode = SListFileSaveToMpq(ha);
            if(dwErrCode != ERROR_SUCCESS)
                dwResultError = dwErrCode;
        }

        if(ha->dwFlags & MPQ_FLAG_ATTRIBUTES_NEW)
        {
            dwErrCode = SAttrFileSaveToMpq(ha);
            if(dwErrCode != ERROR_SUCCESS)
                dwResultError = dwErrCode;
        }

        // Save HET table, BET table, hash table, block table, hi-block table
        if(ha->dwFlags & MPQ_FLAG_CHANGED)
        {
            // Rebuild the HET table
            if(ha->pHetTable != NULL)
                RebuildHetTable(ha);

            // Save all MPQ tables first
            dwErrCode = SaveMPQTables(ha);
            if(dwErrCode != ERROR_SUCCESS)
                dwResultError = dwErrCode;

            // If the archive has weak signature, we need to finish it
            if(ha->dwFileFlags3 != 0)
            {
                dwErrCode = SSignFileFinish(ha);
                if(dwErrCode != ERROR_SUCCESS)
                    dwResultError = dwErrCode;
            }
        }

        // We are no longer saving internal MPQ structures
        ha->dwFlags &= ~MPQ_FLAG_SAVING_TABLES;
    }

    // Return the error
    if(dwResultError != ERROR_SUCCESS)
        SetLastError(dwResultError);
    return (dwResultError == ERROR_SUCCESS);
}
#else
DWORD WINAPI SFileReadArchive(HANDLE hMpq, const char* szFileName, BYTE** dest)
{
    HANDLE file;
    BYTE* buf;
    DWORD fileLen = 0;

    if (SFileOpenFileEx(hMpq, szFileName, &file)) {
        fileLen = SFileGetFileSize(file);

        if (fileLen != 0 && dest != NULL) {
            buf = *dest;
            if (buf == NULL) {
                buf = STORM_ALLOC(BYTE, fileLen);
            }
            if (buf != NULL) {
                if (!SFileReadFile(file, buf, fileLen)) {
                    STORM_FREE(buf);
                    // buf = NULL;
                    fileLen = 0; // failure with -1 ?
                }
            } else {
                fileLen = 0; // report out-of-memory with -1 ?
            }
            *dest = buf;
        }

        SFileCloseFile(file);
    }

    return fileLen;
}

DWORD WINAPI SFileReadLocalFile(const char* szFileName, BYTE** dest)
{
    HANDLE file;
    BYTE* buf;
    DWORD fileLen = 0;

    if (SFileOpenLocalFileEx(szFileName, &file)) {
        fileLen = SFileGetFileSize(file);

        if (fileLen != 0 && dest != NULL) {
            buf = *dest;
            if (buf == NULL) {
                buf = STORM_ALLOC(BYTE, fileLen);
            }
            if (buf != NULL) {
                if (!SFileReadFile(file, buf, fileLen)) {
                    STORM_FREE(buf);
                    // buf = NULL;
                    fileLen = 0; // failure with -1 ?
                }
            } else {
                fileLen = 0; // report out-of-memory with -1 ?
            }
            *dest = buf;
        }

        SFileCloseFile(file);
    }

    return fileLen;
}

static bool WriteHeader(TMPQArchive * ha)
{
    BSWAP_TMPQHEADER(&ha->pHeader, MPQ_FORMAT_VERSION_1);

    const bool success = FileStream_Write(&ha->pStream, 0, &ha->pHeader, sizeof(ha->pHeader));
    BSWAP_TMPQHEADER(&ha->pHeader, MPQ_FORMAT_VERSION_1);
    return success;
}

static bool WriteBlockTable(TMPQArchive * ha)
{
    DWORD blockSize, key = MPQ_KEY_BLOCK_TABLE; //HashStringSlash("(block table)", MPQ_HASH_FILE_KEY);

    blockSize = ha->pHeader.dwBlockTableSize * sizeof(TMPQBlock);

    BSWAP_ARRAY32_UNSIGNED(ha->pBlockTable, blockSize / sizeof(DWORD));

    EncryptMpqBlock(ha->pBlockTable, blockSize, key);
    const bool success = FileStream_Write(&ha->pStream, ha->pHeader.dwBlockTablePos, reinterpret_cast<const char*>(ha->pBlockTable), blockSize);
    DecryptMpqBlock(ha->pBlockTable, blockSize, key);
    BSWAP_ARRAY32_UNSIGNED(ha->pBlockTable, blockSize / sizeof(DWORD));
    return success;
}

static bool WriteHashTable(TMPQArchive * ha)
{
    DWORD hashSize, key = MPQ_KEY_HASH_TABLE; //HashStringSlash("(hash table)", MPQ_HASH_FILE_KEY);

    hashSize = ha->pHeader.dwHashTableSize * sizeof(TMPQHash);

    BSWAP_ARRAY32_UNSIGNED(ha->pHashTable, hashSize / sizeof(DWORD));

    EncryptMpqBlock(ha->pHashTable, hashSize, key);
    const bool success = FileStream_Write(&ha->pStream, ha->pHeader.dwHashTablePos, reinterpret_cast<const char*>(ha->pHashTable), hashSize);
    DecryptMpqBlock(ha->pHashTable, hashSize, key);
    BSWAP_ARRAY32_UNSIGNED(ha->pHashTable, hashSize / sizeof(DWORD));
    return success;
}

static bool WriteHeaderAndTables(TMPQArchive * ha)
{
    return WriteHeader(ha) && WriteBlockTable(ha) && WriteHashTable(ha);
}

HANDLE WINAPI SFileCreateArchive(const TCHAR * szMpqName, DWORD dwHashCount, DWORD dwBlockCount)
{
    TFileStream * pStream = NULL;       // Open file stream
    TMPQArchive * ha = NULL;            // Archive handle
    DWORD dwErrCode = ERROR_SUCCESS;
    DWORD blockSize, hashSize;

    if(dwErrCode == ERROR_SUCCESS)
    {
        if((ha = STORM_ALLOC(TMPQArchive, 1)) == NULL)
            dwErrCode = ERROR_SUCCESS + 1;
    }

    if(dwErrCode == ERROR_SUCCESS)
    {
        dwErrCode = FileStream_CreateFile(&ha->pStream, szMpqName);
        if (dwErrCode != ERROR_SUCCESS) {
            STORM_FREE(ha);
            ha = NULL;
        }
    }
    if(dwErrCode == ERROR_SUCCESS)
    {
        // assert(dwHashCount != 0); required by LoadAnyHashTable
        // assert(blockCount != 0);  required by BuildFileTable
        // dwHashCount must be a power of two
        // assert((dwHashCount & (dwHashCount - 1)) == 0); // required by GetFirstHashEntry / mpqapi_add_hash_entry
        // InitDefaultMpqHeader
        // std::memset(&ha->pHeader, 0, sizeof(ha->pHeader));
        ha->pHeader.dwID = ID_MPQ;
        ha->pHeader.dwHeaderSize = MPQ_HEADER_SIZE_V1;
        ha->pHeader.wSectorSize = MPQ_SECTOR_SIZE_SHIFT_V1;
        ha->pHeader.wFormatVersion = MPQ_FORMAT_VERSION_1;
        ha->pHeader.dwHashTableSize = dwHashCount;
        ha->pHeader.dwBlockTableSize = dwBlockCount;
        ha->pHeader.dwHashTablePos = MPQ_HEADER_SIZE_V1 + dwBlockCount * sizeof(TMPQBlock);
        ha->pHeader.dwBlockTablePos = MPQ_HEADER_SIZE_V1;
        ha->pHeader.dwArchiveSize = ha->pHeader.dwHashTablePos + dwHashCount * sizeof(TMPQHash);

        blockSize = dwBlockCount * sizeof(TMPQBlock);
        ha->pBlockTable = STORM_ALLOC(TMPQBlock, dwBlockCount);
        hashSize = dwHashCount * sizeof(TMPQHash);
        ha->pHashTable = STORM_ALLOC(TMPQHash, dwHashCount);
        if (ha->pBlockTable != NULL && ha->pHashTable != NULL) {
            memset(ha->pBlockTable, 0, blockSize);
            // static_assert(HASH_ENTRY_FREE == 0xFFFFFFFF, "SFileCreateArchive initializes the hashtable with 0xFF to mark the entries as free.");
            memset(ha->pHashTable, 0xFF, hashSize);
        } else {
            dwErrCode = ERROR_SUCCESS + 1;
        }
    }
    if(dwErrCode != ERROR_SUCCESS)
    {
        FreeArchiveHandle(ha);
        ha = NULL;
    }
    return ha;
}

void   WINAPI SFileFlushAndCloseArchive(HANDLE hMpq)
{
    TMPQArchive * ha = IsValidMpqHandle(hMpq);
    if (WriteHeaderAndTables(ha)) {
        FileStream_SetSize(&ha->pStream, ha->pHeader.dwArchiveSize);
    }
    FreeArchiveHandle(ha);
}

static DWORD mpqapi_new_block(TMPQArchive * ha)
{
    TMPQBlock* pBlock;
    DWORD i, blockCount;

    pBlock = ha->pBlockTable;
    blockCount = ha->pHeader.dwBlockTableSize;
    for (i = 0; i < blockCount; i++, pBlock++) {
        if (pBlock->dwFilePos == 0) {
            // assert((pBlock->dwCSize | pBlock->dwFlags | pBlock->dwFSize) == 0);
            return i;
        }
    }

    // app_fatal("Out of free block entries");
    return 0;
}

static void mpqapi_alloc_block(TMPQArchive * ha, DWORD block_offset, DWORD block_size)
{
    TMPQBlock* pBlock;
    DWORD i;
//restart:

    pBlock = ha->pBlockTable;
    for (i = ha->pHeader.dwBlockTableSize; i != 0; i--, pBlock++) {
        if (pBlock->dwFilePos != 0 && pBlock->dwFlags == 0 && pBlock->dwFSize == 0) {
            if (pBlock->dwFilePos + pBlock->dwCSize == block_offset) {
                // preceeding empty block -> mark the block-entry as unallocated and restart(?) with the merged region
                block_offset = pBlock->dwFilePos;
                block_size += pBlock->dwCSize;
                pBlock->dwFilePos = 0;
                pBlock->dwCSize = 0;
                //goto restart;
            }
            if (block_offset + block_size == pBlock->dwFilePos) {
                // succeeding empty block -> mark the block-entry as unallocated and restart(?) with the merged region
                block_size += pBlock->dwCSize;
                pBlock->dwFilePos = 0;
                pBlock->dwCSize = 0;
                //goto restart;
            }
        }
    }
#if DEBUG_MODE
    if (block_offset + block_size > ha->pHeader.dwArchiveSize) {
        app_fatal("MPQ free list error");
    }
#endif
    if (block_offset + block_size == ha->pHeader.dwArchiveSize) {
        ha->pHeader.dwArchiveSize = block_offset;
    } else {
        i = mpqapi_new_block(ha);
        // if (i == -1) return false;
        pBlock = &ha->pBlockTable[i];
        pBlock->dwFilePos = block_offset;
        pBlock->dwCSize = block_size;
        pBlock->dwFSize = 0;
        pBlock->dwFlags = 0;
    }
}

static TMPQHash * mpqapi_add_hash_entry(TMPQArchive * ha, const char * pszName)
{
    TMPQHash * pHash;
    DWORD i, h1, h2, h3, hashCount;

#if DEBUG_MODE
    if (GetFirstHashEntry(ha, pszName) != NULL)
        app_fatal("Hash collision between \"%s\" and existing file\n", pszName);
#endif
    h1 = HashStringSlash(pszName, MPQ_HASH_TABLE_INDEX);
    h2 = HashStringSlash(pszName, MPQ_HASH_NAME_A);
    h3 = HashStringSlash(pszName, MPQ_HASH_NAME_B);
    hashCount = ha->pHeader.dwHashTableSize;
    for (i = hashCount; i != 0; i--, h1++) {
        h1 &= hashCount - 1;
        pHash = &ha->pHashTable[h1];
        if (pHash->dwBlockIndex == HASH_ENTRY_FREE || pHash->dwBlockIndex == HASH_ENTRY_DELETED) {
            pHash->dwName1 = h2;
            pHash->dwName2 = h3;
            pHash->lcLocale = 0;
            pHash->Platform = 0;
            // pHash->hqBlock = block_index;
            return pHash;
        }
    }

    // app_fatal("Out of hash space");
    return NULL;
}

static DWORD mpqapi_find_free_block(TMPQArchive * ha, DWORD size)
{
    TMPQBlock * pBlock;
    DWORD i, result;

    pBlock = ha->pBlockTable;
    for (i = ha->pHeader.dwBlockTableSize; i != 0; i--, pBlock++) {
        result = pBlock->dwFilePos;
        if (result != 0 && pBlock->dwFlags == 0
         && pBlock->dwFSize == 0 && pBlock->dwCSize >= size) {
            pBlock->dwFilePos += size;
            pBlock->dwCSize -= size;

            if (pBlock->dwCSize == 0) {
                pBlock->dwFilePos = 0;
                // memset(pBlock, 0, sizeof(*pBlock));
            }

            return result;
        }
    }

    result = ha->pHeader.dwArchiveSize;
    ha->pHeader.dwArchiveSize = result + size;
    return result;
}

static DWORD SCompImplode(BYTE * srcData, DWORD size)
{
    BYTE * destData;
    char * work_buf;
    unsigned int destSize;

    work_buf = STORM_ALLOC(char, CMP_BUFFER_SIZE);
    destSize = 2 * size;
    if (destSize < 2 * CMP_IMPLODE_DICT_SIZE3)
        destSize = 2 * CMP_IMPLODE_DICT_SIZE3;

    destData = STORM_ALLOC(BYTE, destSize);
    if (work_buf != NULL && destData != NULL) {
        TDataInfo info = TDataInfo(srcData, size, destData, destSize);

        implode(PkwareBufferRead, PkwareBufferWrite, work_buf, &info);
        // copy the result only if the compression reduces the size of the data
        destSize = (size_t)info.pbOutBuff - (size_t)destData;
        if (destSize < size) {
            size = destSize;
            memcpy(srcData, destData, size);
        }
    }
    STORM_FREE(work_buf);
    STORM_FREE(destData);

    return size;
}

static bool mpqapi_write_file_contents(TMPQArchive * ha, void * pbData, DWORD dwLen, DWORD block)
{
    TMPQBlock * pBlk = &ha->pBlockTable[block];
    // assert(dwLen != 0);
    const DWORD num_sectors = ((dwLen - 1) / MPQ_SECTOR_SIZE_V1) + 1;
    const DWORD offset_table_bytesize = sizeof(DWORD) * (num_sectors + 1);
    pBlk->dwCSize = dwLen + offset_table_bytesize;
    pBlk->dwFilePos = mpqapi_find_free_block(ha, pBlk->dwCSize);
    pBlk->dwFSize = dwLen;
    pBlk->dwFlags = MPQ_FILE_EXISTS | MPQ_FILE_IMPLODE;

    // We populate the table of sector offset while we write the data.
    // We can't pre-populate it because we don't know the compressed sector sizes yet.
    // First offset is the start of the first sector, last offset is the end of the last sector.
    DWORD * sectoroffsettable = (DWORD*)STORM_ALLOC(BYTE, offset_table_bytesize);
    {
    DWORD destsize = offset_table_bytesize;
    unsigned cur_sector = 0;
    sectoroffsettable[0] = destsize;
    BYTE * src = (BYTE*)pbData;
    BYTE * dst = (BYTE*)pbData;
    while (true) {
        DWORD len = STORMLIB_MIN(dwLen, MPQ_SECTOR_SIZE_V1);
        DWORD cmplen = SCompImplode(src, len);
        if (src != dst) {
            memmove(dst, src, cmplen);
        }
        src += len;
        dst += cmplen;
        destsize += cmplen; // compressed length
        sectoroffsettable[++cur_sector] = destsize;
        if (dwLen > MPQ_SECTOR_SIZE_V1)
            dwLen -= MPQ_SECTOR_SIZE_V1;
        else
            break;
    }
    BSWAP_ARRAY32_UNSIGNED(sectoroffsettable, num_sectors);
    if (!FileStream_Write(&ha->pStream, pBlk->dwFilePos, reinterpret_cast<const char*>(sectoroffsettable), offset_table_bytesize))
        goto on_error;

    if (!FileStream_Write(&ha->pStream, pBlk->dwFilePos + offset_table_bytesize, reinterpret_cast<const char*>(pbData), destsize - offset_table_bytesize))
        goto on_error;

    if (destsize < pBlk->dwCSize) {
        const DWORD emptyBlockSize = pBlk->dwCSize - destsize;
        //if (emptyBlockSize >= (MPQ_SECTOR_SIZE_V1 / 4)) {
            pBlk->dwCSize = destsize;
            mpqapi_alloc_block(ha, pBlk->dwCSize + pBlk->dwFilePos, emptyBlockSize);
        //}
    }
    STORM_FREE(sectoroffsettable);
    return true;
    }
on_error:
    STORM_FREE(sectoroffsettable);
    return false;
}

bool   WINAPI SFileWriteFile(HANDLE hMpq, const char * szFileName, void * pvData, DWORD dwSize)
{
    TMPQArchive * ha = IsValidMpqHandle(hMpq);
    DWORD block;
    bool bResult = true;

    SFileRemoveFile(ha, szFileName);
    TMPQHash* pHash = mpqapi_add_hash_entry(ha, szFileName);
    // if (pHash == NULL) bResult = false;
    block = mpqapi_new_block(ha);
    // if (block == -1) bResult = false;
    pHash->dwBlockIndex = block;
    if (!mpqapi_write_file_contents(ha, pvData, dwSize, block)) {
        SFileRemoveFile(ha, szFileName);
        bResult = false;
    }
    return bResult;
}

// was in SFileAddFile.cpp
bool   WINAPI SFileRemoveFile(HANDLE hMpq, const char * szFileName)
{
    TMPQArchive * ha = IsValidMpqHandle(hMpq);

    TMPQHash* pHash;
    TMPQBlock* pBlock;
    int block_offset, block_size;
    bool bResult = false;

    pHash = GetFirstHashEntry(ha, szFileName);
    if (pHash != NULL) {
        pBlock = ha->pBlockTable + MPQ_BLOCK_INDEX(pHash);
        pHash->dwBlockIndex = HASH_ENTRY_DELETED;
        block_offset = pBlock->dwFilePos;
        block_size = pBlock->dwCSize;
        memset(pBlock, 0, sizeof(*pBlock));
        mpqapi_alloc_block(ha, block_offset, block_size);
        bResult = true;
    }
    return bResult;
}

void   WINAPI SFileRenameFile(HANDLE hMpq, const char * szOldFileName, const char * szNewFileName)
{
    TMPQArchive * ha = IsValidMpqHandle(hMpq);
    TMPQHash * pHash;
    DWORD block;

    pHash = GetFirstHashEntry(ha, szOldFileName);
    if (pHash != NULL) {
        // if (mpqapi_has_entry(ha, szNewFileName))
            SFileRemoveFile(ha, szNewFileName);
        // assert(pHash->dwBlockIndex != HASH_ENTRY_DELETED);
        block = MPQ_BLOCK_INDEX(pHash);
        pHash->dwBlockIndex = HASH_ENTRY_DELETED;
        pHash = mpqapi_add_hash_entry(ha, szNewFileName);
        // assert(pHash != NULL);
        pHash->dwBlockIndex = block;
    }
}
#endif // FULL

//-----------------------------------------------------------------------------
// bool SFileCloseArchive(HANDLE hMpq);
//

void WINAPI SFileCloseArchive(HANDLE hMpq)
{
    TMPQArchive * ha = IsValidMpqHandle(hMpq);
#ifdef FULL
    bool bResult = false;

    // Only if the handle is valid
    if(ha == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    // Invalidate the add file callback so it won't be called
    // when saving (listfile) and (attributes)
    ha->pfnAddFileCB = NULL;
    ha->pvAddFileUserData = NULL;

    // Flush all unsaved data to the storage
    bResult = SFileFlushArchive(hMpq);
#endif // FULL

    // Free all memory used by MPQ archive
    FreeArchiveHandle(ha);
#ifdef FULL
    return bResult;
#endif
}
