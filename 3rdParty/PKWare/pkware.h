/*****************************************************************************/
/* pkware.h                               Copyright (c) Ladislav Zezula 2003 */
/*---------------------------------------------------------------------------*/
/* Header file for PKWARE Data Compression Library                           */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 31.03.03  1.00  Lad  The first version of pkware.h                        */
/*****************************************************************************/

#ifndef __PKWARE_H__
#define __PKWARE_H__

//-----------------------------------------------------------------------------
// Defines

#define CMP_BINARY             0            // Binary compression
#define CMP_ASCII              1            // Ascii compression

#define CMP_NO_ERROR           0
#define CMP_INVALID_DICTSIZE   1
#define CMP_INVALID_MODE       2
#define CMP_BAD_DATA           3
#define CMP_ABORT              4

#define CMP_IMPLODE_DICT_SIZE1   1024       // Dictionary size of 1024
#define CMP_IMPLODE_DICT_SIZE2   2048       // Dictionary size of 2048
#define CMP_IMPLODE_DICT_SIZE3   4096       // Dictionary size of 4096

//-----------------------------------------------------------------------------
// Define calling convention

#ifndef PKWAREAPI
#ifdef WIN32
#define PKWAREAPI  __cdecl                   // Use for normal __cdecl calling 
#else
#define PKWAREAPI
#endif
#endif

//-----------------------------------------------------------------------------
// Internal structures

// Compression structure
typedef struct TCmpStruct
{
    unsigned int   distance;                // 0000: Backward distance of the currently found repetition, decreased by 1
    unsigned int   out_bytes;               // 0004: # bytes available in out_buff            
    unsigned int   out_bits;                // 0008: # of bits available in the last out byte
#ifdef FULL
    unsigned int   dsize_bits;              // 000C: Number of bits needed for dictionary size. 4 = 0x400, 5 = 0x800, 6 = 0x1000
    unsigned int   dsize_mask;              // 0010: Bit mask for dictionary. 0x0F = 0x400, 0x1F = 0x800, 0x3F = 0x1000
    unsigned int   ctype;                   // 0014: Compression type (CMP_ASCII or CMP_BINARY)
    unsigned int   dsize_bytes;             // 0018: Dictionary size in bytes
    unsigned char  dist_bits[0x40];         // 001C: Distance bits
    unsigned char  dist_codes[0x40];        // 005C: Distance codes
    unsigned char  nChBits[0x306];          // 009C: Table of literal bit lengths to be put to the output stream
    unsigned short nChCodes[0x306];         // 03A2: Table of literal codes to be put to the output stream
#else
    static const unsigned int dsize_bits = 4 + 2;  // 000C: Number of bits needed for dictionary size. 4 = 0x400, 5 = 0x800, 6 = 0x1000
    static const unsigned int dsize_mask = 0x0F | 0x20 | 0x10; // 0010: Bit mask for dictionary. 0x0F = 0x400, 0x1F = 0x800, 0x3F = 0x1000
    static const unsigned int ctype = CMP_BINARY;  // 0014: Compression type (CMP_ASCII or CMP_BINARY)
    static const unsigned int dsize_bytes = CMP_IMPLODE_DICT_SIZE3; // 0018: Dictionary size in bytes
    unsigned char  nChBits[0x206];          // 009C: Table of literal bit lengths to be put to the output stream
    unsigned short nChCodes[0x206];         // 03A2: Table of literal codes to be put to the output stream
#endif
#ifdef FULL
    unsigned short offs09AE;                // 09AE: 
#endif
    void         * param;                   // 09B0: User parameter
#ifdef FULL
    unsigned int (PKWAREAPI *read_buf)(char *buf, unsigned int *size, void *param);  // 9B4
    void         (PKWAREAPI *write_buf)(char *buf, unsigned int *size, void *param); // 9B8
#else
    unsigned int (PKWAREAPI *read_buf)(char *buf, unsigned int size, void *param);  // 9B4
    void         (PKWAREAPI *write_buf)(char *buf, unsigned int size, void *param); // 9B8
#endif
    unsigned short offs09BC[0x204];         // 09BC:
#ifdef FULL
    unsigned long  offs0DC4;                // 0DC4: 
#endif
    unsigned short phash_to_index[0x900];   // 0DC8: Array of indexes (one for each PAIR_HASH) to the "pair_hash_offsets" table
#ifdef FULL
    unsigned short phash_to_index_end;      // 1FC8: End marker for "phash_to_index" table
#endif
    char           out_buff[0x802];         // 1FCA: Compressed data
    unsigned char  work_buff[0x2204];       // 27CC: Work buffer
                                            //  + DICT_OFFSET  => Dictionary
                                            //  + UNCMP_OFFSET => Uncompressed data
    unsigned short phash_offs[0x2204];      // 49D0: Table of offsets for each PAIR_HASH
} TCmpStruct;

#define CMP_BUFFER_SIZE  sizeof(TCmpStruct) // Size of compression structure.
                                            // Defined as 36312 in pkware header file


// Decompression structure
typedef struct
{
#ifdef FULL
    unsigned long offs0000;                 // 0000
#endif
    unsigned long ctype;                    // 0004: Compression type (CMP_BINARY or CMP_ASCII)
    unsigned long outputPos;                // 0008: Position in output buffer
    unsigned long dsize_bits;               // 000C: Dict size (4, 5, 6 for 0x400, 0x800, 0x1000)
    unsigned long dsize_mask;               // 0010: Dict size bitmask (0x0F, 0x1F, 0x3F for 0x400, 0x800, 0x1000)
    unsigned long bit_buff;                 // 0014: 16-bit buffer for processing input data
    unsigned long extra_bits;               // 0018: Number of extra (above 8) bits in bit buffer
    unsigned int  in_pos;                   // 001C: Position in in_buff
    unsigned long in_bytes;                 // 0020: Number of bytes in input buffer
    void        * param;                    // 0024: Custom parameter
#ifdef FULL
    unsigned int (PKWAREAPI *read_buf)(char *buf, unsigned int *size, void *param); // Pointer to function that reads data from the input stream
    void         (PKWAREAPI *write_buf)(char *buf, unsigned int *size, void *param);// Pointer to function that writes data to the output stream
#else
    unsigned int (PKWAREAPI *read_buf)(char *buf, unsigned int size, void *param); // Pointer to function that reads data from the input stream
    void         (PKWAREAPI *write_buf)(char *buf, unsigned int size, void *param);// Pointer to function that writes data to the output stream
#endif

    unsigned char out_buff[0x2204];         // 0030: Output circle buffer.
                                            //       0x0000 - 0x0FFF: Previous uncompressed data, kept for repetitions
                                            //       0x1000 - 0x1FFF: Currently decompressed data
                                            //       0x2000 - 0x2203: Reserve space for the longest possible repetition
    unsigned char in_buff[0x800];           // 2234: Buffer for data to be decompressed
    unsigned char DistPosCodes[0x100];      // 2A34: Table of distance position codes
    unsigned char LengthCodes[0x100];       // 2B34: Table of length codes
#ifdef FULL
    unsigned char offs2C34[0x100];          // 2C34: Buffer for 
    unsigned char offs2D34[0x100];          // 2D34: Buffer for 
    unsigned char offs2E34[0x80];           // 2EB4: Buffer for 
    unsigned char offs2EB4[0x100];          // 2EB4: Buffer for 
    unsigned char ChBitsAsc[0x100];         // 2FB4: Buffer for 
    unsigned char DistBits[0x40];           // 30B4: Numbers of bytes to skip copied block length
    unsigned char LenBits[0x10];            // 30F4: Numbers of bits for skip copied block length
    unsigned char ExLenBits[0x10];          // 3104: Number of valid bits for copied block
    unsigned short LenBase[0x10];           // 3114: Buffer for 
#endif
} TDcmpStruct;

#define EXP_BUFFER_SIZE sizeof(TDcmpStruct) // Size of decompression structure
                                            // Defined as 12596 in pkware headers
#ifndef FULL
typedef struct TDataInfo {
    /*unsigned char * const pbInBuff;
    size_t srcOffset;
    unsigned char * const pbOutBuff;
    size_t destOffset;
    const size_t size;
    TDataInfo(unsigned char * src, unsigned char * dst, size_t s) : pbInBuff(src), srcOffset(0), pbOutBuff(dst), destOffset(0), size(s) {*/
    unsigned char * pbInBuff;
    unsigned char * const pbInBuffEnd;
    unsigned char * pbOutBuff;
    unsigned char * const pbOutBuffEnd;
    TDataInfo(unsigned char * src, size_t inSize, unsigned char * dst, size_t outSize) : pbInBuff(src), pbInBuffEnd(src + inSize), pbOutBuff(dst), pbOutBuffEnd(dst + outSize) {
    }
} TDataInfo;
#endif // !FULL
//-----------------------------------------------------------------------------
// Tables (in explode.c)

extern const unsigned char DistBits[0x40];
extern const unsigned char DistCode[0x40];
extern const unsigned char ExLenBits[0x10];
// extern const unsigned short LenBase[0x10];
extern const unsigned char LenBits[0x10];
extern const unsigned char LenCode[0x10];
#ifdef FULL
extern const unsigned char ChBitsAsc[0x100];
extern const unsigned short ChCodeAsc[0x100];
#endif
//-----------------------------------------------------------------------------
// Public functions

#ifdef __cplusplus
   extern "C" {
#endif

unsigned int PKWAREAPI implode(
#if FULL
   unsigned int (*read_buf)(char *buf, unsigned int *size, void *param),
   void         (*write_buf)(char *buf, unsigned int *size, void *param),
#else
   unsigned int (*read_buf)(char *buf, unsigned int size, void *param),
   void         (*write_buf)(char *buf, unsigned int size, void *param),
#endif
   char         *work_buf,
#if FULL
   void         *param,
   unsigned int *type,
   unsigned int *dsize);
#else
   void         *param);
#endif


unsigned int PKWAREAPI explode(
#if FULL
   unsigned int (*read_buf)(char *buf, unsigned  int *size, void *param),
   void         (*write_buf)(char *buf, unsigned  int *size, void *param),
#else
   unsigned int (*read_buf)(char *buf, unsigned  int size, void *param),
   void         (*write_buf)(char *buf, unsigned  int size, void *param),
#endif
   char         *work_buf,
   void         *param);

#ifndef FULL
unsigned int PkwareBufferRead(char* buf, unsigned int size, void* param);
void PkwareBufferWrite(char* buf, unsigned int size, void* param);
#endif

#ifdef __cplusplus
   }                         // End of 'extern "C"' declaration
#endif

#endif // __PKWARE_H__
