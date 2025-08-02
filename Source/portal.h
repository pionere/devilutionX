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
//void SetPortalStats(int pidx, bool o, int x, int y, int lvl);
void SyncPortals();
void AddInTownPortal(int pidx);
void ActivatePortal(int pidx, int x, int y, int lvl);
void DeactivatePortal(int pidx);
void UseCurrentPortal(int pidx);
void GetPortalLvlPos();
bool PosOkPortal(int x, int y);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __PORTAL_H__ */
