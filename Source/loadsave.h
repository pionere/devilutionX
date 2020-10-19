/**
 * @file loadsave.h
 *
 * Interface of save game functionality.
 */
#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern BYTE *tbuff;

void LoadGame(BOOL firstflag);
char BLoad();
int WLoad();
int ILoad();
BOOL OLoad();
void LoadItems(const int n, ItemStruct *pItem);
void LoadItemData(ItemStruct *is);
void SaveItem(ItemStruct *is);
void SaveItems(ItemStruct *pItem, const int n);

void LoadPlayer(int pnum);
void LoadMonster(int mnum);
void LoadMissile(int mi);
void LoadObject(int oi);
void LoadItem(int ii);
void LoadPremium(int i);
void LoadQuest(int i);
void LoadLighting(int lnum);
void LoadVision(int vnum);
void LoadPortal(int i);
void SaveGame();
void BSave(char v);
void WSave(int v);
void ISave(int v);
void OSave(BOOL v);
void SavePlayer(int pnum);
void SaveMonster(int mnum);
void SaveMissile(int mi);
void SaveObject(int oi);
void SavePremium(int i);
void SaveQuest(int i);
void SaveLighting(int lnum);
void SaveVision(int vnum);
void SavePortal(int i);
void SaveLevel();
void LoadLevel();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __LOADSAVE_H__ */
