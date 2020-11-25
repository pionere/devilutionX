/**
 * @file debug.cpp
 *
 * Implementation of debug functions.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef _DEBUG
BOOL update_seed_check = FALSE;

#define DEBUGSEEDS 4096
int seed_index;
int level_seeds[NUMLEVELS + 1];
int seed_table[DEBUGSEEDS];

BYTE *pSquareCel;
char dMonsDbg[NUMLEVELS][MAXDUNX][MAXDUNY];
char dFlagDbg[NUMLEVELS][MAXDUNX][MAXDUNY];

void LoadDebugGFX()
{
	if (visiondebug)
		pSquareCel = LoadFileInMem("Data\\Square.CEL", NULL);
}

void FreeDebugGFX()
{
	MemFreeDbg(pSquareCel);
}
#endif

void CheckDungeonClear()
{
	int i, j;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dMonster[i][j] != 0)
				app_fatal("Monsters not cleared");
			if (dPlayer[i][j] != 0)
				app_fatal("Players not cleared");

#ifdef _DEBUG
			dMonsDbg[currlevel][i][j] = dFlags[i][j] & BFLAG_VISIBLE;
			dFlagDbg[currlevel][i][j] = dFlags[i][j] & BFLAG_POPULATED;
#endif
		}
	}
}

#ifdef _DEBUG
void GiveGoldCheat()
{
	ItemStruct *pi;
	int i;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (plr[myplr].InvGrid[i] == 0) {
			pi = &plr[myplr].InvList[plr[myplr]._pNumInv];
			SetItemData(pi, IDI_GOLD);
			GetItemSeed(pi);
			SetGoldItemValue(pi, GOLD_MAX_LIMIT);
			plr[myplr]._pGold += GOLD_MAX_LIMIT;
			plr[myplr].InvGrid[i] = ++plr[myplr]._pNumInv;
		}
	}
}

void StoresCheat()
{
	int i;

	numpremium = 0;

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		premiumitem[i]._itype = ITYPE_NONE;

	SpawnPremium(30);

	for (i = 0; i < WITCH_ITEMS; i++)
		witchitem[i]._itype = ITYPE_NONE;

	SpawnWitch(30);
}

void TakeGoldCheat()
{
	int i;
	char ig;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		ig = plr[myplr].InvGrid[i];
		if (ig > 0 && plr[myplr].InvList[ig - 1]._itype == ITYPE_GOLD)
			RemoveInvItem(myplr, ig - 1);
	}

	for (i = 0; i < MAXBELTITEMS; i++) {
		if (plr[myplr].SpdList[i]._itype == ITYPE_GOLD)
			plr[myplr].SpdList[i]._itype = ITYPE_NONE;
	}

	plr[myplr]._pGold = 0;
}

void MaxSpellsCheat()
{
	int i;

	for (i = 1; i < MAX_SPELLS; i++) {
		if (spelldata[i].sBookLvl != -1) {
			plr[myplr]._pMemSpells |= (__int64)1 << (i - 1);
			plr[myplr]._pSplLvl[i] = 10;
		}
	}
}

void SetSpellLevelCheat(char spl, int spllvl)
{
	plr[myplr]._pMemSpells |= (__int64)1 << (spl - 1);
	plr[myplr]._pSplLvl[spl] = spllvl;
}

void SetAllSpellsCheat()
{
	SetSpellLevelCheat(SPL_FIREBOLT, 8);
	SetSpellLevelCheat(SPL_CBOLT, 11);
	SetSpellLevelCheat(SPL_HBOLT, 10);
	SetSpellLevelCheat(SPL_HEAL, 7);
	SetSpellLevelCheat(SPL_HEALOTHER, 5);
	SetSpellLevelCheat(SPL_LIGHTNING, 9);
	SetSpellLevelCheat(SPL_FIREWALL, 5);
	SetSpellLevelCheat(SPL_TELEKINESIS, 3);
	SetSpellLevelCheat(SPL_TOWN, 3);
	SetSpellLevelCheat(SPL_FLASH, 3);
	SetSpellLevelCheat(SPL_RNDTELEPORT, 2);
	SetSpellLevelCheat(SPL_MANASHIELD, 2);
	SetSpellLevelCheat(SPL_WAVE, 4);
	SetSpellLevelCheat(SPL_FIREBALL, 3);
	SetSpellLevelCheat(SPL_STONE, 1);
	SetSpellLevelCheat(SPL_CHAIN, 1);
	SetSpellLevelCheat(SPL_GUARDIAN, 4);
	SetSpellLevelCheat(SPL_ELEMENT, 3);
	SetSpellLevelCheat(SPL_NOVA, 1);
	SetSpellLevelCheat(SPL_GOLEM, 2);
	SetSpellLevelCheat(SPL_FLARE, 1);
	SetSpellLevelCheat(SPL_BONESPIRIT, 1);
}

void PrintDebugPlayer(BOOL bNextPlayer)
{
	char dstr[128];

	if (bNextPlayer)
		dbgplr = ((BYTE)dbgplr + 1) & 3;

	snprintf(dstr, sizeof(dstr), "Plr %i : Active = %i", dbgplr, plr[dbgplr].plractive);
	NetSendCmdString(1 << myplr, dstr);

	if (plr[dbgplr].plractive) {
		snprintf(dstr, sizeof(dstr), "  Plr %i is %s", dbgplr, plr[dbgplr]._pName);
		NetSendCmdString(1 << myplr, dstr);
		snprintf(dstr, sizeof(dstr), "  Lvl = %i : Change = %i", plr[dbgplr].plrlevel, plr[dbgplr]._pLvlChanging);
		NetSendCmdString(1 << myplr, dstr);
		snprintf(dstr, sizeof(dstr), "  x = %i, y = %i : tx = %i, ty = %i", plr[dbgplr]._px, plr[dbgplr]._py, plr[dbgplr]._ptargx, plr[dbgplr]._ptargy);
		NetSendCmdString(1 << myplr, dstr);
		snprintf(dstr, sizeof(dstr), "  mode = %i : daction = %i : walk[0] = %i", plr[dbgplr]._pmode, plr[dbgplr].destAction, plr[dbgplr].walkpath[0]);
		NetSendCmdString(1 << myplr, dstr);
		snprintf(dstr, sizeof(dstr), "  inv = %i : hp = %i", plr[dbgplr]._pInvincible, plr[dbgplr]._pHitPoints);
		NetSendCmdString(1 << myplr, dstr);
	}
}

void PrintDebugQuest()
{
	char dstr[128];

	snprintf(dstr, sizeof(dstr), "Quest %i :  Active = %i, Var1 = %i", dbgqst, quests[dbgqst]._qactive, quests[dbgqst]._qvar1);
	NetSendCmdString(1 << myplr, dstr);

	dbgqst++;
	if (dbgqst == MAXQUESTS)
		dbgqst = 0;
}

void PrintDebugMonster(int m)
{
	BOOL bActive;
	int i;
	char dstr[128];

	snprintf(dstr, sizeof(dstr), "Monster %i = %s", m, monster[m].mName);
	NetSendCmdString(1 << myplr, dstr);
	snprintf(dstr, sizeof(dstr), "X = %i, Y = %i", monster[m]._mx, monster[m]._my);
	NetSendCmdString(1 << myplr, dstr);
	snprintf(dstr, sizeof(dstr), "Enemy = %i, HP = %i", monster[m]._menemy, monster[m]._mhitpoints);
	NetSendCmdString(1 << myplr, dstr);
	snprintf(dstr, sizeof(dstr), "Mode = %i, Var1 = %i", monster[m]._mmode, monster[m]._mVar1);
	NetSendCmdString(1 << myplr, dstr);

	bActive = FALSE;

	for (i = 0; i < nummonsters; i++) {
		if (monstactive[i] == m)
			bActive = TRUE;
	}

	snprintf(dstr, sizeof(dstr), "Active List = %i, Squelch = %i", bActive, monster[m]._msquelch);
	NetSendCmdString(1 << myplr, dstr);
}

void GetDebugMonster()
{
	int mi1, mi2;

	mi1 = pcursmonst;
	if (mi1 == -1) {
		mi2 = dMonster[cursmx][cursmy];
		if (mi2 != 0) {
			mi1 = mi2 - 1;
			if (mi2 <= 0)
				mi1 = -1 - mi2;
		} else {
			mi1 = dbgmon;
		}
	}
	PrintDebugMonster(mi1);
}

void NextDebugMonster()
{
	char dstr[128];

	dbgmon++;
	if (dbgmon == MAXMONSTERS)
		dbgmon = 0;

	snprintf(dstr, sizeof(dstr), "Current debug monster = %i", dbgmon);
	NetSendCmdString(1 << myplr, dstr);
}
#endif

DEVILUTION_END_NAMESPACE
