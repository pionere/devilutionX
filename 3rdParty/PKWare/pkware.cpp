/*****************************************************************************/
/* pkware.cpp                             Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Sample functions of PKWARE Data Compression library                       */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 11.14.23  1.00  Lad  Created                                              */
/*****************************************************************************/

#include <string.h>

#include "pkware.h"

// Callback function loads data from the input buffer.
// Returns number of bytes loaded
//
//   char * buf          - Pointer to a buffer where to store loaded data
//   unsigned int * size - Max. number of bytes to read
//   void * param        - Custom pointer, parameter of implode/explode
#ifdef FULL
unsigned int PKWAREAPI PkwareBufferRead(char* buf, unsigned int* size, void* param)
#else
unsigned int PKWAREAPI PkwareBufferRead(char* buf, unsigned int size, void* param)
#endif
{
    /*TDataInfo* pInfo;
    size_t sSize;

    pInfo = (TDataInfo*)param;

    sSize = pInfo->size - pInfo->srcOffset;
    if (*size < sSize)
        sSize = *size;

    memcpy(buf, (const char *)pInfo->pbInBuff + pInfo->srcOffset, sSize);
    pInfo->srcOffset += sSize;

    return sSize;*/
    TDataInfo * pInfo = (TDataInfo *)param;
    unsigned int nMaxAvail = (unsigned int)(pInfo->pbInBuffEnd - pInfo->pbInBuff);
#ifdef FULL
    unsigned int nToRead = *size;
#else
    unsigned int nToRead = size;
#endif

    // Check the case when not enough data available
    if(nToRead > nMaxAvail)
        nToRead = nMaxAvail;

    // Load data and increment offsets
    memcpy(buf, pInfo->pbInBuff, nToRead);
    pInfo->pbInBuff += nToRead;
    // assert(pInfo->pbInBuff <= pInfo->pbInBuffEnd);
    return nToRead;
}

// Callback function for store output data.
//
//   char * buf          - Pointer to data to be written
//   unsigned int * size - Number of bytes to write
//   void * param        - Custom pointer, parameter of implode/explode
#ifdef FULL
void PKWAREAPI PkwareBufferWrite(char* buf, unsigned int* size, void* param)
#else
void PKWAREAPI PkwareBufferWrite(char* buf, unsigned int size, void* param)
#endif
{
    /*TDataInfo* pInfo;
    size_t sSize;

    pInfo = (TDataInfo*)param;

    sSize = *size;
    memcpy(pInfo->pbOutBuff + pInfo->destOffset, buf, sSize);
    pInfo->destOffset += sSize;*/
    TDataInfo * pInfo = (TDataInfo *)param;
    unsigned int nMaxWrite = (unsigned int)(pInfo->pbOutBuffEnd - pInfo->pbOutBuff);
#ifdef FULL
    unsigned int nToWrite = *size;
#else
    unsigned int nToWrite = size;
#endif

    // Check the case when not enough space in the output buffer
    if(nToWrite > nMaxWrite)
        nToWrite = nMaxWrite;

    // Write output data and increments offsets
    memcpy(pInfo->pbOutBuff, buf, nToWrite);
    pInfo->pbOutBuff += nToWrite;
    // assert(pInfo->pbOutBuff <= pInfo->pbOutBuffEnd);
}
