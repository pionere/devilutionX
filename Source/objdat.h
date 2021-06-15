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

extern const BYTE ObjConvTbl[128];
extern const ObjectData AllObjects[NUM_OBJECTS];
extern const char *const ObjMasterLoadList[NUM_OFILE_TYPES];
#ifdef HELLFIRE
extern const char *const ObjCryptLoadList[NUM_OFILE_TYPES];
extern const char *const ObjNestLoadList[NUM_OFILE_TYPES];
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __OBJDAT_H__ */
