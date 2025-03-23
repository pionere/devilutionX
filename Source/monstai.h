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
void MAI_Round(int mnum);
void MAI_Ranged(int mnum);
#ifdef HELLFIRE
void MAI_Horkdemon(int mnum);
#endif
void MAI_Scav(int mnum);
void MAI_Garg(int mnum);
void MAI_RoundRanged(int mnum);
void MAI_RoundRanged2(int mnum);
void MAI_Golem(int mnum);
void MAI_SkelKing(int mnum);
void MAI_Rhino(int mnum);
void MAI_Counselor(int mnum);
void MAI_Mage(int mnum);
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
