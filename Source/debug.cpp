/**
 * @file debug.cpp
 *
 * Implementation of debug functions.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef _DEBUG

#define DEBUGSEEDS 4096
int seed_index;
int level_seeds[NUM_LEVELS + 1];
int seed_table[DEBUGSEEDS];

BYTE *pSquareCel;
int dbgplr;
int dbgqst;
int dbgmon;
char dMonsDbg[NUM_LEVELS][MAXDUNX][MAXDUNY];
char dFlagDbg[NUM_LEVELS][MAXDUNX][MAXDUNY];

void LoadDebugGFX()
{
	if (visiondebug)
		pSquareCel = LoadFileInMem("Data\\Square.CEL");
}

void FreeDebugGFX()
{
	MemFreeDbg(pSquareCel);
}

void CheckDungeonClear()
{
	int i, j;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dMonster[i][j] != 0)
				app_fatal("Monsters not cleared");
			if (dPlayer[i][j] != 0)
				app_fatal("Players not cleared");

			dMonsDbg[currLvl._dLevelIdx][i][j] = dFlags[i][j] & BFLAG_VISIBLE;
			dFlagDbg[currLvl._dLevelIdx][i][j] = dFlags[i][j] & BFLAG_POPULATED;
		}
	}
}

void GiveGoldCheat()
{
	ItemStruct *pi;
	int i;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (myplr.InvGrid[i] == 0) {
			pi = &myplr.InvList[myplr._pNumInv];
			CreateBaseItem(pi, IDI_GOLD);
			SetGoldItemValue(pi, GOLD_MAX_LIMIT);
			myplr._pGold += GOLD_MAX_LIMIT;
			myplr.InvGrid[i] = ++myplr._pNumInv;
		}
	}
}

void StoresCheat()
{
	int i;
	ItemStruct holditem;

	copy_pod(holditem, items[0]);

	numpremium = 0;
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		premiumitem[i]._itype = ITYPE_NONE;

	SpawnPremium(30);

	for (i = 0; i < WITCH_ITEMS; i++)
		witchitem[i]._itype = ITYPE_NONE;

	SpawnWitch(30);

	copy_pod(items[0], holditem);
}

void TakeGoldCheat()
{
	int i;
	char ig;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		ig = myplr.InvGrid[i];
		if (ig > 0 && myplr.InvList[ig - 1]._itype == ITYPE_GOLD)
			RemoveInvItem(mypnum, ig - 1);
	}

	for (i = 0; i < MAXBELTITEMS; i++) {
		if (myplr.SpdList[i]._itype == ITYPE_GOLD)
			myplr.SpdList[i]._itype = ITYPE_NONE;
	}

	myplr._pGold = 0;
}

void MaxSpellsCheat()
{
	int i;

	for (i = 1; i < NUM_SPELLS; i++) {
		if (spelldata[i].sBookLvl != SPELL_NA) {
			myplr._pMemSkills |= SPELL_MASK(i);
			myplr._pSkillLvl[i] = 10;
		}
	}
}

void SetSpellLevelCheat(char spl, int spllvl)
{
	myplr._pMemSkills |= SPELL_MASK(spl);
	myplr._pSkillLvl[spl] = spllvl;
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
//	SetSpellLevelCheat(SPL_BONESPIRIT, 1);
}

void PrintDebugPlayer(bool bNextPlayer)
{
	if (bNextPlayer)
		dbgplr = ((BYTE)dbgplr + 1) & 3;

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Plr %i : Active = %i", dbgplr, players[dbgplr]._pActive);
	NetSendCmdString(1 << mypnum);

	if (players[dbgplr]._pActive) {
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  Plr %i is %s", dbgplr, players[dbgplr]._pName);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  Lvl = %i : Change = %i", players[dbgplr]._pDunLevel, players[dbgplr]._pLvlChanging);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  x = %i, y = %i : fx = %i, fy = %i", players[dbgplr]._px, players[dbgplr]._py, players[dbgplr]._pfutx, players[dbgplr]._pfuty);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  mode = %i : daction = %i : walk[0] = %i", players[dbgplr]._pmode, players[dbgplr].destAction, players[dbgplr].walkpath[0]);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  inv = %i : hp = %i", players[dbgplr]._pInvincible, players[dbgplr]._pHitPoints);
		NetSendCmdString(1 << mypnum);
	}
}

void PrintDebugQuest()
{
	snprintf(gbNetMsg, sizeof(gbNetMsg), "Quest %i :  Active = %i, Var1 = %i", dbgqst, quests[dbgqst]._qactive, quests[dbgqst]._qvar1);
	NetSendCmdString(1 << mypnum);

	dbgqst++;
	if (dbgqst == NUM_QUESTS)
		dbgqst = 0;
}

void PrintDebugMonster(int m)
{
	bool bActive;
	int i;

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Monster %i = %s", m, monster[m].mName);
	NetSendCmdString(1 << mypnum);
	snprintf(gbNetMsg, sizeof(gbNetMsg), "X = %i, Y = %i", monster[m]._mx, monster[m]._my);
	NetSendCmdString(1 << mypnum);
	snprintf(gbNetMsg, sizeof(gbNetMsg), "Enemy = %i, HP = %i", monster[m]._menemy, monster[m]._mhitpoints);
	NetSendCmdString(1 << mypnum);
	snprintf(gbNetMsg, sizeof(gbNetMsg), "Mode = %i, Var1 = %i", monster[m]._mmode, monster[m]._mVar1);
	NetSendCmdString(1 << mypnum);

	bActive = false;

	for (i = 0; i < nummonsters; i++) {
		if (monstactive[i] == m)
			bActive = true;
	}

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Active List = %i, Squelch = %i", bActive, monster[m]._msquelch);
	NetSendCmdString(1 << mypnum);
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
				mi1 = -(mi2 + 1);
		} else {
			mi1 = dbgmon;
		}
	}
	PrintDebugMonster(mi1);
}

void NextDebugMonster()
{
	dbgmon++;
	if (dbgmon == MAXMONSTERS)
		dbgmon = 0;

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Current debug monster = %i", dbgmon);
	NetSendCmdString(1 << mypnum);
}

void DumpDungeon()
{
	FILE *f0 = fopen("f:\\dundump0.txt", "wb");
	FILE *f1 = fopen("f:\\dundump1.txt", "wb");
	FILE *f2 = fopen("f:\\dundump2.txt", "wb");
	for (int j = 0; j < 48; j++)
		for (int i = 0; i < 48; i++) {
			BYTE v;
			if (i < 40 && j < 40) {
				v = dungeon[i][j];
			} else
				v = 0;
			if (i < 16)
				fputc(v, f0);
			else if (i < 32)
				fputc(v, f1);
			else
				fputc(v, f2);
		}
	fclose(f0);
	fclose(f1);
	fclose(f2);
}

void ValidateData()
{
	// quests
	for (int i = 0; i < lengthof(AllLevels); i++) {
		int j = 0;
		for ( ; j < lengthof(AllLevels[i].dMonTypes); j++) {
			if (AllLevels[i].dMonTypes[j] == MT_INVALID)
				break;
		}
		if (j == lengthof(AllLevels[i].dMonTypes))
			app_fatal("Missing closing MT_INVALID on level %s (%d)", AllLevels[i].dLevelName, i);
	}

	// monsters
	for (int i = 0; i < NUM_MTYPES; i++) {
		const MonsterData& md = monsterdata[i];
		// check RETREAT_DISTANCE for MonFallenFear
		if (md.mAi == AI_FALLEN && md.mInt > 3)
			app_fatal("Invalid mInt %d for %s (%d)", md.mInt, md.mName, i);
		if (md.mAi == AI_COUNSLR && md.mInt > 5)
			app_fatal("Invalid mInt %d for %s (%d)", md.mInt, md.mName, i);
		if (md.mAnimFrames[MA_WALK] > 24) // required by MonWalkDir
			app_fatal("Too many(%d) walk-frames for %s (%d).", md.mAnimFrames[MA_WALK], md.mName, i);
	}

	// umt checks for GetLevelMTypes
#ifdef HELLFIRE
	assert(uniqMonData[UMT_HORKDMN].mtype == MT_HORKDMN);
	assert(uniqMonData[UMT_DEFILER].mtype == MT_DEFILER);
	assert(uniqMonData[UMT_NAKRUL].mtype == MT_NAKRUL);
#endif
	assert(uniqMonData[UMT_BUTCHER].mtype == MT_CLEAVER);
	assert(uniqMonData[UMT_GARBUD].mtype == MT_NGOATMC);
	assert(uniqMonData[UMT_ZHAR].mtype == MT_COUNSLR);
	assert(uniqMonData[UMT_SNOTSPIL].mtype == MT_BFALLSP);
	assert(uniqMonData[UMT_LACHDAN].mtype == MT_RBLACK);
	assert(uniqMonData[UMT_WARLORD].mtype == MT_BTBLACK);
	// umt checks for PlaceQuestMonsters
	assert(uniqMonData[UMT_LAZARUS].mtype == MT_ADVOCATE);
	assert(uniqMonData[UMT_BLACKJADE].mtype == MT_HLSPWN);
	assert(uniqMonData[UMT_RED_VEX].mtype == MT_HLSPWN);

	for (int i = 0; uniqMonData[i].mtype != MT_INVALID; i++) {
		const UniqMonData& um = uniqMonData[i];
		int j = 0;
		int lvl = um.muLevelIdx;
		if (lvl == 0 && um.mQuestId != Q_INVALID)
			app_fatal("Inconsistent unique monster %s (%d). Has a quest, but no quest-level.", um.mName, i);
		if (lvl != 0 && um.mQuestId == Q_INVALID
		 && (lvl != DLV_HELL4 || (um.mtype != MT_ADVOCATE && um.mtype != MT_RBLACK))
#ifdef HELLFIRE
		 && ((lvl != DLV_NEST2 && lvl != DLV_NEST3) || (um.mtype != MT_HORKSPWN))
		 && (lvl != DLV_CRYPT4 || (um.mtype != MT_ARCHLICH))
		 && (lvl != DLV_NEST3 || um.mtype != MT_HORKDMN)
		 && (lvl != DLV_NEST4 || um.mtype != MT_DEFILER)
#endif
		 ) {
			for (j = 0; AllLevels[lvl].dMonTypes[j] != MT_INVALID; j++)
				if (AllLevels[lvl].dMonTypes[j] == um.mtype)
					break;
			if (AllLevels[lvl].dMonTypes[j] == MT_INVALID)
				app_fatal("Useless unique monster %s (%d)", um.mName, i);
		}
		if (um.mAi == AI_FALLEN && um.mint > 3)
			app_fatal("Invalid mInt %d for %s (%d)", um.mint, um.mName, i);
		if (um.mAi == AI_COUNSLR && um.mint > 5)
			app_fatal("Invalid mInt %d for %s (%d)", um.mint, um.mName, i);
	}

	// items
	int minAmu, minLightArmor, minMediumArmor, minHeavyArmor; //, maxStaff = 0;
	minAmu = minLightArmor = minMediumArmor = minHeavyArmor = MAXCHARLEVEL;
	for (int i = 0; i < NUM_IDI; i++) {
		const ItemData& ids = AllItemsList[i];
		if (strlen(ids.iName) > 32 - 1)
			app_fatal("Too long name for %s (%d)", ids.iName, i);
		if (ids.itype == ITYPE_LARMOR && ids.iMinMLvl < minLightArmor)
			minLightArmor = ids.iMinMLvl;
		if (ids.itype == ITYPE_MARMOR && ids.iMinMLvl < minMediumArmor)
			minMediumArmor = ids.iMinMLvl;
		if (ids.itype == ITYPE_HARMOR && ids.iMinMLvl < minHeavyArmor)
			minHeavyArmor = ids.iMinMLvl;
		if (ids.itype == ITYPE_AMULET && ids.iMinMLvl < minAmu)
			minAmu = ids.iMinMLvl;
		//if (ids.itype == ITYPE_STAFF && strlen(ids.iName) > maxStaff)
		//	maxStaff = strlen(ids.iName);
	}
	if (minLightArmor > 1)
		app_fatal("No light armor for OperateArmorStand. Current minimum is level %d", minLightArmor);
	if (minMediumArmor > 10)
		app_fatal("No medium armor for OperateArmorStand. Current minimum is level %d", minMediumArmor);
	if (minHeavyArmor > 24)
		app_fatal("No heavy armor for OperateArmorStand. Current minimum is level %d", minHeavyArmor);
	if (uniqMonData[UMT_HORKDMN].muLevel < minAmu)
		app_fatal("No amulet for THEODORE. Current minimum is level %d, while the monster level is %d.", minAmu, uniqMonData[UMT_HORKDMN].muLevel);
	/*for (const AffixData *pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {

	}
	for (const AffixData *pres = PL_Suffix; pres->PLPower != IPL_INVALID; pres++) {

	}*/

	// spells
	for (int i = 0; i < NUM_SPELLS; i++) {
		const SpellData& sd = spelldata[i];
		if (i != sd.sName)
			app_fatal("Invalid sName %d for %s (%d:%d)", sd.sName, sd.sNameText, i, sd.sName);
		if (sd.sBookLvl != SPELL_NA) {
			if (sd.sBookLvl < BOOK_MIN)
				app_fatal("Invalid sBookLvl %d for %s (%d:%d)", sd.sBookLvl, sd.sNameText, i, sd.sName);
			if (sd.sBookCost <= 0)
				app_fatal("Invalid sBookCost %d for %s (%d:%d)", sd.sBookCost, sd.sNameText, i, sd.sName);
			if (strlen(sd.sNameText) > 32 - (strlen("Book of ") + 1))
				app_fatal("Too long name for %s (%d:%d)", sd.sNameText, i, sd.sName);
		}
		if (sd.sStaffLvl != SPELL_NA) {
			if (sd.sStaffLvl < STAFF_MIN)
				app_fatal("Invalid sStaffLvl %d for %s (%d:%d)", sd.sStaffLvl, sd.sNameText, i, sd.sName);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d:%d)", sd.sStaffCost, sd.sNameText, i, sd.sName);
			//if (strlen(sd.sNameText) > 32 - (maxStaff + 4 + 1))
			if (strlen(sd.sNameText) > 32 - (strlen("Staff of ") + 1))
				app_fatal("Too long name for %s (%d:%d)", sd.sNameText, i, sd.sName);
		}
		if (sd.sScrollLvl != SPELL_NA) {
			if (sd.sScrollLvl < SCRL_MIN)
				app_fatal("Invalid sScrollLvl %d for %s (%d:%d)", sd.sScrollLvl, sd.sNameText, i, sd.sName);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d:%d)", sd.sStaffCost, sd.sNameText, i, sd.sName);
			if (strlen(sd.sNameText) > 32 - (strlen("Scroll of ") + 1))
				app_fatal("Too long name for %s (%d:%d)", sd.sNameText, i, sd.sName);
		}
	}

	// missiles
	for (int i = 0; i < NUM_MISTYPES; i++) {
		const MissileData &md = missiledata[i];
		if (md.mName != i)
			app_fatal("Invalid mName %d for missile %d.", md.mName, i);
	}
}
#endif

DEVILUTION_END_NAMESPACE
