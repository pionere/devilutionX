/**
 * @file misproc.h
 *
 * Interface of missile-processing functions.
 */
#ifndef __MISPROC_H__
#define __MISPROC_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HELLFIRE
int AddFireRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddLightRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddNovaRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddWaveRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddStoneRune(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddLightwall(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddFireexp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddRingC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddFireball2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
#endif
int AddDone(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddArrow(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddFirebolt(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddMage(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddMagmaball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddLightball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddPoison(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddWind(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddAcid(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddAcidpud(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddKrull(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddTeleport(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddRndTeleport(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddFirewall(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddFireball(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddLightningC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddLightning(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddBloodBoilC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddBloodBoil(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddBleed(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddMisexp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddFlash(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddFireWave(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddMeteor(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddChain(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddRhino(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddCharge(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddFireman(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddFlare(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddStone(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddShroud(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddGuardian(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddGolem(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddHeal(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddHealOther(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddElemental(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddWallC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddFireWaveC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddNovaC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddOpItem(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddDisarm(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddInferno(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddInfernoC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
//int AddFireTrap(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddBarrelExp(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddCboltC(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddCbolt(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddResurrect(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddAttract(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddTelekinesis(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddTown(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddPortal(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddApocaC2(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddManashield(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddInfra(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddRage(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddPulse(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
int AddCallToArms(int mi, int sx, int sy, int dx, int dy, int midir, int micaster, int misource, int spllvl);
void MI_Dummy(int mi);
void MI_Arrow(int mi);
void MI_AsArrow(int mi);
void MI_Firebolt(int mi);
void MI_Lightball(int mi);
void MI_Poison(int mi);
void MI_Mage(int mi);
void MI_Wind(int mi);
//void MI_Krull(int mi);
void MI_Acid(int mi);
void MI_Acidpud(int mi);
void MI_Firewall(int mi);
//void MI_Fireball(int mi);
#ifdef HELLFIRE
void MI_HorkSpawn(int mi);
void MI_Rune(int mi);
//void MI_Lightwall(int mi);
#endif
void MI_LightningC(int mi);
void MI_Lightning(int mi);
void MI_BloodBoilC(int mi);
void MI_BloodBoil(int mi);
void MI_SwampC(int mi);
void MI_Bleed(int mi);
void MI_Portal(int mi);
void MI_Flash(int mi);
void MI_Flash2(int mi);
void MI_FireWave(int mi);
void MI_Meteor(int mi);
void MI_Guardian(int mi);
void MI_Chain(int mi);
void MI_Misexp(int mi);
void MI_MiniExp(int mi);
void MI_LongExp(int mi);
void MI_ExtExp(int mi);
void MI_Acidsplat(int mi);
void MI_Stone(int mi);
void MI_Shroud(int mi);
void MI_Rhino(int mi);
void MI_Charge(int mi);
//void MI_Fireman(int mi);
void MI_WallC(int mi);
void MI_ApocaC(int mi);
void MI_Inferno(int mi);
void MI_InfernoC(int mi);
//void MI_FireTrap(int mi);
void MI_Cbolt(int mi);
void MI_Elemental(int mi);
void MI_Pulse(int mi);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MISPROC_H__ */
