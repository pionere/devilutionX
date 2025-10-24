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

void GetSkillDetails(int sn, int sl, SkillDetails* skd);
unsigned CalcMonsterDam(unsigned mor, BYTE mRes, unsigned mindam, unsigned maxdam, bool penetrates);
unsigned CalcPlrDam(int pnum, BYTE mRes, unsigned mindam, unsigned maxdam);
int CheckMonCol(int _mnum_);
int CheckPlrCol(int _pnum_);
int AddElementalExplosion(int fdam, int ldam, int mdam, int hdam, bool isMonster, int mpnum);
int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, int micaster, int misource, int spllvl);
void RemovePortalMissile(int pnum);
void LoadMissileGFX(BYTE midx);
void InitGameMissileGFX();
void FreeGameMissileGFX();
void FreeMonMissileGFX();
void InitMissiles();
void ProcessMissiles();
void SyncMissilesAnim();

inline bool CheckHit(int hitper)
{
	if (hitper > 75) {
		hitper = 75 + ((hitper - 75) >> 2);
	} else if (hitper < 25) {
		hitper = 25 + ((hitper - 25) >> 2);
	}
	return random_(0, 100) < hitper;
}

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISSILES_H__ */
