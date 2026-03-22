/**
 * @file objdat.h
 *
 * Interface of all object data.
 */
#ifndef __OBJDAT_H__
#define __OBJDAT_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern const int8_t ObjConvTbl[128];
extern const ObjTypeConv objTypeConv[NUM_OBJVERSIONS + 1];
extern const ObjectData objectdata[NUM_OBJECTS];
extern const ObjFileData objfiledata[NUM_OFILE_TYPES];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __OBJDAT_H__ */
