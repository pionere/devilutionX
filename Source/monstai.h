/**
 * @file monstai.h
 *
 * Interface of the AI-functions of monsters.
 */
#ifndef __MONSTAI_H__
#define __MONSTAI_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void MAI_Zombie(int mnum);
void MAI_SkelSd(int mnum);
void MAI_Snake(int mnum);
void MAI_Bat(int mnum);
void MAI_SkelBow(int mnum);
void MAI_Fat(int mnum);
void MAI_Sneak(int mnum);
//void MAI_Fireman(int mnum);
void MAI_Fallen(int mnum);
void MAI_Cleaver(int mnum);
void MAI_GoatMc(int mnum);
void MAI_GoatBow(int mnum);
void MAI_Succ(int mnum);
void MAI_SnowWich(int mnum);
void MAI_HlSpwn(int mnum);
void MAI_SolBrnr(int mnum);
void MAI_AcidUniq(int mnum);
#ifdef HELLFIRE
void MAI_Firebat(int mnum);
void MAI_Torchant(int mnum);
void MAI_Horkdemon(int mnum);
void MAI_Lich(int mnum);
void MAI_ArchLich(int mnum);
void MAI_PsychOrb(int mnum);
void MAI_NecromOrb(int mnum);
void MAI_BoneDemon(int mnum);
#endif
void MAI_Scav(int mnum);
void MAI_Garg(int mnum);
void MAI_Magma(int mnum);
void MAI_Storm(int mnum);
void MAI_Storm2(int mnum);
void MAI_Acid(int mnum);
void MAI_Diablo(int mnum);
void MAI_Mega(int mnum);
void MAI_Golem(int mnum);
void MAI_SkelKing(int mnum);
void MAI_Rhino(int mnum);
void MAI_Counselor(int mnum);
void MAI_Garbud(int mnum);
void MAI_Zhar(int mnum);
void MAI_SnotSpil(int mnum);
void MAI_Lazarus(int mnum);
void MAI_Lazhelp(int mnum);
void MAI_Lachdanan(int mnum);
void MAI_Warlord(int mnum);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __MONSTAI_H__ */
