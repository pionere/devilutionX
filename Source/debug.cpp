/**
 * @file debug.cpp
 *
 * Implementation of debug functions.
 */
#include <chrono>
#include "all.h"
#include "misproc.h"

DEVILUTION_BEGIN_NAMESPACE

#if DEBUG_MODE

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

			dMonsDbg[currLvl._dLevelIdx][i][j] = dFlags[i][j] & BFLAG_ALERT;
			dFlagDbg[currLvl._dLevelIdx][i][j] = dFlags[i][j] & BFLAG_POPULATED;
		}
	}
}

void GiveGoldCheat()
{
	ItemStruct *pi;
	int i;

	pi = myplr._pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_itype == ITYPE_NONE) {
			CreateBaseItem(pi, IDI_GOLD);
			SetGoldItemValue(pi, GOLD_MAX_LIMIT);
			myplr._pGold += GOLD_MAX_LIMIT;
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
		premiumitems[i]._itype = ITYPE_NONE;

	SpawnPremium(30);

	for (i = 0; i < WITCH_ITEMS; i++)
		witchitem[i]._itype = ITYPE_NONE;

	SpawnWitch(30);

	copy_pod(items[0], holditem);
}

void TakeGoldCheat()
{
	ItemStruct* pi;
	int i;

	pi = myplr._pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_itype == ITYPE_GOLD)
			pi->_itype = ITYPE_NONE;
	}

	myplr._pGold = 0;
}

void MaxSpellsCheat()
{
	int i;

	for (i = 0; i < NUM_SPELLS; i++) {
		if (spelldata[i].sBookLvl != SPELL_NA) {
			myplr._pMemSkills |= SPELL_MASK(i);
			myplr._pSkillLvlBase[i] = MAXSPLLEVEL;
		}
	}
	CalcPlrItemVals(mypnum, false);
}

static void SetSpellLevelCheat(char spl, int spllvl)
{
	myplr._pMemSkills |= SPELL_MASK(spl);
	myplr._pSkillLvlBase[spl] = spllvl;
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
	SetSpellLevelCheat(SPL_ELEMENTAL, 3);
	SetSpellLevelCheat(SPL_NOVA, 1);
	SetSpellLevelCheat(SPL_GOLEM, 2);
	SetSpellLevelCheat(SPL_FLARE, 1);
//	SetSpellLevelCheat(SPL_BONESPIRIT, 1);
	CalcPlrItemVals(mypnum, false);
}

void PrintDebugPlayer(bool bNextPlayer)
{
	if (bNextPlayer)
		dbgplr = ((BYTE)dbgplr + 1) & 3;

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Plr %d : Active = %d", dbgplr, players[dbgplr]._pActive);
	NetSendCmdString(1 << mypnum);

	if (players[dbgplr]._pActive) {
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  Plr %d is %s", dbgplr, players[dbgplr]._pName);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  Lvl = %d : Change = %d", players[dbgplr]._pDunLevel, players[dbgplr]._pLvlChanging);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  x = %d, y = %d : fx = %d, fy = %d", players[dbgplr]._px, players[dbgplr]._py, players[dbgplr]._pfutx, players[dbgplr]._pfuty);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  mode = %d : daction = %d : walk[0] = %d", players[dbgplr]._pmode, players[dbgplr].destAction, players[dbgplr].walkpath[0]);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "  inv = %d : hp = %d", players[dbgplr]._pInvincible, players[dbgplr]._pHitPoints);
		NetSendCmdString(1 << mypnum);
	}
}

void PrintDebugQuest()
{
	snprintf(gbNetMsg, sizeof(gbNetMsg), "Quest %d :  Active = %d, Var1 = %d", dbgqst, quests[dbgqst]._qactive, quests[dbgqst]._qvar1);
	NetSendCmdString(1 << mypnum);

	dbgqst++;
	if (dbgqst == NUM_QUESTS)
		dbgqst = 0;
}

void PrintDebugMonster(int m)
{
	bool bActive;
	int i;

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Monster %d = %s", m, monsters[m]._mName);
	NetSendCmdString(1 << mypnum);
	snprintf(gbNetMsg, sizeof(gbNetMsg), "X = %d, Y = %d", monsters[m]._mx, monsters[m]._my);
	NetSendCmdString(1 << mypnum);
	snprintf(gbNetMsg, sizeof(gbNetMsg), "Enemy = %d, HP = %d", monsters[m]._menemy, monsters[m]._mhitpoints);
	NetSendCmdString(1 << mypnum);
	snprintf(gbNetMsg, sizeof(gbNetMsg), "Mode = %d, Var1 = %d", monsters[m]._mmode, monsters[m]._mVar1);
	NetSendCmdString(1 << mypnum);

	bActive = monsters[m]._mmode <= MM_INGAME_LAST;

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Active List = %d, Squelch = %d", bActive, monsters[m]._msquelch);
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

	snprintf(gbNetMsg, sizeof(gbNetMsg), "Current debug monster = %d", dbgmon);
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
#endif /* DEBUG_MODE */

#if DEBUG_MODE || DEV_MODE
void ValidateData()
{
	int i;

	// text
	if (GetHugeStringWidth("Pause") != 135)
		app_fatal("gmenu_draw_pause expects hardcoded width 135.");

	// cursors
	for (i = 0; i < lengthof(InvItemWidth); i++) {
		if (i != CURSOR_NONE && InvItemWidth[i] == 0)
			app_fatal("Invalid (zero) cursor width at %d.", i);
	}
	for (i = 0; i < lengthof(InvItemHeight); i++) {
		if (i != CURSOR_NONE && InvItemHeight[i] == 0)
			app_fatal("Invalid (zero) cursor height at %d.", i);
	}

	// quests
	for (i = 0; i < lengthof(AllLevels); i++) {
		int j = 0;
		int minscatts[MAX_LVLMTYPES] = { 0 };
		int mintypes[MAX_LVLMTYPES];
		for ( ; j < lengthof(AllLevels[i].dMonTypes); j++) {
			if (AllLevels[i].dMonTypes[j] == MT_INVALID)
				break;
			int mfn = monsterdata[AllLevels[i].dMonTypes[j]].moFileNum;
			for (int k = 0; k < MAX_LVLMTYPES; k++) {
				int moi = monfiledata[mfn].moImage;
				if (minscatts[k] < moi) {
					if (minscatts[k] == 0) {
						minscatts[k] = moi;
						mintypes[k] = mfn;
						break;
					}
					continue;
				} else {
					int nfn = mintypes[k];

					minscatts[k] = moi;
					mintypes[k] = mfn;

					mfn = nfn;
				}
			}
		}
		int imgtot = monsterdata[monsterdata[MT_GOLEM].moFileNum].moFileNum;
		if (i != DLV_TOWN && i < NUM_STDLVLS && minscatts[0] > MAX_LVLMIMAGE - imgtot)
			app_fatal("Monster types on level %d do not fit to the limit (%d). Lowest complexity is %d.", i, MAX_LVLMIMAGE - imgtot, minscatts[0]);
		int k = 0;
		for ( ; k < MAX_LVLMTYPES - 2; k++) {
			if (minscatts[k] == 0)
				break;
			imgtot += minscatts[k];
		}
		if (i != DLV_TOWN && i < NUM_STDLVLS && imgtot < MAX_LVLMIMAGE && j > k) {
			tempstr[0] = '\0';
			for (int n = 0; n < std::min(j, MAX_LVLMTYPES); n++) {
				snprintf(tempstr, sizeof(tempstr), "%s, %d (%d)", tempstr, mintypes[n], minscatts[n]);
			}
			app_fatal("Monsters with low complexity on level %d: total:%d, monster-types(%d):%s", i, imgtot, k, tempstr);
		}

		if (j == lengthof(AllLevels[i].dMonTypes))
			app_fatal("Missing closing MT_INVALID on level %s (%d)", AllLevels[i].dLevelName, i);
		if (i != DLV_TOWN && AllLevels[i].dLevel == 0) // required by GetItemAttrs
			app_fatal("Too low dLevel on level %s (%d)", AllLevels[i].dLevelName, i);
		if ((AllLevels[i].dLevel * 8 - AllLevels[i].dLevel * 2) >= 0x7FFF) // required by GetItemAttrs
			app_fatal("Too high dLevel on level %s (%d)", AllLevels[i].dLevelName, i);
	}

	// monsters
	for (i = 0; i < NUM_MTYPES; i++) {
		const MonsterData& md = monsterdata[i];
		// check RETREAT_DISTANCE for MonFallenFear
		if (md.mAI.aiType == AI_FALLEN && md.mAI.aiInt > 3)
			app_fatal("Invalid mInt %d for %s (%d)", md.mAI.aiInt, md.mName, i);
		if (md.mAI.aiType == AI_COUNSLR && md.mAI.aiInt > 3)
			app_fatal("Invalid mInt %d for %s (%d)", md.mAI.aiInt, md.mName, i);
		if ((md.mAI.aiType == AI_GOLUM || md.mAI.aiType == AI_SKELKING) && !(md.mFlags & MFLAG_CAN_OPEN_DOOR))
			app_fatal("AI_GOLUM and AI_SKELKING always check the doors (%s, %d)", md.mName, i);
		if ((md.mAI.aiType == AI_FALLEN || md.mAI.aiType == AI_SNAKE || md.mAI.aiType == AI_SNEAK || md.mAI.aiType == AI_SKELBOW) && (md.mFlags & MFLAG_CAN_OPEN_DOOR))
			app_fatal("AI_FALLEN,  AI_SNAKE, AI_SNEAK and AI_SKELBOW never check the doors (%s, %d)", md.mName, i);
#ifdef HELLFIRE
		if ((md.mAI.aiType == AI_HORKDMN) && (md.mFlags & MFLAG_CAN_OPEN_DOOR))
			app_fatal("AI_HORKDMN never check the doors (%s, %d)", md.mName, i);
#endif
		if ((md.mAI.aiType == AI_CLEAVER || md.mAI.aiType == AI_FAT || md.mAI.aiType == AI_BAT) && (md.mFlags & MFLAG_CAN_OPEN_DOOR) && !(md.mFlags & MFLAG_SEARCH))
			app_fatal("AI_CLEAVER, AI_FAT and AI_BAT only check the doors while searching (%s, %d)", md.mName, i);
		if (md.mLevel > UINT8_MAX - HELL_LEVEL_BONUS)
			app_fatal("Too high mLevel %d for %s (%d).", md.mLevel, md.mName, i);
		if (md.mLevel + HELL_LEVEL_BONUS > CF_LEVEL && (md.mTreasure & NO_DROP) == 0)
			app_fatal("Invalid mLevel %d for %s (%d). Too high to set the level of item-drop.", md.mLevel, md.mName, i);
		if (md.moFileNum == MOFILE_DIABLO && !(md.mFlags & MFLAG_NOCORPSE))
			app_fatal("MOFILE_DIABLO does not have corpse animation but MFLAG_NOCORPSE is not set for %s (%d).", md.mName, i);
		if (md.mHit > UINT16_MAX - HELL_TO_HIT_BONUS)
			app_fatal("Too high mHit %d for %s (%d).", md.mHit, md.mName, i);
		if (md.mHit2 > UINT16_MAX - HELL_TO_HIT_BONUS)
			app_fatal("Too high mHit2 %d for %s (%d).", md.mHit2, md.mName, i);
		if (md.mMagic > UINT8_MAX - HELL_MAGIC_BONUS)
			app_fatal("Too high mMagic %d for %s (%d).", md.mMagic, md.mName, i);
		if (md.mMinDamage * 4 > UINT8_MAX - 6)
			app_fatal("Too high mMinDamage %d for %s (%d).", md.mMinDamage, md.mName, i);
		if (md.mMinDamage2 * 4 > UINT8_MAX - 6)
			app_fatal("Too high mMinDamage2 %d for %s (%d).", md.mMinDamage2, md.mName, i);
		if (md.mMaxDamage * 4 > UINT8_MAX - 6)
			app_fatal("Too high mMaxDamage %d for %s (%d).", md.mMaxDamage, md.mName, i);
		if (md.mMaxDamage2 * 4 > UINT8_MAX - 6)
			app_fatal("Too high mMaxDamage2 %d for %s (%d).", md.mMaxDamage2, md.mName, i);
		if (md.mArmorClass > UINT8_MAX - HELL_AC_BONUS)
			app_fatal("Too high mArmorClass %d for %s (%d).", md.mArmorClass, md.mName, i);
		if (md.mEvasion > UINT8_MAX - HELL_EVASION_BONUS)
			app_fatal("Too high mEvasion %d for %s (%d).", md.mEvasion, md.mName, i);
		if (md.mMinHP <= 0)
			app_fatal("Invalid mMinHP %d for %s (%d)", md.mMinHP, md.mName, i);
		if (md.mMinHP > md.mMaxHP)
			app_fatal("Too high mMinHP %d for %s (%d)", md.mMinHP, md.mName, i);
		if (md.mMaxHP * 4 > UINT16_MAX - 200) // required by InitMonsterStats
			app_fatal("Too high mMaxHP %d for %s (%d)", md.mMaxHP, md.mName, i);
		if ((md.mMaxHP - md.mMinHP) * 4 + 200 >= 0x7FFF) // required by InitMonster
			app_fatal("Min/MaxHP range (%d-%d) too high for %s (%d)", md.mMinHP, md.mMaxHP, md.mName, i);
		if ((md.mExp + DIFFICULTY_EXP_BONUS) > (UINT32_MAX / 4))
			app_fatal("Too high mExp %d for %s (%d)", md.mExp, md.mName, i);
		uint16_t res = md.mMagicRes;
		uint16_t resH = md.mMagicRes2;
		for (int j = 0; j < 8; j++, res >>= 2, resH >>= 2) {
			if ((res & 3) > (resH & 3)) {
				app_fatal("Bad mMagicRes2 %d (%d) for %s (%d): worse than mMagicRes %d.", md.mMagicRes2, j, md.mName, i, md.mMagicRes);
			}
		}
	}
	for (i = 0; i < NUM_MOFILE; i++) {
		const MonFileData& md = monfiledata[i];
		if (md.moAnimFrames[MA_STAND] > 0x7FFF) // required by InitMonster
			app_fatal("Too many(%d) stand-frames for %s (%d).", md.moAnimFrames[MA_STAND], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_STAND] >= 0x7FFF) // required by InitMonster
			app_fatal("Too long(%d) standing animation for %s (%d).", md.moAnimFrameLen[MA_STAND], md.moGfxFile, i);
		if (md.moAnimFrames[MA_WALK] > 24) // required by MonWalkDir
			app_fatal("Too many(%d) walk-frames for %s (%d).", md.moAnimFrames[MA_WALK], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_WALK] * md.moAnimFrames[MA_WALK] >= SQUELCH_LOW)
			app_fatal("Too long(%d) walking animation for %s (%d) to finish before relax.", md.moAnimFrameLen[MA_WALK] * md.moAnimFrames[MA_WALK], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_ATTACK] * md.moAnimFrames[MA_ATTACK] >= SQUELCH_LOW)
			app_fatal("Too long(%d) attack animation for %s (%d) to finish before relax.", md.moAnimFrameLen[MA_ATTACK] * md.moAnimFrames[MA_ATTACK], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_SPECIAL] * md.moAnimFrames[MA_SPECIAL] >= SQUELCH_LOW)
			app_fatal("Too long(%d) special animation for %s (%d) to finish before relax.", md.moAnimFrameLen[MA_SPECIAL] * md.moAnimFrames[MA_SPECIAL], md.moGfxFile, i);
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

	for (i = 0; uniqMonData[i].mtype != MT_INVALID; i++) {
		const UniqMonData& um = uniqMonData[i];
		if (um.mtype >= NUM_MTYPES)
			app_fatal("Invalid unique monster type %d for %s (%d)", um.mtype, um.mName, i);
		int j = 0;
		int lvl = um.muLevelIdx;
		if (lvl == 0 && um.mQuestId != Q_INVALID)
			app_fatal("Inconsistent unique monster %s (%d). Has a quest, but no quest-level.", um.mName, i);
		if (lvl != 0 && um.mQuestId != Q_INVALID && lvl != questlist[um.mQuestId]._qdlvl)
			app_fatal("Inconsistent unique monster %s (%d). Has a quest, but its level-idx (%d) does not match the quest-level (%d).", um.mName, i, lvl, questlist[um.mQuestId]._qdlvl);
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
		if (um.mAI.aiType == AI_FALLEN && um.mAI.aiInt > 3)
			app_fatal("Invalid mInt %d for %s (%d)", um.mAI.aiInt, um.mName, i);
		if (um.mAI.aiType == AI_COUNSLR && um.mAI.aiInt > 3)
			app_fatal("Invalid mInt %d for %s (%d)", um.mAI.aiInt, um.mName, i);
		if ((um.mAI.aiType == AI_GOLUM || um.mAI.aiType == AI_SKELKING) && !(monsterdata[um.mtype].mFlags & MFLAG_CAN_OPEN_DOOR))
			app_fatal("Unique AI_GOLUM and AI_SKELKING always check the doors (%s, %d)", um.mName, i);
		if ((um.mAI.aiType == AI_FALLEN || um.mAI.aiType == AI_SNAKE || um.mAI.aiType == AI_SNEAK || um.mAI.aiType == AI_SKELBOW) && (monsterdata[um.mtype].mFlags & MFLAG_CAN_OPEN_DOOR))
			app_fatal("Unique AI_FALLEN, AI_CLEAVER, AI_SNAKE, AI_SNEAK, AI_SKELBOW and AI_FAT never check the doors (%s, %d)", um.mName, i);
#ifdef HELLFIRE
		if ((um.mAI.aiType == AI_HORKDMN) && (monsterdata[um.mtype].mFlags & MFLAG_CAN_OPEN_DOOR))
			app_fatal("Unique AI_HORKDMN never check the doors (%s, %d)", um.mName, i);
#endif
		if ((um.mAI.aiType == AI_CLEAVER || um.mAI.aiType == AI_FAT) && (monsterdata[um.mtype].mFlags & MFLAG_CAN_OPEN_DOOR) && !(monsterdata[um.mtype].mFlags & MFLAG_SEARCH))
			app_fatal("Unique AI_CLEAVER and AI_FAT only check the doors while searching (%s, %d)", um.mName, i);
		if (um.muLevel + HELL_LEVEL_BONUS > CF_LEVEL && (monsterdata[um.mtype].mTreasure & NO_DROP) == 0)
			app_fatal("Invalid muLevel %d for %s (%d). Too high in hell to set the level of item-drop.", um.muLevel, um.mName, i);
		if ((um.mUnqFlags & UMF_LEADER) != 0 && ((um.mUnqFlags & UMF_GROUP) == 0))
			app_fatal("Unique monster %s (%d) is a leader without group.", um.mName, i);
		if (um.mUnqHit + monsterdata[um.mtype].mHit > UINT16_MAX - HELL_TO_HIT_BONUS) // required by PlaceUniqueMonst
			app_fatal("Too high mUnqHit %d for unique monster %s (%d).", um.mUnqHit, um.mName, i);
		if (um.mUnqAC + monsterdata[um.mtype].mArmorClass > UINT8_MAX - HELL_AC_BONUS) // required by PlaceUniqueMonst
			app_fatal("Too high mUnqAC %d for unique monster %s (%d).", um.mUnqAC, um.mName, i);
		if (um.mmaxhp < 2) // required by PlaceUniqueMonst
			app_fatal("Too low mmaxhp %d for unique monster %s (%d).", um.mmaxhp, um.mName, i);
		if ((monsterdata[um.mtype].mExp + DIFFICULTY_EXP_BONUS) > (UINT32_MAX / (4 * 2))) // required by PlaceUniqueMonst
			app_fatal("Too high mExp %d for %s (%d)", monsterdata[um.mtype].mExp, um.mName, i);
#if DEBUG_MODE
		uint16_t res = monsterdata[um.mtype].mMagicRes;
		uint16_t resU = um.mMagicRes;
		for (int j = 0; j < 8; j++, res >>= 2, resU >>= 2) {
			if ((res & 3) > (resU & 3)) {
				DoLog("Warn: Weak muMagicRes %d (%d) for %s (%d): worse than mMagicRes %d.", um.mMagicRes, j, um.mName, i, monsterdata[um.mtype].mMagicRes);
			}
		}
#endif
	}

	// items
	if (AllItemsList[IDI_HEAL].iMiscId != IMISC_HEAL)
		app_fatal("IDI_HEAL is not a heal potion, its miscId is %d, iminlvl %d.", AllItemsList[IDI_HEAL].iMiscId, AllItemsList[IDI_HEAL].iMinMLvl);
	if (AllItemsList[IDI_FULLHEAL].iMiscId != IMISC_FULLHEAL)
		app_fatal("IDI_FULLHEAL is not a heal potion, its miscId is %d, iminlvl %d.", AllItemsList[IDI_FULLHEAL].iMiscId, AllItemsList[IDI_FULLHEAL].iMinMLvl);
	if (AllItemsList[IDI_MANA].iMiscId != IMISC_MANA)
		app_fatal("IDI_MANA is not a mana potion, its miscId is %d, iminlvl %d.", AllItemsList[IDI_MANA].iMiscId, AllItemsList[IDI_MANA].iMinMLvl);
	if (AllItemsList[IDI_FULLMANA].iMiscId != IMISC_FULLMANA)
		app_fatal("IDI_FULLMANA is not a mana potion, its miscId is %d, iminlvl %d.", AllItemsList[IDI_FULLMANA].iMiscId, AllItemsList[IDI_FULLMANA].iMinMLvl);
	if (AllItemsList[IDI_REJUV].iMiscId != IMISC_REJUV)
		app_fatal("IDI_REJUV is not a rejuv potion, its miscId is %d, iminlvl %d.", AllItemsList[IDI_REJUV].iMiscId, AllItemsList[IDI_REJUV].iMinMLvl);
	if (AllItemsList[IDI_FULLREJUV].iMiscId != IMISC_FULLREJUV)
		app_fatal("IDI_FULLREJUV is not a rejuv potion, its miscId is %d.", AllItemsList[IDI_FULLREJUV].iMiscId, AllItemsList[IDI_FULLREJUV].iMinMLvl);
	if (AllItemsList[IDI_BOOK1].iMiscId != IMISC_BOOK)
		app_fatal("IDI_BOOK1 is not a book, its miscId is %d, iminlvl %d.", AllItemsList[IDI_BOOK1].iMiscId, AllItemsList[IDI_BOOK1].iMinMLvl);
	if (AllItemsList[IDI_BOOK4].iMiscId != IMISC_BOOK)
		app_fatal("IDI_BOOK4 is not a book, its miscId is %d, iminlvl %d.", AllItemsList[IDI_BOOK4].iMiscId, AllItemsList[IDI_BOOK4].iMinMLvl);
	static_assert(IDI_BOOK4 - IDI_BOOK1 == 3, "Invalid IDI_BOOK indices.");
	if (AllItemsList[IDI_CLUB].iCurs != ICURS_CLUB)
		app_fatal("IDI_CLUB is not a club, its cursor is %d, iminlvl %d.", AllItemsList[IDI_CLUB].iCurs, AllItemsList[IDI_CLUB].iMinMLvl);
	if (AllItemsList[IDI_DROPSHSTAFF].iUniqType != UITYPE_SHORTSTAFF)
		app_fatal("IDI_DROPSHSTAFF is not a short staff, its utype is %d, iminlvl %d.", AllItemsList[UITYPE_SHORTSTAFF].iUniqType, AllItemsList[UITYPE_SHORTSTAFF].iMinMLvl);
	int minAmu, minLightArmor, minMediumArmor, minHeavyArmor; //, maxStaff = 0;
	minAmu = minLightArmor = minMediumArmor = minHeavyArmor = MAXCHARLEVEL;
	int rnddrops = 0;
	for (i = 0; i < NUM_IDI; i++) {
		const ItemData& ids = AllItemsList[i];
		if (strlen(ids.iName) > 32 - 1)
			app_fatal("Too long name for %s (%d)", ids.iName, i);
		rnddrops += ids.iRnd;
		if (i < IDI_RNDDROP_FIRST && ids.iRnd != 0)
			app_fatal("Invalid iRnd value for %s (%d)", ids.iName, i);
		if (ids.itype == ITYPE_NONE)
			app_fatal("Invalid itype value for %s (%d)", ids.iName, i);
		if (ids.itype == ITYPE_LARMOR && ids.iMinMLvl < minLightArmor && ids.iRnd != 0)
			minLightArmor = ids.iMinMLvl;
		if (ids.itype == ITYPE_MARMOR && ids.iMinMLvl < minMediumArmor && ids.iRnd != 0)
			minMediumArmor = ids.iMinMLvl;
		if (ids.itype == ITYPE_HARMOR && ids.iMinMLvl < minHeavyArmor && ids.iRnd != 0)
			minHeavyArmor = ids.iMinMLvl;
		if (ids.iMinMLvl == 0 && ids.itype != ITYPE_MISC && ids.itype != ITYPE_GOLD && ids.iMiscId != IMISC_UNIQUE)
			app_fatal("iMinMLvl field is not set for %s (%d).", ids.iName, i);
		if (ids.iMiscId == IMISC_UNIQUE && ids.iRnd != 0)
			app_fatal("Fix unique item %s (%d) should not be part of the loot.", ids.iName, i);
		if (ids.iClass == ICLASS_ARMOR) {
			if (ids.itype != ITYPE_LARMOR && ids.itype != ITYPE_MARMOR
			 && ids.itype != ITYPE_HARMOR && ids.itype != ITYPE_SHIELD
			 && ids.itype != ITYPE_HELM)
				app_fatal("Invalid type (%d) set for %s (%d), which is an armor.", ids.itype, ids.iName, i);
			if (ids.iMinAC > ids.iMaxAC)
				app_fatal("Too high iMinAC %d for %s (%d)", ids.iMinAC, ids.iName, i);
			if (ids.iMaxAC - ids.iMinAC >= 0x7FFF) // required by SetItemData
				app_fatal("Min/MaxAC range (%d-%d) too high for %s (%d)", ids.iMinAC, ids.iMaxAC, ids.iName, i);
		} else {
			if (ids.iMinAC != 0 || ids.iMaxAC != 0)
				app_fatal("AC set for %s (%d), which is not an armor.", ids.iName, i);
		}
		if (ids.iClass == ICLASS_WEAPON) {
			if (ids.itype != ITYPE_SWORD && ids.itype != ITYPE_AXE
			 && ids.itype != ITYPE_BOW && ids.itype != ITYPE_MACE
			 && ids.itype != ITYPE_STAFF)
				app_fatal("Invalid type (%d) set for %s (%d), which is a weapon.", ids.itype, ids.iName, i);
			if (ids.iDamType == IDAM_NONE)
				app_fatal("Damage type not set for %s (%d), which is a weapon.", ids.iName, i);
			if (ids.iMaxDam == 0)
				app_fatal("Damage not set for %s (%d), which is a weapon.", ids.iName, i);
		} else {
			if (ids.iDamType != IDAM_NONE)
				app_fatal("Damage type (%d) set for %s (%d), which is not a weapon.", ids.iDamType, ids.iName, i);
			if (ids.iMinDam != 0 || ids.iMaxDam != 0)
				app_fatal("Damage set for %s (%d), which is not a weapon.", ids.iName, i);
			if (ids.iBaseCrit != 0)
				app_fatal("Crit.chance set for %s (%d), which is not a weapon.", ids.iName, i);
		}
		if (ids.itype == ITYPE_AMULET && ids.iMinMLvl < minAmu && ids.iRnd != 0)
			minAmu = ids.iMinMLvl;
		//if (ids.itype == ITYPE_STAFF && strlen(ids.iName) > maxStaff)
		//	maxStaff = strlen(ids.iName);
		if (ids.iLoc == ILOC_BELT) {
			if (!ids.iUsable)
				app_fatal("Belt item %s (%d) should be usable.", ids.iName, i);
			if (InvItemWidth[ids.iCurs + CURSOR_FIRSTITEM] != INV_SLOT_SIZE_PX)
				app_fatal("Belt item %s (%d) is too wide.", ids.iName, i);
			if (InvItemHeight[ids.iCurs + CURSOR_FIRSTITEM] != INV_SLOT_SIZE_PX)
				app_fatal("Belt item %s (%d) is too tall.", ids.iName, i);
		}
		if (ids.iDurability * 3 >= DUR_INDESTRUCTIBLE) // required by SaveItemPower/IPL_DUR
			app_fatal("Item %s (%d) has too high durability I.", ids.iName, i);
		if (ids.iDurability * 3 >= 0x7FFF) // required by ItemRndDur
			app_fatal("Item %s (%d) has too high durability II.", ids.iName, i);
		if (ids.iUsable) {
			switch (ids.iMiscId) {
			case IMISC_HEAL:
			case IMISC_FULLHEAL:
			case IMISC_MANA:
			case IMISC_FULLMANA:
			case IMISC_REJUV:
			case IMISC_FULLREJUV:
			case IMISC_SCROLL:
#ifdef HELLFIRE
			case IMISC_RUNE:
#endif
			case IMISC_BOOK:
			case IMISC_SPECELIX:
			//case IMISC_MAPOFDOOM:
			case IMISC_NOTE:
			case IMISC_OILQLTY:
			case IMISC_OILZEN:
			case IMISC_OILSTR:
			case IMISC_OILDEX:
			case IMISC_OILVIT:
			case IMISC_OILMAG:
			case IMISC_OILRESIST:
			case IMISC_OILCHANCE:
			case IMISC_OILCLEAN:
				break;
			default:
				app_fatal("Usable item %s (%d) with miscId %d is not handled by SyncUseItem.", ids.iName, i, ids.iMiscId);
			}
			if (ids.iDurability == 0)
				app_fatal("Usable item %s (%d) with miscId %d must have a non-zero durablity(stacksize).", ids.iName, i, ids.iMiscId);
		}
		if (ids.iClass == ICLASS_QUEST && ids.iLoc != ILOC_UNEQUIPABLE)
			app_fatal("Quest item %s (%d) must be unequippable, not %d", ids.iName, i, ids.iLoc);
		if (ids.iClass == ICLASS_QUEST && ids.itype != ITYPE_MISC)
			app_fatal("Quest item %s (%d) must be have 'misc' itype, otherwise it might be sold at vendors.", ids.iName, i);
	}
#if UNOPTIMIZED_RNDITEMS
	if (rnddrops > ITEM_RNDDROP_MAX)
		app_fatal("Too many drop options: %d. Maximum is %d", rnddrops, ITEM_RNDDROP_MAX);
#else
	if (rnddrops > 0x7FFF)
		app_fatal("Too many drop options: %d. Maximum is %d", rnddrops, 0x7FFF);
#endif
	if (minLightArmor > 1)
		app_fatal("No light armor for OperateArmorStand. Current minimum is level %d", minLightArmor);
	if (minMediumArmor > 10)
		app_fatal("No medium armor for OperateArmorStand. Current minimum is level %d", minMediumArmor);
	if (minHeavyArmor > 24)
		app_fatal("No heavy armor for OperateArmorStand. Current minimum is level %d", minHeavyArmor);
	if (uniqMonData[UMT_HORKDMN].muLevel < minAmu)
		app_fatal("No amulet for THEODORE. Current minimum is level %d, while the monster level is %d.", minAmu, uniqMonData[UMT_HORKDMN].muLevel);
	rnddrops = 0; i = 0;
	for (const AffixData *pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++, i++) {
		rnddrops += pres->PLDouble ? 2 : 1;
		if (pres->PLParam2 < pres->PLParam1) {
			app_fatal("Invalid PLParam set for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
		}
		if (pres->PLParam2 - pres->PLParam1 >= 0x7FFF) { // required by SaveItemPower
			app_fatal("PLParam too high for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
		}
		if (pres->PLPower == IPL_TOHIT_DAMP) {
			if ((pres->PLParam2 >> 2) - (pres->PLParam1 >> 2) == 0) { // required by SaveItemPower
				app_fatal("PLParam too low for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
			//if ((pres->PLParam2 >> 2) - (pres->PLParam1 >> 2) >= 0x7FFF) { // required by SaveItemPower
			//	app_fatal("PLParam too high for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			//}
		}
		if (pres->PLPower == IPL_FASTATTACK) {
			if (pres->PLParam1 < 1 || pres->PLParam2 > 4) {
				app_fatal("Invalid PLParam set for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
		}
		if (pres->PLPower == IPL_FASTRECOVER) {
			if (pres->PLParam1 < 1 || pres->PLParam2 > 3) {
				app_fatal("Invalid PLParam set for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
		}
		if (pres->PLPower == IPL_FASTCAST) {
			if (pres->PLParam1 < 1 || pres->PLParam2 > 3) {
				app_fatal("Invalid PLParam set for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
		}
		if (pres->PLPower == IPL_FASTWALK) {
			if (pres->PLParam1 < 1 || pres->PLParam2 > 3) {
				app_fatal("Invalid PLParam set for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
		}
		if (pres->PLPower == IPL_DUR) {
			if (pres->PLParam2 > 200) {
				app_fatal("PLParam too high for %d. prefix (power:%d, pparam2:%d)", i, pres->PLPower, pres->PLParam2);
			}
		}
	}
	if (rnddrops > ITEM_RNDAFFIX_MAX || rnddrops > 0x7FFF)
		app_fatal("Too many prefix options: %d. Maximum is %d", rnddrops, ITEM_RNDAFFIX_MAX);
	rnddrops = 0;
	const AffixData* sufs = PL_Suffix;
	for (i = 0; sufs->PLPower != IPL_INVALID; sufs++, i++) {
		if (sufs->PLDouble)
			app_fatal("Invalid PLDouble set for %d. suffix (power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
		rnddrops++;
		if (sufs->PLPower == IPL_FASTATTACK) {
			if (sufs->PLParam1 < 1 || sufs->PLParam2 > 4) {
				app_fatal("Invalid PLParam set for %d. suffix (power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
			}
		}
		if (sufs->PLPower == IPL_FASTRECOVER) {
			if (sufs->PLParam1 < 1 || sufs->PLParam2 > 3) {
				app_fatal("Invalid PLParam set for %d. suffix (power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
			}
		}
		if (sufs->PLPower == IPL_FASTCAST) {
			if (sufs->PLParam1 < 1 || sufs->PLParam2 > 3) {
				app_fatal("Invalid PLParam set for %d. suffix (power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
			}
		}
		if (sufs->PLPower == IPL_FASTWALK) {
			if (sufs->PLParam1 < 1 || sufs->PLParam2 > 3) {
				app_fatal("Invalid PLParam set for %d. suffix (power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
			}
		}
		if (sufs->PLPower == IPL_DUR) {
			if (sufs->PLParam2 > 200) {
				app_fatal("PLParam too high for %d. suffix (power:%d, pparam2:%d)", i, sufs->PLPower, sufs->PLParam2);
			}
		}
		for (const AffixData *pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {
			if ((sufs->PLIType & pres->PLIType) == 0)
				continue;
			if (sufs->PLPower == pres->PLPower) {
				app_fatal("Same power is set as prefix and suffix at the same time: suffix(%d. power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
			}
			if (pres->PLPower == IPL_ATTRIBS &&
				(sufs->PLPower == IPL_STR || sufs->PLPower == IPL_MAG || sufs->PLPower == IPL_DEX || sufs->PLPower == IPL_VIT)) {
				app_fatal("IPL_ATTRIBS and IPL_STR/IPL_MAG/IPL_DEX/IPL_VIT might be set for the same item.");
			}
			if (sufs->PLPower == IPL_ATTRIBS &&
				(pres->PLPower == IPL_STR || pres->PLPower == IPL_MAG || pres->PLPower == IPL_DEX || pres->PLPower == IPL_VIT)) {
				app_fatal("IPL_STR/IPL_MAG/IPL_DEX/IPL_VIT and IPL_ATTRIBS might be set for the same item.");
			}
			if (pres->PLPower == IPL_ALLRES &&
				(sufs->PLPower == IPL_FIRERES || sufs->PLPower == IPL_LIGHTRES || sufs->PLPower == IPL_MAGICRES || sufs->PLPower == IPL_ACIDRES)) {
				app_fatal("IPL_ALLRES and IPL_FIRERES/IPL_LIGHTRES/IPL_MAGICRES/IPL_ACIDRES might be set for the same item.");
			}
			if (sufs->PLPower == IPL_ALLRES &&
				(pres->PLPower == IPL_FIRERES || pres->PLPower == IPL_LIGHTRES || pres->PLPower == IPL_MAGICRES || pres->PLPower == IPL_ACIDRES)) {
				app_fatal("IPL_FIRERES/IPL_LIGHTRES/IPL_MAGICRES/IPL_ACIDRES and IPL_ALLRES might be set for the same item.");
			}
			if (sufs->PLPower == IPL_TOHIT && pres->PLPower == IPL_TOHIT_DAMP) {
				app_fatal("IPL_TOHIT_DAMP and IPL_TOHIT might be set for the same item.");
			}
			if (pres->PLPower == IPL_TOHIT && sufs->PLPower == IPL_TOHIT_DAMP) {
				app_fatal("IPL_TOHIT and IPL_TOHIT_DAMP might be set for the same item.");
			}
			if ((pres->PLPower == IPL_DAMP || pres->PLPower == IPL_TOHIT_DAMP || pres->PLPower == IPL_CRYSTALLINE) &&
				(sufs->PLPower == IPL_DAMP || sufs->PLPower == IPL_TOHIT_DAMP || sufs->PLPower == IPL_CRYSTALLINE)) {
				app_fatal("IPL_DAMP/IPL_TOHIT_DAMP/IPL_CRYSTALLINE might be set for the same item.");
			}
		}
	}
	if (rnddrops > ITEM_RNDAFFIX_MAX || rnddrops > 0x7FFF)
		app_fatal("Too many suffix options: %d. Maximum is %d", rnddrops, ITEM_RNDAFFIX_MAX);

#if 0
	for (i = 1; i < MAXCHARLEVEL; i++) {
		int a = 0, b = 0, c = 0, w = 0;
		for (const AffixData *pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {
			if (pres->PLMinLvl > i) {
				if (pres->PLMinLvl <= (i << 1) && pres->PLOk)
					c++;
				continue;
			}
			if (pres->PLMinLvl >= (i >> 1) && pres->PLOk) {
				a++;
				c++;
			}
			if (pres->PLMinLvl >= (i >> 2)) {
				b++;
				if (!pres->PLOk)
					w++;
			}
		}
		int as = 0, bs = 0, cs = 0, ws = 0;
		for (const AffixData *pres = PL_Suffix; pres->PLPower != IPL_INVALID; pres++) {
			if (pres->PLMinLvl > i) {
				if (pres->PLMinLvl <= (i << 1) && pres->PLOk)
					cs++;
				continue;
			}
			if (pres->PLMinLvl >= (i >> 1) && pres->PLOk) {
				as++;
				cs++;
			}
			if (pres->PLMinLvl >= (i >> 2)) {
				bs++;
				if (!pres->PLOk)
					ws++;
			}
		}
		LogErrorF("ITEMAFF", "Affix for lvl%2d: shop(%d:%d) loot(%d:%d/%d:%d) boy(%d:%d)", i, a, as, b, bs, w, ws, c, cs);
	}
#endif
	// unique items
	for (i = 0; i < NUM_UITEM; i++) {
		const UniqItemData& ui = UniqueItemList[i];
		if (ui.UIPower1 == IPL_INVALID)
			app_fatal("Unique item '%s' %d does not have any affix", ui.UIName, i);
		//if (ui.UIPower1 != IPL_INVALID) {
			if (ui.UIPower1 == ui.UIPower2) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 1vs2.", ui.UIName, i);
			}
			if (ui.UIPower1 == ui.UIPower3) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 1vs3.", ui.UIName, i);
			}
			if (ui.UIPower1 == ui.UIPower4) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 1vs4.", ui.UIName, i);
			}
			if (ui.UIPower1 == ui.UIPower5) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 1vs5.", ui.UIName, i);
			}
			if (ui.UIPower1 == ui.UIPower6) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 1vs6.", ui.UIName, i);
			}
		//}
		if (ui.UIPower2 != IPL_INVALID) {
			if (ui.UIPower2 == ui.UIPower3) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 2vs3.", ui.UIName, i);
			}
			if (ui.UIPower2 == ui.UIPower4) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 2vs4.", ui.UIName, i);
			}
			if (ui.UIPower2 == ui.UIPower5) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 2vs5.", ui.UIName, i);
			}
			if (ui.UIPower2 == ui.UIPower6) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 2vs6.", ui.UIName, i);
			}
		} else {
			if (ui.UIPower3 != IPL_INVALID || ui.UIPower4 != IPL_INVALID || ui.UIPower5 != IPL_INVALID || ui.UIPower6 != IPL_INVALID)
				app_fatal("Unique item '%s' %d ignores its set affix, because UIPower2 is IPL_INVALID.", ui.UIName, i);
		}
		if (ui.UIPower3 != IPL_INVALID) {
			if (ui.UIPower3 == ui.UIPower4) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 3vs4.", ui.UIName, i);
			}
			if (ui.UIPower3 == ui.UIPower5) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 3vs5.", ui.UIName, i);
			}
			if (ui.UIPower3 == ui.UIPower6) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 3vs6.", ui.UIName, i);
			}
		} else {
			if (ui.UIPower4 != IPL_INVALID || ui.UIPower5 != IPL_INVALID || ui.UIPower6 != IPL_INVALID)
				app_fatal("Unique item '%s' %d ignores its set affix, because UIPower3 is IPL_INVALID.", ui.UIName, i);
		}
		if (ui.UIPower4 != IPL_INVALID) {
			if (ui.UIPower4 == ui.UIPower5) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 4vs5.", ui.UIName, i);
			}
			if (ui.UIPower4 == ui.UIPower6) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 4vs6.", ui.UIName, i);
			}
		} else {
			if (ui.UIPower5 != IPL_INVALID || ui.UIPower6 != IPL_INVALID)
				app_fatal("Unique item '%s' %d ignores its set affix, because UIPower4 is IPL_INVALID.", ui.UIName, i);
		}
		if (ui.UIPower5 != IPL_INVALID) {
			if (ui.UIPower5 == ui.UIPower6) {
				app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, 5vs6.", ui.UIName, i);
			}
		} else {
			if (ui.UIPower6 != IPL_INVALID)
				app_fatal("Unique item '%s' %d ignores its set affix, because UIPower5 is IPL_INVALID.", ui.UIName, i);
		}
		if (ui.UIPower1 == IPL_ATTRIBS || ui.UIPower2 == IPL_ATTRIBS || ui.UIPower3 == IPL_ATTRIBS || ui.UIPower4 == IPL_ATTRIBS || ui.UIPower5 == IPL_ATTRIBS || ui.UIPower6 == IPL_ATTRIBS) {
			if ((ui.UIPower1 == IPL_STR || ui.UIPower2 == IPL_STR || ui.UIPower3 == IPL_STR || ui.UIPower4 == IPL_STR || ui.UIPower5 == IPL_STR || ui.UIPower6 == IPL_STR) ||
			    (ui.UIPower1 == IPL_MAG || ui.UIPower2 == IPL_MAG || ui.UIPower3 == IPL_MAG || ui.UIPower4 == IPL_MAG || ui.UIPower5 == IPL_MAG || ui.UIPower6 == IPL_MAG) ||
			    (ui.UIPower1 == IPL_DEX || ui.UIPower2 == IPL_DEX || ui.UIPower3 == IPL_DEX || ui.UIPower4 == IPL_DEX || ui.UIPower5 == IPL_DEX || ui.UIPower6 == IPL_DEX) ||
				(ui.UIPower1 == IPL_VIT || ui.UIPower2 == IPL_VIT || ui.UIPower3 == IPL_VIT || ui.UIPower4 == IPL_VIT || ui.UIPower5 == IPL_VIT || ui.UIPower6 == IPL_VIT)) {
				app_fatal("SaveItemPower does not support IPL_ATTRIBS and IPL_STR/IPL_MAG/IPL_DEX/IPL_VIT modifiers at the same time on '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_ALLRES || ui.UIPower2 == IPL_ALLRES || ui.UIPower3 == IPL_ALLRES || ui.UIPower4 == IPL_ALLRES || ui.UIPower5 == IPL_ALLRES || ui.UIPower6 == IPL_ALLRES) {
			if ((ui.UIPower1 == IPL_FIRERES || ui.UIPower2 == IPL_FIRERES || ui.UIPower3 == IPL_FIRERES || ui.UIPower4 == IPL_FIRERES || ui.UIPower5 == IPL_FIRERES || ui.UIPower6 == IPL_FIRERES) ||
			    (ui.UIPower1 == IPL_LIGHTRES || ui.UIPower2 == IPL_LIGHTRES || ui.UIPower3 == IPL_LIGHTRES || ui.UIPower4 == IPL_LIGHTRES || ui.UIPower5 == IPL_LIGHTRES || ui.UIPower6 == IPL_LIGHTRES) ||
			    (ui.UIPower1 == IPL_MAGICRES || ui.UIPower2 == IPL_MAGICRES || ui.UIPower3 == IPL_MAGICRES || ui.UIPower4 == IPL_MAGICRES || ui.UIPower5 == IPL_MAGICRES || ui.UIPower6 == IPL_MAGICRES) ||
				(ui.UIPower1 == IPL_ACIDRES || ui.UIPower2 == IPL_ACIDRES || ui.UIPower3 == IPL_ACIDRES || ui.UIPower4 == IPL_ACIDRES || ui.UIPower5 == IPL_ACIDRES || ui.UIPower6 == IPL_ACIDRES)) {
				app_fatal("SaveItemPower does not support IPL_ALLRES and IPL_FIRERES/IPL_LIGHTRES/IPL_MAGICRES/IPL_ACIDRES modifiers at the same time on '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_TOHIT || ui.UIPower2 == IPL_TOHIT || ui.UIPower3 == IPL_TOHIT || ui.UIPower4 == IPL_TOHIT || ui.UIPower5 == IPL_TOHIT || ui.UIPower6 == IPL_TOHIT) {
			if (ui.UIPower1 == IPL_TOHIT_DAMP || ui.UIPower2 == IPL_TOHIT_DAMP || ui.UIPower3 == IPL_TOHIT_DAMP || ui.UIPower4 == IPL_TOHIT_DAMP || ui.UIPower5 == IPL_TOHIT_DAMP || ui.UIPower6 == IPL_TOHIT_DAMP) {
				app_fatal("SaveItemPower does not support IPL_TOHIT and IPL_TOHIT_DAMP modifiers at the same time on '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_DAMP || ui.UIPower2 == IPL_DAMP || ui.UIPower3 == IPL_DAMP || ui.UIPower4 == IPL_DAMP || ui.UIPower5 == IPL_DAMP || ui.UIPower6 == IPL_DAMP) {
			if ((ui.UIPower1 == IPL_TOHIT_DAMP || ui.UIPower2 == IPL_TOHIT_DAMP || ui.UIPower3 == IPL_TOHIT_DAMP || ui.UIPower4 == IPL_TOHIT_DAMP || ui.UIPower5 == IPL_TOHIT_DAMP || ui.UIPower6 == IPL_TOHIT_DAMP) ||
			    (ui.UIPower1 == IPL_CRYSTALLINE || ui.UIPower2 == IPL_CRYSTALLINE || ui.UIPower3 == IPL_CRYSTALLINE || ui.UIPower4 == IPL_CRYSTALLINE || ui.UIPower5 == IPL_CRYSTALLINE || ui.UIPower6 == IPL_CRYSTALLINE)) {
				app_fatal("SaveItemPower does not support IPL_DAMP and IPL_TOHIT_DAMP/IPL_CRYSTALLINE modifiers at the same time on '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_TOHIT_DAMP || ui.UIPower2 == IPL_TOHIT_DAMP || ui.UIPower3 == IPL_TOHIT_DAMP || ui.UIPower4 == IPL_TOHIT_DAMP || ui.UIPower5 == IPL_TOHIT_DAMP || ui.UIPower6 == IPL_TOHIT_DAMP) {
			if (ui.UIPower1 == IPL_CRYSTALLINE || ui.UIPower2 == IPL_CRYSTALLINE || ui.UIPower3 == IPL_CRYSTALLINE || ui.UIPower4 == IPL_CRYSTALLINE || ui.UIPower5 == IPL_CRYSTALLINE || ui.UIPower6 == IPL_CRYSTALLINE) {
				app_fatal("SaveItemPower does not support IPL_TOHIT_DAMP and IPL_CRYSTALLINE modifiers at the same time on '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_FASTATTACK) {
			if (ui.UIParam1a < 1 || ui.UIParam1b > 4) {
				app_fatal("Invalid UIParam1 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_FASTRECOVER) {
			if (ui.UIParam1a < 1 || ui.UIParam1b > 3) {
				app_fatal("Invalid UIParam1 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_FASTCAST) {
			if (ui.UIParam1a < 1 || ui.UIParam1b > 3) {
				app_fatal("Invalid UIParam1 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_FASTWALK) {
			if (ui.UIParam1a < 1 || ui.UIParam1b > 3) {
				app_fatal("Invalid UIParam1 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower1 == IPL_DUR) {
			if (ui.UIParam1b > 200) {
				app_fatal("Invalid UIParam1 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower2 == IPL_FASTATTACK) {
			if (ui.UIParam2a < 1 || ui.UIParam2b > 4) {
				app_fatal("Invalid UIParam2 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower2 == IPL_FASTRECOVER) {
			if (ui.UIParam2a < 1 || ui.UIParam2a > 3) {
				app_fatal("Invalid UIParam2a set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower2 == IPL_FASTCAST) {
			if (ui.UIParam2a < 1 || ui.UIParam2b > 3) {
				app_fatal("Invalid UIParam2 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower2 == IPL_FASTWALK) {
			if (ui.UIParam2a < 1 || ui.UIParam2b > 3) {
				app_fatal("Invalid UIParam2 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower2 == IPL_DUR) {
			if (ui.UIParam2b > 200) {
				app_fatal("Invalid UIParam2 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower3 == IPL_FASTATTACK) {
			if (ui.UIParam3a < 1 || ui.UIParam3b > 4) {
				app_fatal("Invalid UIParam3 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower3 == IPL_FASTRECOVER) {
			if (ui.UIParam3a < 1 || ui.UIParam3b > 3) {
				app_fatal("Invalid UIParam3 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower3 == IPL_FASTCAST) {
			if (ui.UIParam3a < 1 || ui.UIParam3b > 3) {
				app_fatal("Invalid UIParam3 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower3 == IPL_FASTWALK) {
			if (ui.UIParam3a < 1 || ui.UIParam3b > 3) {
				app_fatal("Invalid UIParam3 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower3 == IPL_DUR) {
			if (ui.UIParam3b > 200) {
				app_fatal("Invalid UIParam3 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower4 == IPL_FASTATTACK) {
			if (ui.UIParam4a < 1 || ui.UIParam4b > 4) {
				app_fatal("Invalid UIParam4 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower4 == IPL_FASTRECOVER) {
			if (ui.UIParam4a < 1 || ui.UIParam4b > 3) {
				app_fatal("Invalid UIParam4 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower4 == IPL_FASTCAST) {
			if (ui.UIParam4a < 1 || ui.UIParam4b > 3) {
				app_fatal("Invalid UIParam4 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower4 == IPL_FASTWALK) {
			if (ui.UIParam4a < 1 || ui.UIParam4b > 3) {
				app_fatal("Invalid UIParam4 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower4 == IPL_DUR) {
			if (ui.UIParam4b > 200) {
				app_fatal("Invalid UIParam4 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower5 == IPL_FASTATTACK) {
			if (ui.UIParam5a < 1 || ui.UIParam5b > 4) {
				app_fatal("Invalid UIParam5 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower5 == IPL_FASTRECOVER) {
			if (ui.UIParam5a < 1 || ui.UIParam5b > 3) {
				app_fatal("Invalid UIParam5 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower5 == IPL_FASTCAST) {
			if (ui.UIParam5a < 1 || ui.UIParam5b > 3) {
				app_fatal("Invalid UIParam5 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower5 == IPL_FASTWALK) {
			if (ui.UIParam5a < 1 || ui.UIParam5b > 3) {
				app_fatal("Invalid UIParam5 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower5 == IPL_DUR) {
			if (ui.UIParam5b > 200) {
				app_fatal("Invalid UIParam5 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower6 == IPL_FASTATTACK) {
			if (ui.UIParam6a < 1 || ui.UIParam6b > 4) {
				app_fatal("Invalid UIParam6 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower6 == IPL_FASTRECOVER) {
			if (ui.UIParam6a < 1 || ui.UIParam6b > 3) {
				app_fatal("Invalid UIParam6 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower6 == IPL_FASTCAST) {
			if (ui.UIParam6a < 1 || ui.UIParam6b > 3) {
				app_fatal("Invalid UIParam6 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower6 == IPL_FASTWALK) {
			if (ui.UIParam6a < 1 || ui.UIParam6b > 3) {
				app_fatal("Invalid UIParam6 set for '%s' %d.", ui.UIName, i);
			}
		}
		if (ui.UIPower6 == IPL_DUR) {
			if (ui.UIParam6b > 200) {
				app_fatal("Invalid UIParam6 set for '%s' %d.", ui.UIName, i);
			}
		}
	}
	assert(itemfiledata[ItemCAnimTbl[ICURS_MAGIC_ROCK]].iAnimLen == 10); // required by ProcessItems
	// objects
	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		const ObjFileData& od = objfiledata[i];
		if (od.oAnimFlag) {
			if (od.oAnimFrameLen <= 0)
				app_fatal("Invalid oAnimFrameLen %d for %s (%d)", od.oAnimFrameLen, od.ofName, i);
			if (od.oAnimLen <= 1) // required by SetupObject
				app_fatal("Invalid oAnimLen %d for %s (%d)", od.oAnimLen, od.ofName, i);
			if (od.oAnimLen >= 0x7FFF) // required by SetupObject
				app_fatal("Too high oAnimLen %d for %s (%d)", od.oAnimLen, od.ofName, i);
		}
	}
	// spells
	bool hasBookSpell = false, hasStaffSpell = false, hasScrollSpell = false, hasRuneSpell = false;
	for (i = 0; i < NUM_SPELLS; i++) {
		const SpellData& sd = spelldata[i];
		if (i == SPL_IDENTIFY || i == SPL_OIL || i == SPL_REPAIR || i == SPL_RECHARGE
#ifdef HELLFIRE
			|| i == SPL_BUCKLE || i == SPL_WHITTLE
#endif
		) {
			if (sd.scCurs > CURSOR_LAST_ITEMTGT)
				app_fatal("Invalid scCurs %d for %s (%d)", sd.scCurs, sd.sNameText, i);
			if (sd.spCurs > CURSOR_LAST_ITEMTGT)
				app_fatal("Invalid spCurs %d for %s (%d)", sd.spCurs, sd.sNameText, i);
		} else {
			if (sd.scCurs != CURSOR_NONE && sd.scCurs <= CURSOR_LAST_ITEMTGT)
				app_fatal("Invalid scCurs %d for %s (%d)", sd.scCurs, sd.sNameText, i);
			if (sd.spCurs != CURSOR_NONE && sd.spCurs <= CURSOR_LAST_ITEMTGT)
				app_fatal("Invalid spCurs %d for %s (%d)", sd.spCurs, sd.sNameText, i);
		}
		ItemStruct* is = NULL;
		if (SPELL_RUNE(i)) {
			if (sd.sBookLvl != SPELL_NA)
				app_fatal("Invalid sBookLvl %d for %s (%d)", sd.sBookLvl, sd.sNameText, i);
			if (sd.sStaffLvl != SPELL_NA)
				app_fatal("Invalid sStaffLvl %d for %s (%d)", sd.sStaffLvl, sd.sNameText, i);
			if (sd.sScrollLvl < RUNE_MIN)
				app_fatal("Invalid sScrollLvl %d for %s (%d)", sd.sScrollLvl, sd.sNameText, i);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d)", sd.sStaffCost, sd.sNameText, i);
			if (strlen(sd.sNameText) > sizeof(is->_iName) - (strlen("Rune of ") + 1))
				app_fatal("Too long name for %s (%d)", sd.sNameText, i);
			hasRuneSpell = true;
			continue;
		}
		if (sd.sBookLvl != SPELL_NA) {
			if (sd.sType != STYPE_NONE && sd.sType != STYPE_FIRE && sd.sType != STYPE_MAGIC && sd.sType != STYPE_LIGHTNING)
				app_fatal("Invalid sType %d for %s (%d)", sd.sType, sd.sNameText, i);
			if (sd.sBookLvl < BOOK_MIN)
				app_fatal("Invalid sBookLvl %d for %s (%d)", sd.sBookLvl, sd.sNameText, i);
			if (sd.sBookCost <= 0)
				app_fatal("Invalid sBookCost %d for %s (%d)", sd.sBookCost, sd.sNameText, i);
			if (strlen(sd.sNameText) > sizeof(is->_iName) - (strlen("Book of ") + 1))
				app_fatal("Too long name for %s (%d)", sd.sNameText, i);
			hasBookSpell = true;
		}
		if (sd.sStaffLvl != SPELL_NA) {
			if (sd.sStaffLvl < STAFF_MIN)
				app_fatal("Invalid sStaffLvl %d for %s (%d)", sd.sStaffLvl, sd.sNameText, i);
			if (sd.sStaffMin > sd.sStaffMax)
				app_fatal("Too high sStaffMin %d for %s (%d)", sd.sStaffMin, sd.sNameText, i);
			if (sd.sStaffMax - sd.sStaffMin >= 0x7FFF) // required by GetStaffSpell
				app_fatal("Too high sStaffMax %d for %s (%d)", sd.sStaffMin, sd.sNameText, i);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d)", sd.sStaffCost, sd.sNameText, i);
			//if (strlen(sd.sNameText) > sizeof(is->_iName) - (maxStaff + 4 + 1))
			if (strlen(sd.sNameText) > sizeof(is->_iName) - (strlen("Staff of ") + 1))
				app_fatal("Too long name for %s (%d)", sd.sNameText, i);
			hasStaffSpell = true;
		}
		if (sd.sScrollLvl != SPELL_NA) {
			if (sd.sScrollLvl < SCRL_MIN)
				app_fatal("Invalid sScrollLvl %d for %s (%d)", sd.sScrollLvl, sd.sNameText, i);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d)", sd.sStaffCost, sd.sNameText, i);
			if (strlen(sd.sNameText) > sizeof(is->_iName) - (strlen("Scroll of ") + 1))
				app_fatal("Too long name for %s (%d)", sd.sNameText, i);
			if ((sd.sSkillFlags & SDFLAG_TARGETED) && sd.scCurs == CURSOR_NONE)
				app_fatal("Targeted skill %s (%d) does not have scCurs.", sd.sNameText, i);
			hasScrollSpell = true;
		}
	}
	if (!hasBookSpell)
		app_fatal("No book spell for GetBookSpell.");
	if (!hasStaffSpell)
		app_fatal("No staff spell for GetStaffSpell.");
	if (!hasScrollSpell)
		app_fatal("No scroll spell for GetScrollSpell.");
	if (!hasRuneSpell)
		app_fatal("No rune spell for GetRuneSpell.");

	// missiles
	for (i = 0; i < NUM_MISTYPES; i++) {
		const MissileData &md = missiledata[i];
		if (md.mAddProc == NULL)
			app_fatal("Missile %d has no valid mAddProc.", i);
		if (md.mAddProc == AddMisexp) {
			for (int j = 0; j < misfiledata[md.mFileNum].mfAnimFAmt; j++) {
				assert(misfiledata[md.mFileNum].mfAnimFrameLen[j] == 1);
			}
		}
#ifdef HELLFIRE
		if (md.mAddProc == AddHorkSpawn) {
			for (int j = 0; j < misfiledata[md.mFileNum].mfAnimFAmt; j++) {
				assert(misfiledata[md.mFileNum].mfAnimFrameLen[j] == 1);
				assert(misfiledata[md.mFileNum].mfAnimLen[j] == 9);
			}
		}
#endif
		if (md.mProc == NULL)
			app_fatal("Missile %d has no valid mProc.", i);
		if (md.mProc == MI_Misexp || md.mProc == MI_MiniExp) {
			for (int j = 0; j < misfiledata[md.mFileNum].mfAnimFAmt; j++) {
				assert(misfiledata[md.mFileNum].mfAnimLen[j] < 16 /* lengthof(ExpLight) */);
			}
		}
		if (md.mDrawFlag) {
			if (md.mFileNum == MFILE_NONE && i != MIS_RHINO && i != MIS_CHARGE)
				app_fatal("Missile %d is drawn, but has no valid mFileNum.", i);
		} else {
			if (md.mFileNum != MFILE_NONE)
				app_fatal("Missile %d is not drawn, but has valid mFileNum.", i);
			if (md.miSFX != SFX_NONE)
				app_fatal("Missile %d is not drawn, but has valid miSFX.", i);
		}
	}
	assert(misfiledata[MFILE_LGHNING].mfAnimLen[0] == misfiledata[MFILE_THINLGHT].mfAnimLen[0]); // required by AddLightning
	assert(misfiledata[MFILE_FIREWAL].mfAnimFrameLen[0] == 1); // required by MI_Firewall
	assert(misfiledata[MFILE_FIREWAL].mfAnimLen[0] < 14 /* lengthof(FireWallLight) */); // required by MI_Firewall
	assert(missiledata[MIS_FIREWALL].mlSFX == LS_WALLLOOP); // required by MI_Firewall
	assert(missiledata[MIS_FIREWALL].mlSFXCnt == 1); // required by MI_Firewall
	assert(misfiledata[MFILE_RPORTAL].mfAnimLen[0] < 17 /* lengthof(ExpLight) */); // required by MI_Portal
	assert(misfiledata[MFILE_PORTAL].mfAnimLen[0] < 17 /* lengthof(ExpLight) */); // required by MI_Portal
	assert(misfiledata[MFILE_PORTAL].mfAnimLen[0] == misfiledata[MFILE_RPORTAL].mfAnimLen[0]); // required by MI_Portal
	assert(misfiledata[MFILE_PORTAL].mfAnimFrameLen[0] == 1); // required by MI_Portal
	assert(misfiledata[MFILE_RPORTAL].mfAnimFrameLen[0] == 1); // required by MI_Portal
	assert(misfiledata[MFILE_BLUEXFR].mfAnimFrameLen[0] == 1); // required by MI_Flash
	assert(misfiledata[MFILE_BLUEXBK].mfAnimFrameLen[0] == 1); // required by MI_Flash2
	assert(misfiledata[MFILE_FIREWAL].mfAnimLen[0] < 14 /* lengthof(FireWallLight) */); // required by MI_FireWave
	assert(misfiledata[MFILE_FIREWAL].mfAnimFrameLen[0] == 1); // required by MI_FireWave
	assert(misfiledata[MFILE_FIREBA].mfAnimFrameLen[0] == 1); // required by MI_Meteor
	assert(misfiledata[MFILE_GUARD].mfAnimFrameLen[0] == 1); // required by MI_Guardian
	assert(((1 + misfiledata[MFILE_GUARD].mfAnimLen[0]) >> 1) <= MAX_LIGHT_RAD); // required by MI_Guardian
	assert(misfiledata[MFILE_GUARD].mfAnimFrameLen[2] == 1); // required by MI_Guardian
	assert(misfiledata[MFILE_INFERNO].mfAnimLen[0] < 24); // required by MI_Inferno
	assert(misfiledata[missiledata[MIS_ACIDPUD].mFileNum].mfAnimFAmt < NUM_DIRS); // required by MI_Acidsplat
	assert(monfiledata[MOFILE_SNAKE].moAnimFrames[MA_ATTACK] == 13); // required by MI_Rhino
	assert(monfiledata[MOFILE_SNAKE].moAnimFrameLen[MA_ATTACK] == 1); // required by MI_Rhino

	// towners
	for (i = 0; i < STORE_TOWNERS; i++) {
		//const int(*gl)[2] = &GossipList[i];
		const int(&gl)[2] = GossipList[i];
		if (gl[0] > gl[1]) {
			app_fatal("Invalid GossipList (%d-%d) for %d", gl[0], gl[1], i);
		}
		if (gl[1] - gl[0] >= 0x7FFF) { // required by S_TalkEnter
			app_fatal("Too high GossipList range (%d-%d) for %d", gl[0], gl[1], i);
		}
	}
}
#endif /* DEBUG_MODE || DEV_MODE */

#if DEV_MODE
void LogErrorF(const char* type, const char* msg, ...)
{
	char tmp[256];
	//snprintf(tmp, sizeof(tmp), "f:\\logdebug%d_%d.txt", mypnum, SDL_ThreadID());
	snprintf(tmp, sizeof(tmp), "f:\\logdebug%d.txt", mypnum);
	FILE *f0 = fopen(tmp, "a+");
	if (f0 == NULL)
		return;

	va_list va;

	va_start(va, msg);

	vsnprintf(tmp, sizeof(tmp), msg, va);

	va_end(va);

	fputs(tmp, f0);

	using namespace std::chrono;
	milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
	//snprintf(tmp, sizeof(tmp), " @ %llu", ms.count());
	snprintf(tmp, sizeof(tmp), " @ %u", gdwGameLogicTurn);
	fputs(tmp, f0);

	fputc('\n', f0);

	fclose(f0);
}
#endif /* DEV_MODE */

DEVILUTION_END_NAMESPACE
