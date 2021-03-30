/**
 * @file trigs.h
 *
 * Interface of functionality for triggering events when the player enters an area.
 */
#ifndef __TRIGS_H__
#define __TRIGS_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int numtrigs;
extern TriggerStruct trigs[MAXTRIGGERS];
extern int TWarpFrom;

void InitTriggers();
void InitVPEntryTrigger();
void InitVPReturnTrigger();
void CheckTrigForce();
void CheckTriggers();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __TRIGS_H__ */
