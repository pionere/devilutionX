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

extern const int ObjTypeConv[];
extern const ObjDataStruct AllObjects[];
extern const char *const ObjMasterLoadList[];
#ifdef HELLFIRE
extern const char *const ObjCryptLoadList[];
extern const char *const ObjHiveLoadList[];
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __OBJDAT_H__ */
