/**
 * @file missiles.h
 *
 * Interface of missile functionality.
 */
#ifndef __MISSILES_H__
#define __MISSILES_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern int missileactive[MAXMISSILES];
extern MissileStruct missile[MAXMISSILES];
extern int nummissiles;

void GetDamageAmt(int sn, int sl, int *mind, int *maxd);
unsigned CalcMonsterDam(uint16_t mor, BYTE mRes, unsigned mindam, unsigned maxdam, bool penetrates);
unsigned CalcPlrDam(int pnum, BYTE mRes, unsigned mindam, unsigned maxdam);
int CheckMonCol(int _mnum_);
int CheckPlrCol(int _pnum_);
void AddElementalExplosion(int dx, int dy, int fdam, int ldam, int mdam, int hdam);
int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, int micaster, int misource, int spllvl);
void DeleteMissile(int mi, int idx);
void LoadMissileGFX(BYTE midx);
void InitMissileGFX();
void FreeMissiles();
void FreeMissiles2();
void InitMissiles();
void ProcessMissiles();
void SyncMissilesAnim();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISSILES_H__ */
