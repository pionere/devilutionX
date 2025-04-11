/*****************************************************************************/
/* SFileReadFile.cpp                      Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Description :                                                             */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* xx.xx.99  1.00  Lad  The first version of SFileReadFile.cpp               */
/* 24.03.99  1.00  Lad  Added the SFileGetFileInfo function                  */
/*****************************************************************************/

#define __STORMLIB_SELF__
#include "StormLib.h"
#include "StormCommon.h"

//-----------------------------------------------------------------------------
// External references (not public functions)
#ifdef FULL_COMP
int WINAPI SCompDecompressX(TMPQArchive * ha, void * pvOutBuffer, int * pcbOutBuffer, void * pbInBuffer, int cbInBuffer);
#endif
//-----------------------------------------------------------------------------
// Local functions

//  hf            - MPQ File handle.
//  pbBuffer      - Pointer to target buffer to store sectors.
//  dwByteOffset  - Position of sector in the file (relative to file begin)
//  dwBytesToRead - Number of bytes to read. Must be multiplier of sector size.
//  pdwBytesRead  - Stored number of bytes loaded
#ifdef FULL
static DWORD ReadMpqSectors(TMPQFile * hf, LPBYTE pbBuffer, DWORD dwByteOffset, DWORD dwBytesToRead, LPDWORD pdwBytesRead)
{
#ifdef FULL
    ULONGLONG RawFilePos;
#else
    FILESIZE_T RawFilePos;
#endif
    TMPQArchive * ha = hf->ha;
    TFileEntry * pFileEntry = hf->pFileEntry;
    LPBYTE pbRawSector = NULL;
    LPBYTE pbOutSector = pbBuffer;
    LPBYTE pbInSector = pbBuffer;
    DWORD dwRawBytesToRead;
    DWORD dwRawSectorOffset = dwByteOffset;
#ifdef FULL
    DWORD dwSectorsToRead = dwBytesToRead / ha->dwSectorSize;
    DWORD dwSectorIndex = dwByteOffset / ha->dwSectorSize;
#else
    DWORD dwSectorsToRead = dwBytesToRead / MPQ_SECTOR_SIZE_V1;
    DWORD dwSectorIndex = dwByteOffset / MPQ_SECTOR_SIZE_V1;
#endif
    DWORD dwSectorsDone = 0;
    DWORD dwBytesRead = 0;
    DWORD dwErrCode = ERROR_SUCCESS;

    // Note that dwByteOffset must be aligned to size of one sector
    // Note that dwBytesToRead must be a multiplier of one sector size
    // This is local function, so we won't check if that's true.
    // Note that files stored in single units are processed by a separate function

    // If there is not enough bytes remaining, cut dwBytesToRead
#ifdef FULL
    if ((dwByteOffset + dwBytesToRead) > hf->dwDataSize)
        dwBytesToRead = hf->dwDataSize - dwByteOffset;
#else
    if ((dwByteOffset + dwBytesToRead) > pFileEntry->dwFileSize)
        dwBytesToRead = pFileEntry->dwFileSize - dwByteOffset;
#endif
    dwRawBytesToRead = dwBytesToRead;

    // Perform all necessary work to do with compressed files
    if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK) {
        // If the sector positions are not loaded yet, do it
        if (hf->SectorOffsets == NULL) {
#ifdef FULL
            dwErrCode = AllocateSectorOffsets(hf, true);
#else
            dwErrCode = AllocateSectorOffsets(hf);
#endif
            if (dwErrCode != ERROR_SUCCESS || hf->SectorOffsets == NULL)
                return dwErrCode;
        }
#ifdef FULL
        // If the sector checksums are not loaded yet, load them now.
        if (hf->SectorChksums == NULL
         && (pFileEntry->dwFlags & MPQ_FILE_SECTOR_CRC)
         && hf->bLoadedSectorCRCs == false) {
            //
            // Sector CRCs is plain crap feature. It is almost never present,
            // often it's empty, or the end offset of sector CRCs is zero.
            // We only try to load sector CRCs once, and regardless if it fails
            // or not, we won't try that again for the given file.
            //
            AllocateSectorChecksums(hf, true);
            hf->bLoadedSectorCRCs = true;
        }
#endif
        // TODO: If the raw data MD5s are not loaded yet, load them now
        // Only do it if the MPQ is of format 4.0
//      if(ha->pHeader->wFormatVersion >= MPQ_FORMAT_VERSION_4 && ha->pHeader->dwRawChunkSize != 0)
//      {
//          dwErrCode = AllocateRawMD5s(hf, true);
//          if(dwErrCode != ERROR_SUCCESS)
//              return dwErrCode;
//      }

        // Assign the temporary buffer as target for read operation
        dwRawSectorOffset = hf->SectorOffsets[dwSectorIndex];
        dwRawBytesToRead = hf->SectorOffsets[dwSectorIndex + dwSectorsToRead] - dwRawSectorOffset;

        // If the file is compressed, also allocate secondary buffer
        pbInSector = pbRawSector = STORM_ALLOC(BYTE, dwRawBytesToRead);
        if (pbRawSector == NULL)
#ifdef FULL
            return ERROR_NOT_ENOUGH_MEMORY;
#else
            return ERROR_SUCCESS + 1;
#endif
    }

    // Calculate raw file offset where the sector(s) are stored.
    RawFilePos = CalculateRawSectorOffset(hf, dwRawSectorOffset);

    // Set file pointer and read all required sectors
    if (FileStream_Read(ha->pStream, &RawFilePos, pbInSector, dwRawBytesToRead)) {
        // Now we have to decrypt and decompress all file sectors that have been loaded
        for (DWORD i = 0; i < dwSectorsToRead; i++) {
#ifdef FULL
            DWORD dwRawBytesInThisSector = ha->dwSectorSize;
            DWORD dwBytesInThisSector = ha->dwSectorSize;
#else
            DWORD dwRawBytesInThisSector = MPQ_SECTOR_SIZE_V1;
            DWORD dwBytesInThisSector = MPQ_SECTOR_SIZE_V1;
#endif
            DWORD dwIndex = dwSectorIndex + i;

            // If there is not enough bytes in the last sector,
            // cut the number of bytes in this sector
            if (dwRawBytesInThisSector > dwBytesToRead)
                dwRawBytesInThisSector = dwBytesToRead;
            if (dwBytesInThisSector > dwBytesToRead)
                dwBytesInThisSector = dwBytesToRead;

            // If the file is compressed, we have to adjust the raw sector size
            if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK)
                dwRawBytesInThisSector = hf->SectorOffsets[dwIndex + 1] - hf->SectorOffsets[dwIndex];

            // If the file is encrypted, we have to decrypt the sector
            if (pFileEntry->dwFlags & MPQ_FILE_ENCRYPTED) {
                BSWAP_ARRAY32_UNSIGNED(pbInSector, dwRawBytesInThisSector);
#ifdef FULL
                // If we don't know the key, try to detect it by file content
                if (hf->dwFileKey == 0) {
                    hf->dwFileKey = DetectFileKeyByContent(pbInSector, dwBytesInThisSector, hf->dwDataSize);
                    if (hf->dwFileKey == 0) {
                        dwErrCode = ERROR_UNKNOWN_FILE_KEY;
                        break;
                    }
                }
#endif
                DecryptMpqBlock(pbInSector, dwRawBytesInThisSector, hf->dwFileKey + dwIndex);
                BSWAP_ARRAY32_UNSIGNED(pbInSector, dwRawBytesInThisSector);
            }

#ifdef FULL_CRC
            // If the file has sector CRC check turned on, perform it
            if (hf->bCheckSectorCRCs && hf->SectorChksums != NULL) {
                DWORD dwAdlerExpected = hf->SectorChksums[dwIndex];
                DWORD dwAdlerValue = 0;

                // We can only check sector CRC when it's not zero
                // Neither can we check it if it's 0xFFFFFFFF.
                if (dwAdlerExpected != 0 && dwAdlerExpected != 0xFFFFFFFF) {
                    dwAdlerValue = adler32(0, pbInSector, dwRawBytesInThisSector);
                    if (dwAdlerValue != dwAdlerExpected) {
                        dwErrCode = ERROR_CHECKSUM_ERROR;
                        break;
                    }
                }
            }
#endif

            // If the sector is really compressed, decompress it.
            // WARNING : Some sectors may not be compressed, it can be determined only
            // by comparing uncompressed and compressed size !!!
            if (dwRawBytesInThisSector < dwBytesInThisSector) {
                int cbOutSector = dwBytesInThisSector;
                int cbInSector = dwRawBytesInThisSector;
                int nResult = 0;
#ifdef FULL_COMP
                // Is the file compressed by Blizzard's multiple compression ?
                if(pFileEntry->dwFlags & MPQ_FILE_COMPRESS) {
                    // Remember the last used compression
                    hf->dwCompression0 = pbInSector[0];

                    // Decompress the data. We need to perform MPQ-specific decompression,
                    // as multiple Blizzard games may have their own decompression tables
                    // and even decompression methods.
                    nResult = SCompDecompressX(ha, pbOutSector, &cbOutSector, pbInSector, cbInSector);
                }
                // Is the file compressed by PKWARE Data Compression Library ?
                else if (pFileEntry->dwFlags & MPQ_FILE_IMPLODE) {
#else
                if (pFileEntry->dwFlags & MPQ_FILE_IMPLODE) {
#endif
                    nResult = SCompExplode(pbOutSector, &cbOutSector, pbInSector, cbInSector);
                }

                // Did the decompression fail ?
                if (nResult == 0) {
#ifdef FULL
                    dwErrCode = ERROR_FILE_CORRUPT;
#else
                    dwErrCode = ERROR_SUCCESS + 1;
#endif
                    break;
                }
            } else {
                if (pbOutSector != pbInSector)
                    memcpy(pbOutSector, pbInSector, dwBytesInThisSector);
            }

            // Move pointers
            dwBytesToRead -= dwBytesInThisSector;
            dwByteOffset += dwBytesInThisSector;
            dwBytesRead += dwBytesInThisSector;
            pbOutSector += dwBytesInThisSector;
            pbInSector += dwRawBytesInThisSector;
            dwSectorsDone++;
        }
    } else {
#ifdef FULL
        dwErrCode = GetLastError();
#else
        dwErrCode = ERROR_SUCCESS + 1;
#endif
    }

    // Free all used buffers
    if (pbRawSector != NULL)
        STORM_FREE(pbRawSector);

    // Give the caller thenumber of bytes read
    *pdwBytesRead = dwBytesRead;
    return dwErrCode;
}
#else
static DWORD ReadMpqSectors(TMPQFile * hf, LPBYTE pbBuffer, DWORD dwBytesToRead)
{
    FILESIZE_T RawFilePos;
    TMPQArchive * ha = hf->ha;
#ifdef FULL
    TFileEntry * pFileEntry = hf->pFileEntry;
#else
    TMPQBlock * pFileEntry = hf->pFileEntry;
#endif
    LPBYTE pbRawSector = NULL;
    LPBYTE pbOutSector = pbBuffer;
    LPBYTE pbInSector = pbBuffer;
    DWORD dwRawBytesToRead;
#ifdef FULL
    DWORD dwRawSectorOffset = dwByteOffset;
    DWORD dwSectorsToRead = dwBytesToRead / ha->dwSectorSize;
    DWORD dwSectorIndex = dwByteOffset / ha->dwSectorSize;
    DWORD dwBytesRead = 0;
#else
    DWORD dwRawSectorOffset = 0;
    DWORD dwSectorsToRead = ((dwBytesToRead - 1) / MPQ_SECTOR_SIZE_V1) + 1;
    DWORD dwSectorIndex = 0;
#endif
    DWORD dwErrCode = ERROR_SUCCESS;
    LPDWORD SectorOffsets = NULL;
    // Note that dwByteOffset must be aligned to size of one sector
    // Note that dwBytesToRead must be a multiplier of one sector size
    // This is local function, so we won't check if that's true.
    // Note that files stored in single units are processed by a separate function

    // If there is not enough bytes remaining, cut dwBytesToRead
#ifdef FULL
    if((dwByteOffset + dwBytesToRead) > hf->dwDataSize)
        dwBytesToRead = hf->dwDataSize - dwByteOffset;
#endif
    dwRawBytesToRead = dwBytesToRead;

    // Perform all necessary work to do with compressed files
    if(pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK) {
        // If the sector positions are not loaded yet, do it
        SectorOffsets = AllocateSectorOffsets(hf);
        if(SectorOffsets == NULL)
            return ERROR_SUCCESS + 1;
#ifdef FULL
        // If the sector checksums are not loaded yet, load them now.
        if(hf->SectorChksums == NULL
         && (pFileEntry->dwFlags & MPQ_FILE_SECTOR_CRC)
         && hf->bLoadedSectorCRCs == false) {
            //
            // Sector CRCs is plain crap feature. It is almost never present,
            // often it's empty, or the end offset of sector CRCs is zero.
            // We only try to load sector CRCs once, and regardless if it fails
            // or not, we won't try that again for the given file.
            //
            AllocateSectorChecksums(hf, true);
            hf->bLoadedSectorCRCs = true;
        }
#endif
        // TODO: If the raw data MD5s are not loaded yet, load them now
        // Only do it if the MPQ is of format 4.0
//      if(ha->pHeader->wFormatVersion >= MPQ_FORMAT_VERSION_4 && ha->pHeader->dwRawChunkSize != 0)
//      {
//          dwErrCode = AllocateRawMD5s(hf, true);
//          if(dwErrCode != ERROR_SUCCESS)
//              return dwErrCode;
//      }

        // Assign the temporary buffer as target for read operation
        dwRawSectorOffset = SectorOffsets[dwSectorIndex];
        dwRawBytesToRead = SectorOffsets[dwSectorIndex + dwSectorsToRead] - dwRawSectorOffset;

        // If the file is compressed, also allocate secondary buffer
        pbInSector = pbRawSector = STORM_ALLOC(BYTE, dwRawBytesToRead);
        if(pbRawSector == NULL) {
            STORM_FREE(SectorOffsets);
            return ERROR_SUCCESS + 1;
        }
    }

    // Calculate raw file offset where the sector(s) are stored.
    RawFilePos = CalculateRawSectorOffset(hf, dwRawSectorOffset);

    // Set file pointer and read all required sectors
    if(FileStream_Read(&ha->pStream, RawFilePos, pbInSector, dwRawBytesToRead)) {
        // Now we have to decrypt and decompress all file sectors that have been loaded
        for (DWORD i = 0; i < dwSectorsToRead; i++) {
#ifdef FULL
            DWORD dwRawBytesInThisSector = ha->dwSectorSize;
            DWORD dwBytesInThisSector = ha->dwSectorSize;
#else
            DWORD dwRawBytesInThisSector = MPQ_SECTOR_SIZE_V1;
            DWORD dwBytesInThisSector = MPQ_SECTOR_SIZE_V1;
#endif
            DWORD dwIndex = dwSectorIndex + i;

            // If there is not enough bytes in the last sector,
            // cut the number of bytes in this sector
            if(dwRawBytesInThisSector > dwBytesToRead)
                dwRawBytesInThisSector = dwBytesToRead;
            if(dwBytesInThisSector > dwBytesToRead)
                dwBytesInThisSector = dwBytesToRead;

            // If the file is compressed, we have to adjust the raw sector size
            if(pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK)
                dwRawBytesInThisSector = SectorOffsets[dwIndex + 1] - SectorOffsets[dwIndex];

            // If the file is encrypted, we have to decrypt the sector
            if(pFileEntry->dwFlags & MPQ_FILE_ENCRYPTED) {
                BSWAP_ARRAY32_UNSIGNED(pbInSector, dwRawBytesInThisSector);
#ifdef FULL
                // If we don't know the key, try to detect it by file content
                if (hf->dwFileKey == 0) {
                    hf->dwFileKey = DetectFileKeyByContent(pbInSector, dwBytesInThisSector, hf->dwDataSize);
                    if (hf->dwFileKey == 0) {
                        dwErrCode = ERROR_UNKNOWN_FILE_KEY;
                        break;
                    }
                }
#endif
                DecryptMpqBlock(pbInSector, dwRawBytesInThisSector, hf->dwFileKey + dwIndex);
                BSWAP_ARRAY32_UNSIGNED(pbInSector, dwRawBytesInThisSector);
            }

#ifdef FULL_CRC
            // If the file has sector CRC check turned on, perform it
            if(hf->bCheckSectorCRCs && hf->SectorChksums != NULL) {
                DWORD dwAdlerExpected = hf->SectorChksums[dwIndex];
                DWORD dwAdlerValue = 0;

                // We can only check sector CRC when it's not zero
                // Neither can we check it if it's 0xFFFFFFFF.
                if (dwAdlerExpected != 0 && dwAdlerExpected != 0xFFFFFFFF) {
                    dwAdlerValue = adler32(0, pbInSector, dwRawBytesInThisSector);
                    if (dwAdlerValue != dwAdlerExpected) {
                        dwErrCode = ERROR_CHECKSUM_ERROR;
                        break;
                    }
                }
            }
#endif

            // If the sector is really compressed, decompress it.
            // WARNING : Some sectors may not be compressed, it can be determined only
            // by comparing uncompressed and compressed size !!!
            if(dwRawBytesInThisSector < dwBytesInThisSector) {
#ifdef FULL_COMP
                int cbOutSector = dwBytesInThisSector;
                int cbInSector = dwRawBytesInThisSector;
                int nResult = 0;

                // Is the file compressed by Blizzard's multiple compression ?
                if(pFileEntry->dwFlags & MPQ_FILE_COMPRESS) {
                    // Remember the last used compression
                    hf->dwCompression0 = pbInSector[0];

                    // Decompress the data. We need to perform MPQ-specific decompression,
                    // as multiple Blizzard games may have their own decompression tables
                    // and even decompression methods.
                    nResult = SCompDecompressX(ha, pbOutSector, &cbOutSector, pbInSector, cbInSector);
                }
                // Is the file compressed by PKWARE Data Compression Library ?
                else if (pFileEntry->dwFlags & MPQ_FILE_IMPLODE) {
                    nResult = SCompExplode(pbOutSector, &cbOutSector, pbInSector, cbInSector);
                }

                // Did the decompression fail ?
                if (nResult == 0) {
                    dwErrCode = ERROR_FILE_CORRUPT;
                    break;
                }
#else
                dwErrCode = SCompExplode(pbOutSector, dwBytesInThisSector, pbInSector, dwRawBytesInThisSector);

                // Did the decompression fail ?
                if(dwErrCode != ERROR_SUCCESS) {
                    break;
                }
#endif
            } else {
                if(pbOutSector != pbInSector)
                    memcpy(pbOutSector, pbInSector, dwBytesInThisSector);
            }

            // Move pointers
            dwBytesToRead -= dwBytesInThisSector;
            pbOutSector += dwBytesInThisSector;
            pbInSector += dwRawBytesInThisSector;
        }
    } else {
        dwErrCode = ERROR_SUCCESS + 1;
    }

    // Free all used buffers
    STORM_FREE(pbRawSector);
    STORM_FREE(SectorOffsets);

    return dwErrCode;
}
#endif
#ifdef FULL
static DWORD ReadMpqFileSingleUnit(TMPQFile *hf, void *pvBuffer, DWORD dwToRead, LPDWORD pdwBytesRead)
{
    DWORD dwFilePos = hf->dwFilePos;
    ULONGLONG RawFilePos = hf->RawFilePos;
    TMPQArchive * ha = hf->ha;
    TFileEntry * pFileEntry = hf->pFileEntry;
    LPBYTE pbCompressed = NULL;
    LPBYTE pbRawData;
    DWORD dwErrCode = ERROR_SUCCESS;

    // If the file buffer is not allocated yet, do it.
    if (hf->pbFileSector == NULL) {
        dwErrCode = AllocateSectorBuffer(hf);
        if (dwErrCode != ERROR_SUCCESS || hf->pbFileSector == NULL)
            return dwErrCode;
    }

    // If the file is a patch file, adjust raw data offset
    if (hf->pPatchInfo != NULL)
        RawFilePos += hf->pPatchInfo->dwLength;
    pbRawData = hf->pbFileSector;

    // If the file sector is not loaded yet, do it
    if (hf->dwSectorOffs != 0) {
        // Is the file compressed?
        if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK) {
            // Allocate space for compressed data
            pbCompressed = STORM_ALLOC(BYTE, pFileEntry->dwCmpSize);
            if(pbCompressed == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            pbRawData = pbCompressed;
        }

        // Load the raw (compressed, encrypted) data
        if (!FileStream_Read(ha->pStream, &RawFilePos, pbRawData, pFileEntry->dwCmpSize)) {
            STORM_FREE(pbCompressed);
            return GetLastError();
        }

        // If the file is encrypted, we have to decrypt the data first
        if (pFileEntry->dwFlags & MPQ_FILE_ENCRYPTED) {
            BSWAP_ARRAY32_UNSIGNED(pbRawData, pFileEntry->dwCmpSize);
            DecryptMpqBlock(pbRawData, pFileEntry->dwCmpSize, hf->dwFileKey);
            BSWAP_ARRAY32_UNSIGNED(pbRawData, pFileEntry->dwCmpSize);
        }

        // If the file is compressed, we have to decompress it now
        if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK) {
            int cbOutBuffer = (int)hf->dwDataSize;
            int cbInBuffer = (int)pFileEntry->dwCmpSize;
            int nResult = 0;

            //
            // If the file is an incremental patch, the size of compressed data
            // is determined as pFileEntry->dwCmpSize - sizeof(TPatchInfo)
            //
            // In "wow-update-12694.MPQ" from Wow-Cataclysm BETA:
            //
            // File                                    CmprSize   DcmpSize DataSize Compressed?
            // --------------------------------------  ---------- -------- -------- ---------------
            // esES\DBFilesClient\LightSkyBox.dbc      0xBE->0xA2  0xBC     0xBC     Yes
            // deDE\DBFilesClient\MountCapability.dbc  0x93->0x77  0x77     0x77     No
            //

            if (pFileEntry->dwFlags & MPQ_FILE_PATCH_FILE)
                cbInBuffer = cbInBuffer - sizeof(TPatchInfo);
            // Is the file compressed by Blizzard's multiple compression ?
            if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS) {
                // Remember the last used compression
                hf->dwCompression0 = pbRawData[0];

                // Decompress the file
                if (ha->pHeader->wFormatVersion >= MPQ_FORMAT_VERSION_2)
                    nResult = SCompDecompress2(hf->pbFileSector, &cbOutBuffer, pbRawData, cbInBuffer);
                else
                    nResult = SCompDecompress(hf->pbFileSector, &cbOutBuffer, pbRawData, cbInBuffer);
            }

            // Is the file compressed by PKWARE Data Compression Library ?
            // Note: Single unit files compressed with IMPLODE are not supported by Blizzard
            else if(pFileEntry->dwFlags & MPQ_FILE_IMPLODE)
                nResult = SCompExplode(hf->pbFileSector, &cbOutBuffer, pbRawData, cbInBuffer);

            dwErrCode = (nResult != 0) ? ERROR_SUCCESS : ERROR_FILE_CORRUPT;
        } else {
            if (hf->pbFileSector != NULL && pbRawData != hf->pbFileSector)
                memcpy(hf->pbFileSector, pbRawData, hf->dwDataSize);
        }

        // Free the decompression buffer.
        if (pbCompressed != NULL)
            STORM_FREE(pbCompressed);

        // The file sector is now properly loaded
        hf->dwSectorOffs = 0;
    }

    // At this moment, we have the file loaded into the file buffer.
    // Copy as much as the caller wants
    if (dwErrCode == ERROR_SUCCESS && hf->dwSectorOffs == 0) {
        // File position is greater or equal to file size ?
        if (dwFilePos >= hf->dwDataSize) {
            *pdwBytesRead = 0;
            return ERROR_SUCCESS;
        }

        // If not enough bytes remaining in the file, cut them
        if ((hf->dwDataSize - dwFilePos) < dwToRead)
            dwToRead = (hf->dwDataSize - dwFilePos);

        // Copy the bytes
        memcpy(pvBuffer, hf->pbFileSector + dwFilePos, dwToRead);

        // Give the number of bytes read
        *pdwBytesRead = dwToRead;
    }

    // An error, sorry
    return dwErrCode;
}

static DWORD ReadMpkFileSingleUnit(TMPQFile *hf, void *pvBuffer, DWORD dwToRead, LPDWORD pdwBytesRead)
{
    DWORD dwFilePos = hf->dwFilePos;
    ULONGLONG RawFilePos = hf->RawFilePos + 0x0C;   // For some reason, MPK files start at position (hf->RawFilePos + 0x0C)
    TMPQArchive * ha = hf->ha;
    TFileEntry * pFileEntry = hf->pFileEntry;
    LPBYTE pbCompressed = NULL;
    LPBYTE pbRawData = hf->pbFileSector;
    DWORD dwErrCode = ERROR_SUCCESS;

    // We do not support patch files in MPK archives
    assert(hf->pPatchInfo == NULL);

    // If the file buffer is not allocated yet, do it.
    if (hf->pbFileSector == NULL) {
        dwErrCode = AllocateSectorBuffer(hf);
        if (dwErrCode != ERROR_SUCCESS || hf->pbFileSector == NULL)
            return dwErrCode;
        pbRawData = hf->pbFileSector;
    }

    // If the file sector is not loaded yet, do it
    if (hf->dwSectorOffs != 0) {
        // Is the file compressed?
        if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK) {
            // Allocate space for compressed data
            pbCompressed = STORM_ALLOC(BYTE, pFileEntry->dwCmpSize);
            if(pbCompressed == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
            pbRawData = pbCompressed;
        }

        // Load the raw (compressed, encrypted) data
        if (!FileStream_Read(ha->pStream, &RawFilePos, pbRawData, pFileEntry->dwCmpSize)) {
            STORM_FREE(pbCompressed);
            return GetLastError();
        }

        // If the file is encrypted, we have to decrypt the data first
        if (pFileEntry->dwFlags & MPQ_FILE_ENCRYPTED) {
            DecryptMpkTable(pbRawData, pFileEntry->dwCmpSize);
        }

        // If the file is compressed, we have to decompress it now
        if (pFileEntry->dwFlags & MPQ_FILE_COMPRESS_MASK) {
            int cbOutBuffer = (int)hf->dwDataSize;

            hf->dwCompression0 = pbRawData[0];
            if(!SCompDecompressMpk(hf->pbFileSector, &cbOutBuffer, pbRawData, (int)pFileEntry->dwCmpSize))
                dwErrCode = ERROR_FILE_CORRUPT;
        } else {
            if (pbRawData != hf->pbFileSector)
                memcpy(hf->pbFileSector, pbRawData, hf->dwDataSize);
        }

        // Free the decompression buffer.
        if (pbCompressed != NULL)
            STORM_FREE(pbCompressed);

        // The file sector is now properly loaded
        hf->dwSectorOffs = 0;
    }

    // At this moment, we have the file loaded into the file buffer.
    // Copy as much as the caller wants
    if (dwErrCode == ERROR_SUCCESS && hf->dwSectorOffs == 0) {
        // File position is greater or equal to file size ?
        if (dwFilePos >= hf->dwDataSize) {
            *pdwBytesRead = 0;
            return ERROR_SUCCESS;
        }

        // If not enough bytes remaining in the file, cut them
        if ((hf->dwDataSize - dwFilePos) < dwToRead)
            dwToRead = (hf->dwDataSize - dwFilePos);

        // Copy the bytes
        memcpy(pvBuffer, hf->pbFileSector + dwFilePos, dwToRead);

        // Give the number of bytes read
        *pdwBytesRead = dwToRead;
        return ERROR_SUCCESS;
    }

    // An error, sorry
    return ERROR_CAN_NOT_COMPLETE;
}

static DWORD ReadMpqFileSectorFile(TMPQFile *hf, void *pvBuffer, DWORD dwBytesToRead, LPDWORD pdwBytesRead)
#else
static DWORD ReadMpqFileSectorFile(TMPQFile *hf, void *pvBuffer, DWORD dwBytesToRead)
#endif
{
#ifdef FULL
    TMPQArchive * ha = hf->ha;
    DWORD dwFilePos = hf->dwFilePos;
#endif
    LPBYTE pbBuffer = (BYTE *)pvBuffer;
#ifdef FULL
    DWORD dwTotalBytesRead = 0;                         // Total bytes read in all three parts
    DWORD dwSectorSizeMask = ha->dwSectorSize - 1;      // Mask for block size, usually 0x0FFF
    DWORD dwFileSectorPos;                              // File offset of the loaded sector
    DWORD dwBytesRead;                                  // Number of bytes read (temporary variable)
#endif
    DWORD dwErrCode;
#ifdef FULL
    // If not enough bytes in the file remaining, cut them
    if (dwFilePos >= hf->dwDataSize)
        dwBytesToRead = 0;
    else if (dwBytesToRead > (hf->dwDataSize - dwFilePos))
        dwBytesToRead = (hf->dwDataSize - dwFilePos);

    // Compute sector position in the file
    dwFileSectorPos = dwFilePos & ~dwSectorSizeMask;  // Position in the block

    // If the file sector buffer is not allocated yet, do it now
    if (hf->pbFileSector == NULL) {
        dwErrCode = AllocateSectorBuffer(hf);
        if (dwErrCode != ERROR_SUCCESS || hf->pbFileSector == NULL)
            return dwErrCode;
    }

    // Load the first (incomplete) file sector
    if (dwFilePos & dwSectorSizeMask) {
#ifdef FULL
        DWORD dwBytesInSector = ha->dwSectorSize;
#else
        DWORD dwBytesInSector = MPQ_SECTOR_SIZE_V1;
#endif
        DWORD dwBufferOffs = dwFilePos & dwSectorSizeMask;
        DWORD dwToCopy;

        // Is the file sector already loaded ?
        if (hf->dwSectorOffs != dwFileSectorPos) {
            // Load one MPQ sector into archive buffer
#ifdef FULL
            dwErrCode = ReadMpqSectors(hf, hf->pbFileSector, dwFileSectorPos, ha->dwSectorSize, &dwBytesInSector);
#else
            dwErrCode = ReadMpqSectors(hf, hf->pbFileSector, dwFileSectorPos, dwBytesInSector, &dwBytesInSector);
#endif
            if(dwErrCode != ERROR_SUCCESS)
                return dwErrCode;

            // Remember that the data loaded to the sector have new file offset
            hf->dwSectorOffs = dwFileSectorPos;
        } else {
            if ((dwFileSectorPos + dwBytesInSector) > hf->dwDataSize)
                dwBytesInSector = hf->dwDataSize - dwFileSectorPos;
        }

        // Copy the data from the offset in the loaded sector to the end of the sector
        dwToCopy = dwBytesInSector - dwBufferOffs;
        if (dwToCopy > dwBytesToRead)
            dwToCopy = dwBytesToRead;

        // Copy data from sector buffer into target buffer
        memcpy(pbBuffer, hf->pbFileSector + dwBufferOffs, dwToCopy);

        // Update pointers and byte counts
        dwTotalBytesRead += dwToCopy;
        dwFileSectorPos  += dwBytesInSector;
        pbBuffer         += dwToCopy;
        dwBytesToRead    -= dwToCopy;
    }
#endif
    // Load the whole ("middle") sectors only if there is at least one full sector to be read
#ifdef FULL
    if (dwBytesToRead >= ha->dwSectorSize) {
#else
    if (dwBytesToRead > 0) {
#endif
#ifdef FULL
        DWORD dwBlockBytes = dwBytesToRead & ~dwSectorSizeMask;

        // Load all sectors to the output buffer
        dwErrCode = ReadMpqSectors(hf, pbBuffer, dwFileSectorPos, dwBlockBytes, &dwBytesRead);
        if(dwErrCode != ERROR_SUCCESS)
#else
        dwErrCode = ReadMpqSectors(hf, pbBuffer, dwBytesToRead);
#endif
            return dwErrCode;
#ifdef FULL
        // Update pointers
        dwTotalBytesRead += dwBytesRead;
        dwFileSectorPos  += dwBytesRead;
        pbBuffer         += dwBytesRead;
        dwBytesToRead    -= dwBytesRead;
#endif
    }
#ifdef FULL
    // Read the terminating sector
    if (dwBytesToRead > 0) {
        DWORD dwToCopy = ha->dwSectorSize;

        // Is the file sector already loaded ?
        if (hf->dwSectorOffs != dwFileSectorPos) {
            // Load one MPQ sector into archive buffer
            dwErrCode = ReadMpqSectors(hf, hf->pbFileSector, dwFileSectorPos, ha->dwSectorSize, &dwBytesRead);
            // Load one MPQ sector into archive buffer
            if(dwErrCode != ERROR_SUCCESS)
                return dwErrCode;
            // Remember that the data loaded to the sector have new file offset
            hf->dwSectorOffs = dwFileSectorPos;
        }
        // Check number of bytes read
        if (dwToCopy > dwBytesToRead)
            dwToCopy = dwBytesToRead;

        // Copy the data from the cached last sector to the caller's buffer
        memcpy(pbBuffer, hf->pbFileSector, dwToCopy);

        // Update pointers
        dwTotalBytesRead += dwToCopy;
    }

    // Store total number of bytes read to the caller
    *pdwBytesRead = dwTotalBytesRead;
#endif
    return ERROR_SUCCESS;
}

#ifdef FULL
static DWORD ReadMpqFilePatchFile(TMPQFile *hf, void *pvBuffer, DWORD dwToRead, LPDWORD pdwBytesRead)
{
    DWORD dwFilePos = hf->dwFilePos;
    TMPQPatcher Patcher;
    DWORD dwBytesToRead = dwToRead;
    DWORD dwBytesRead = 0;
    DWORD dwErrCode = ERROR_SUCCESS;

    // Make sure that the patch file is loaded completely
    if(dwErrCode == ERROR_SUCCESS && hf->pbFileData == NULL)
    {
        // Initialize patching process and allocate data
        dwErrCode = Patch_InitPatcher(&Patcher, hf);
        if(dwErrCode != ERROR_SUCCESS)
            return dwErrCode;

        // Set the current data size
        Patcher.cbFileData = hf->pFileEntry->dwFileSize;

        // Initialize the patcher object with initial file data
        hf->dwFilePos = 0; // FIXME: check if this is safe
        if(hf->pFileEntry->dwFlags & MPQ_FILE_SINGLE_UNIT)
            dwErrCode = ReadMpqFileSingleUnit(hf, Patcher.pbFileData1, Patcher.cbFileData, &dwBytesRead);
        else
            dwErrCode = ReadMpqFileSectorFile(hf, Patcher.pbFileData1, Patcher.cbFileData, &dwBytesRead);
        hf->dwFilePos = dwFilePos;

        // Perform the patching process
        if(dwErrCode == ERROR_SUCCESS)
            dwErrCode = Patch_Process(&Patcher, hf);

        // Finalize the patcher structure
        Patch_Finalize(&Patcher);
        dwBytesRead = 0;
    }

    // If there is something to read, do it
    if(dwErrCode == ERROR_SUCCESS)
    {
        if(dwFilePos < hf->cbFileData)
        {
            // Make sure we don't copy more than file size
            if((dwFilePos + dwToRead) > hf->cbFileData)
                dwToRead = hf->cbFileData - dwFilePos;

            // Copy the appropriate amount of the file data to the caller's buffer
            memcpy(pvBuffer, hf->pbFileData + dwFilePos, dwToRead);
            dwBytesRead = dwToRead;
        }

        // Set the proper error code
        dwErrCode = (dwBytesRead == dwBytesToRead) ? ERROR_SUCCESS : ERROR_HANDLE_EOF;
    }

    // Give the result to the caller
    if(pdwBytesRead != NULL)
        *pdwBytesRead = dwBytesRead;
    return dwErrCode;
}

static DWORD ReadMpqFileLocalFile(TMPQFile *hf, void *pvBuffer, DWORD dwToRead, LPDWORD pdwBytesRead)
#else
static DWORD ReadMpqFileLocalFile(TMPQFile *hf, void *pvBuffer, DWORD dwToRead)
#endif // FULL
{
#ifdef FULL
    ULONGLONG FilePosition1;
    ULONGLONG FilePosition2;
#else
    FILESIZE_T FilePosition1;
#endif
    DWORD dwBytesRead = 0;
    DWORD dwErrCode = ERROR_SUCCESS;

#ifdef FULL
    assert(hf->pStream != NULL);
    FileStream_GetPos(hf->pStream, &FilePosition1);
#else
    FilePosition1 = 0;
#endif

    // Because stream I/O functions are designed to read
    // "all or nothing", we compare file position before and after,
    // and if they differ, we assume that number of bytes read
    // is the difference between them
#ifdef FULL
    if (!FileStream_Read(hf->pStream, &FilePosition1, pvBuffer, dwToRead)) {
        // If not all bytes have been read, then return the number of bytes read
        if ((dwErrCode = GetLastError()) == ERROR_HANDLE_EOF) {
            FileStream_GetPos(hf->pStream, &FilePosition2);
            dwBytesRead = (DWORD)(FilePosition2 - FilePosition1);
        }
    } else {
        dwBytesRead = dwToRead;
    }
#else
    if(!FileStream_Read(&hf->pStream, FilePosition1, pvBuffer, dwToRead))
        dwErrCode = ERROR_SUCCESS + 1;
#endif
#ifdef FULL
    *pdwBytesRead = dwBytesRead;
#endif
    return dwErrCode;
}

//-----------------------------------------------------------------------------
// SFileReadFile
#ifdef FULL
bool WINAPI SFileReadFile(HANDLE hFile, void * pvBuffer, DWORD dwToRead/*, LPDWORD pdwRead*/)
{
    TMPQFile * hf;
    DWORD dwBytesRead = 0;                      // Number of bytes read
    DWORD dwErrCode = ERROR_SUCCESS;

    // Always zero the result
    //if (pdwRead != NULL)
    //    *pdwRead = 0;
#ifdef FULL
    // Check valid parameters
    if((hf = IsValidFileHandle(hFile)) == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    if (pvBuffer == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    // If we didn't load the patch info yet, do it now
    if (hf->pFileEntry != NULL
     && (hf->pFileEntry->dwFlags & MPQ_FILE_PATCH_FILE)
     && hf->pPatchInfo == NULL) {
        dwErrCode = AllocatePatchInfo(hf, true);
        if (dwErrCode != ERROR_SUCCESS || hf->pPatchInfo == NULL) {
            SetLastError(dwErrCode);
            return false;
        }
    }

    // Clear the last used compression
    hf->dwCompression0 = 0;

    // If the file is local file, read the data directly from the stream
    if (hf->pStream != NULL) {
        dwErrCode = ReadMpqFileLocalFile(hf, pvBuffer, dwToRead, &dwBytesRead);
    // If the file is a patch file, we have to read it special way
    } else if (hf->hfPatch != NULL
     && (hf->pFileEntry->dwFlags & MPQ_FILE_PATCH_FILE) == 0) {
        dwErrCode = ReadMpqFilePatchFile(hf, pvBuffer, dwToRead, &dwBytesRead);
    // If the archive is a MPK archive, we need special way to read the file
    } else if (hf->ha->dwSubType == MPQ_SUBTYPE_MPK) {
        dwErrCode = ReadMpkFileSingleUnit(hf, pvBuffer, dwToRead, &dwBytesRead);
    // If the file is single unit file, redirect it to read file
    } else if(hf->pFileEntry->dwFlags & MPQ_FILE_SINGLE_UNIT) {
        dwErrCode = ReadMpqFileSingleUnit(hf, pvBuffer, dwToRead, &dwBytesRead);
#else
    // If the file is local file, read the data directly from the stream
    if (hf->pFileEntry == NULL) {
        dwErrCode = ReadMpqFileLocalFile(hf, pvBuffer, dwToRead);
#endif
    // Otherwise read it as sector based MPQ file
    } else {
#ifdef FULL
        dwErrCode = ReadMpqFileSectorFile(hf, pvBuffer, dwToRead, &dwBytesRead);
#else
        dwErrCode = ReadMpqFileSectorFile(hf, pvBuffer, dwToRead);
#endif
    }
#ifdef FULL
    // Increment the file position
    hf->dwFilePos += dwBytesRead;

    // Give the caller the number of bytes read
    //if (pdwRead != NULL)
    //   *pdwRead = dwBytesRead;

    // If the read operation succeeded, but not full number of bytes was read,
    // set the last error to ERROR_HANDLE_EOF
    if (dwErrCode == ERROR_SUCCESS && (dwBytesRead < dwToRead))
        dwErrCode = ERROR_HANDLE_EOF;

    // If something failed, set the last error value
    if (dwErrCode != ERROR_SUCCESS)
        SetLastError(dwErrCode);
#endif
    return (dwErrCode == ERROR_SUCCESS);
}
#else
DWORD   WINAPI SFileReadMpqFileEx(HANDLE hFile, void * lpBuffer, DWORD dwToRead)
{
    TMPQFile *hf = IsValidFileHandle(hFile);

    return ReadMpqFileSectorFile(hf, lpBuffer, dwToRead);
}

DWORD   WINAPI SFileReadLocalFileEx(HANDLE hFile, void * lpBuffer, DWORD dwToRead)
{
    TMPQFile *hf = IsValidFileHandle(hFile);

    return ReadMpqFileLocalFile(hf, lpBuffer, dwToRead);
}
#endif
//-----------------------------------------------------------------------------
// SFileGetFileSize
#ifdef FULL
DWORD WINAPI SFileGetFileSize(HANDLE hFile)
{
#ifdef FULL
    ULONGLONG FileSize;
#else
    DWORD FileSize = 0;
#endif
    TMPQFile *hf = IsValidFileHandle(hFile);

    // Validate the file handle before we go on
    if (hf != NULL) {
#ifdef FULL
        // Make sure that the variable is initialized
        FileSize = 0;

        // If the file is patched file, we have to get the size of the last version
        if (hf->hfPatch != NULL) {
            // Walk through the entire patch chain, take the last version
            while (hf != NULL) {
                // Get the size of the currently pointed version
                FileSize = hf->pFileEntry->dwFileSize;

                // Move to the next patch file in the hierarchy
                hf = hf->hfPatch;
            }
        } else {
#endif // FULL
            // Is it a local file ?
#ifdef FULL
            if (hf->pStream != NULL) {
                FileStream_GetSize(hf->pStream, &FileSize);
#else
            if (hf->pFileEntry == NULL) {
                FileSize = (DWORD)FileStream_GetSize(&hf->pStream);
#endif
            } else {
#ifdef FULL
                FileSize = hf->dwDataSize;
#else
                FileSize = hf->pFileEntry->dwFSize;
#endif
            }
        }
#ifdef FULL
        // If opened from archive, return file size
        //if (pdwFileSizeHigh != NULL)
        //    *pdwFileSizeHigh = (DWORD)(FileSize >> 32);
        return FileSize;
    }
    SetLastError(ERROR_INVALID_HANDLE);
    return SFILE_INVALID_SIZE;
#else
    return FileSize;
#endif
}
#else
DWORD WINAPI SFileGetLocalFileSize(HANDLE hFile)
{
    DWORD FileSize = 0;
    TMPQFile *hf = IsValidFileHandle(hFile);

    FileSize = (DWORD)FileStream_GetSize(&hf->pStream);
    return FileSize;
}
DWORD WINAPI SFileGetMpqFileSize(HANDLE hFile)
{
    DWORD FileSize = 0;
    TMPQFile *hf = IsValidFileHandle(hFile);

    FileSize = hf->pFileEntry->dwFSize;
    return FileSize;
}
#endif
#ifdef FULL
DWORD WINAPI SFileGetFilePointer(HANDLE hFile)
{
    TMPQFile *hf = IsValidFileHandle(hFile);
    ULONGLONG CurrPosition;

    // If the hFile is not a valid file handle, return an error.
    if (hf == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
        return SFILE_INVALID_POS;
    }
    // Retrieve the current file position
    if (hf->pStream != NULL) {
        FileStream_GetPos(hf->pStream, &CurrPosition);
    } else {
        CurrPosition = hf->dwFilePos;
    }
    return (DWORD)CurrPosition;
}

DWORD WINAPI SFileSetFilePointer(HANDLE hFile, long lFilePos, unsigned dwMoveMethod)
{
    TMPQFile *hf = IsValidFileHandle(hFile);
    ULONGLONG OldPosition;
    ULONGLONG NewPosition;
    ULONGLONG FileSize;
    ULONGLONG DeltaPos;

    // If the hFile is not a valid file handle, return an error.
    if (hf == NULL) {
        SetLastError(ERROR_INVALID_HANDLE);
        return SFILE_INVALID_POS;
    }

    // Retrieve the file size for handling the limits
    if (hf->pStream != NULL) {
        FileStream_GetSize(hf->pStream, &FileSize);
    } else {
        FileSize = SFileGetFileSize(hf);
    }

    // Handle the NULL and non-NULL values of plFilePosHigh
    // Non-NULL: The DeltaPos is combined from lFilePos and *lpFilePosHigh
    // NULL: The DeltaPos is sign-extended value of lFilePos
    //DeltaPos = (plFilePosHigh != NULL) ? MAKE_OFFSET64(plFilePosHigh[0], lFilePos) : (ULONGLONG)(LONGLONG)lFilePos;
    DeltaPos = (ULONGLONG)(LONGLONG)lFilePos;

    // Get the relative point where to move from
    switch(dwMoveMethod) {
        case FILE_BEGIN:

            // Move relative to the file begin.
            OldPosition = 0;
            break;

        case FILE_CURRENT:

            // Retrieve the current file position
            if(hf->pStream != NULL)
            {
                FileStream_GetPos(hf->pStream, &OldPosition);
            }
            else
            {
                OldPosition = hf->dwFilePos;
            }
            break;

        case FILE_END:

            // Move relative to the end of the file
            OldPosition = FileSize;
            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return SFILE_INVALID_POS;
    }

    // Calculate the new position
    NewPosition = OldPosition + DeltaPos;

    // If moving backward, don't allow the new position go negative
    if ((LONGLONG)DeltaPos < 0) {
        if (NewPosition > FileSize) {// Position is negative
            SetLastError(ERROR_NEGATIVE_SEEK);
            return SFILE_INVALID_POS;
        }
    // If moving forward, don't allow the new position go past the end of the file
    } else {
        if(NewPosition > FileSize)
            NewPosition = FileSize;
    }

    // Now apply the file pointer to the file
    if (hf->pStream != NULL) {
        if (!FileStream_Read(hf->pStream, &NewPosition, NULL, 0))
            return SFILE_INVALID_POS;
    }

    // Also, store the new file position to the TMPQFile struct
    hf->dwFilePos = (DWORD)NewPosition;

    // Return the new file position
    //if(plFilePosHigh != NULL)
    //    *plFilePosHigh = (LONG)(NewPosition >> 32);
    return (DWORD)NewPosition;
}
#endif
