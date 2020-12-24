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
int GetSpellLevel(int pnum, int sn);
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
void AddHiveexpC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddGreatLightRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddImmolationRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddReflection(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddBerserk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddHorkSpawn(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddJester(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddStealPots(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddManaTrap(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddSpecArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddWarp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddHiveexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireball3(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddLightArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlashfr(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlashbk(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddManaRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddMagiRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddElementalRing(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddSearch(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddCboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddHboltArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
#endif
void AddLArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddWeapFExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddWeapLExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddTown(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlash2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddChain(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void miss_null_11(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void miss_null_12(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void miss_null_13(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireman(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void miss_null_1D(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddStone(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddEtherealize(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void miss_null_1F(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void miss_null_23(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddApocaExp(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddElement(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddIdentify(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFirewallC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddNovaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddBloodboil(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddRepair(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddRecharge(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddApocaC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlame(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddFlameC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddCboltC(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddHbolt(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddResurrectBeam(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddBoneSpirit(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddRportal(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
void AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, char micaster, int misource, int spllvl);
int AddMissile(int sx, int sy, int dx, int dy, int midir, int mitype, char micaster, int misource, int midam, int spllvl);
void MI_Dummy(int mi);
void MI_Golem(int mi);
void MI_SetManashield(int mi);
void MI_LArrow(int mi);
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
void MI_LightArrow(int mi);
void MI_Flashfr(int mi);
void MI_Flashbk(int mi);
void MI_Reflect(int mi);
void MI_FireRing(int mi);
void MI_LightRing(int mi);
void MI_Search(int mi);
void MI_LightwallC(int mi);
void MI_FireNovaC(int mi);
void MI_SpecArrow(int mi);
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
void mi_null_11(int mi);
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
void MI_Bonespirit(int mi);
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
