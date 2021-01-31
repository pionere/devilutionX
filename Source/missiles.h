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
extern BOOL MissilePreFlag;

void GetDamageAmt(int sn, int *mind, int *maxd);
unsigned CalcMonsterDam(unsigned short mor, BYTE mRes, unsigned mindam, unsigned maxdam);
int CalcPlrDam(PlayerStruct *p, BYTE mRes, unsigned mindam, unsigned maxdam);
void AddElementalExplosion(int dx, int dy, int fdam, int ldam, int mdam, int hdam);
void DeleteMissile(int mi, int idx);
BOOL MonsterTrapHit(int mnum, int mindam, int maxdam, int dist, int mitype, BOOL shift);
BOOL PlayerTrapHit(int pnum, int mind, int maxd, int dist, int mitype, BOOL shift);
void SetMissAnim(int mi, int animtype);
void SetMissDir(int mi, int dir);
void LoadMissileGFX(BYTE midx);
void InitMissileGFX();
void FreeMissiles();
void FreeMissiles2();
void InitMissiles();
#ifdef HELLFIRE
int AddHiveexpC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddGreatLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddImmolationRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddHiveexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireball3(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddElementalRing(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddWhittle(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
#endif
//int AddLArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddWeapFExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddWeapLExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddTown(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddChain(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireman(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddStone(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddEtherealize(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddApocaExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddElement(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddIdentify(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFirewallC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddNovaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddBloodboil(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddRepair(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFlame(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddFlameC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddCboltC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddHbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddResurrectBeam(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddRportal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, char micaster, int misource, int midam, int spllvl);
void MI_Dummy(int mi);
void MI_Golem(int mi);
//void MI_LArrow(int mi);
void MI_Arrow(int mi);
void MI_Firebolt(int mi);
void MI_Lightball(int mi);
void MI_Krull(int mi);
void MI_Acidpud(int mi);
void MI_Firewall(int mi);
void MI_Fireball(int mi);
#ifdef HELLFIRE
void MI_HorkSpawn(int mi);
void MI_Rune(int mi);
void MI_Lightwall(int mi);
void MI_Hiveexp(int mi);
void MI_Fireball3(int mi);
void MI_FireRing(int mi);
void MI_LightRing(int mi);
void MI_LightwallC(int mi);
void MI_FireNovaC(int mi);
#endif
void MI_LightningC(int mi);
void MI_Lightning(int mi);
void MI_Town(int mi);
void MI_Flash(int mi);
void MI_Flash2(int mi);
void MI_Etherealize(int mi);
void MI_FireWave(int mi);
void MI_Guardian(int mi);
void MI_Chain(int mi);
void MI_WeapExp(int mi);
void MI_Misexp(int mi);
void MI_Acidsplat(int mi);
void MI_Teleport(int mi);
void MI_Stone(int mi);
void MI_ApocaExp(int mi);
void MI_Rhino(int mi);
void MI_Fireman(int mi);
void MI_FirewallC(int mi);
void MI_Infra(int mi);
void MI_ApocaC(int mi);
void MI_FireWaveC(int mi);
void MI_LightNovaC(int mi);
void MI_Bloodboil(int mi);
void MI_Flame(int mi);
void MI_FlameC(int mi);
void MI_Cbolt(int mi);
void MI_Hbolt(int mi);
void MI_Element(int mi);
void MI_EleExp(int mi);
void MI_ResurrectBeam(int mi);
void MI_Rportal(int mi);
void ProcessMissiles();
void missiles_process_charge();
void ClearMissileSpot(int mi);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISSILES_H__ */
