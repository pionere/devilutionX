/**
 * @file portal.h
 *
 * Interface of functionality for handling town portals.
 */
#ifndef __PORTAL_H__
#define __PORTAL_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern PortalStruct portals[MAXPORTAL];

void InitPortals();
//void SetPortalStats(int i, bool o, int x, int y, int lvl);
void AddWarpMissile(int i, int x, int y);
void SyncPortals();
void AddInTownPortal(int i);
void ActivatePortal(int i, int x, int y, int lvl);
void DeactivatePortal(int i);
void RemovePortalMissile(int i);
void UseCurrentPortal(int i);
void GetPortalLvlPos();
bool PosOkPortal(int x, int y);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PORTAL_H__ */
