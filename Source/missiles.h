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
extern int missileavail[MAXMISSILES];
extern MissileStruct missile[MAXMISSILES];
extern int nummissiles;
extern BOOL ManashieldFlag;
extern BOOL MissilePreFlag;

void GetDamageAmt(int sn, int *mind, int *maxd);
BOOL CheckBlock(int fx, int fy, int tx, int ty);
int FindClosest(int sx, int sy, int rad);
int GetSpellLevel(int pnum, int sn);
int GetDirection8(int x1, int y1, int x2, int y2);
int GetDirection16(int x1, int y1, int x2, int y2);
void DeleteMissile(int mi, int i);
void GetMissileVel(int mi, int sx, int sy, int dx, int dy, int v);
void PutMissile(int mi);
void GetMissilePos(int mi);
void MoveMissilePos(int mi);
BOOL MonsterTrapHit(int mnum, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift);
BOOL MonsterMHit(int pnum, int mnum, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift);
BOOL PlayerMHit(int pnum, int mnum, int dist, int mind, int maxd, int mitype, BOOLEAN shift, int earflag);
BOOL Plr2PlrMHit(int offp, int defp, int mindam, int maxdam, int dist, int mitype, BOOLEAN shift);
void CheckMissileCol(int mi, int mindam, int maxdam, BOOL shift, int mx, int my, BOOLEAN nodel);
void SetMissAnim(int mi, int animtype);
void SetMissDir(int mi, int dir);
void LoadMissileGFX(BYTE midx);
void InitMissileGFX();
void FreeMissileGFX(int midx);
void FreeMissiles();
void FreeMissiles2();
void InitMissiles();
#ifdef HELLFIRE
void AddHiveExplosion(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
BOOLEAN missiles_found_target(int mi, int *x, int *y, int rad);
void AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddGreatLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddImmolationRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddReflection(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddBerserk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddJester(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddStealPots(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddManaTrap(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddSpecArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddWarp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddLightWall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddHivectrl(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddImmolation(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFireNova(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddLightArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlashfr(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlashbk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddManaRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddMagiRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddElementalRing(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddSearch(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddCboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddHboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
#endif
void AddLArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void GetVileMissPos(int mi, int dx, int dy);
void AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_33(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddLightctrl(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddWeapexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
BOOL CheckIfTrig(int x, int y);
void AddTown(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFiremove(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddChain(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_11(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_12(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_13(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_32(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_1D(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddStone(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddEtherealize(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_1F(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void miss_null_23(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddBoom(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddElement(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddIdentify(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFirewallC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddWave(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddNova(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddBloodboil(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddRepair(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddApoca(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlame(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddFlamec(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddHbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddResurrectBeam(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddBoneSpirit(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddRportal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
void AddDiabApoca(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int id, int dam);
int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, char micaster, int id, int midam, int spllvl);
BOOL Sentfire(int mi, int sx, int sy);
void MI_Dummy(int mi);
void MI_Golem(int mi);
void MI_SetManashield(int mi);
void MI_LArrow(int mi);
void MI_Arrow(int mi);
void MI_Firebolt(int mi);
void MI_Lightball(int mi);
void mi_null_33(int mi);
void MI_Acidpud(int mi);
void MI_Firewall(int mi);
void MI_Fireball(int mi);
#ifdef HELLFIRE
void MI_HorkSpawn(int mi);
void MI_Rune(int mi);
void MI_LightWall(int mi);
void MI_Hivectrl(int mi);
void MI_Immolation(int mi);
void MI_LightArrow(int mi);
void MI_Flashfr(int mi);
void MI_Flashbk(int mi);
void MI_Reflect(int mi);
void MI_FireRing(int mi);
void MI_LightRing(int mi);
void MI_Search(int mi);
void MI_LightningWall(int mi);
void MI_FireNova(int mi);
void MI_SpecArrow(int mi);
#endif
void MI_Lightctrl(int mi);
void MI_Lightning(int mi);
void MI_Town(int mi);
void MI_Flash(int mi);
void MI_Flash2(int mi);
void MI_Manashield(int mi);
void MI_Etherealize(int mi);
void MI_Firemove(int mi);
void MI_Guardian(int mi);
void MI_Chain(int mi);
void mi_null_11(int mi);
void MI_Weapexp(int mi);
void MI_Misexp(int mi);
void MI_Acidsplat(int mi);
void MI_Teleport(int mi);
void MI_Stone(int mi);
void MI_Boom(int mi);
void MI_Rhino(int mi);
void mi_null_32(int mi);
void MI_FirewallC(int mi);
void MI_Infra(int mi);
void MI_Apoca(int mi);
void MI_Wave(int mi);
void MI_Nova(int mi);
void MI_Bloodboil(int mi);
void MI_Flame(int mi);
void MI_Flamec(int mi);
void MI_Cbolt(int mi);
void MI_Hbolt(int mi);
void MI_Element(int mi);
void MI_Bonespirit(int mi);
void MI_ResurrectBeam(int mi);
void MI_Rportal(int mi);
void ProcessMissiles();
void missiles_process_charge();
void ClearMissileSpot(int mi);

/* rdata */

extern int XDirAdd[8];
extern int YDirAdd[8];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISSILES_H__ */
