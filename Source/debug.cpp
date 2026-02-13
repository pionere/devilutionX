/**
 * @file debug.cpp
 *
 * Implementation of debug functions.
 */
#include <chrono>
#include "all.h"
#include "misproc.h"
#include "sfxdat.h"
#include "engine/render/text_render.h"
#include "dvlnet/packet.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

#if DEBUG_MODE
#define DEBUG_DATA
void CheckDungeonClear()
{
	int i, j;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (dMonster[i][j] != 0)
				app_fatal("Monsters not cleared");
			if (dPlayer[i][j] != 0)
				app_fatal("Players not cleared");
		}
	}
}

void DumpDungeon()
{
	FILE* f0 = FileOpen("f:\\dundump0.txt", "wb");
	FILE* f1 = FileOpen("f:\\dundump1.txt", "wb");
	FILE* f2 = FileOpen("f:\\dundump2.txt", "wb");
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
/*static const char* ReadTextLine(const char* str, char lineSep, int limit)
{
	int w;
	BYTE c;

	c = 0;
	w = 0;
	while (*str != lineSep && w < limit) {
		tempstr[c] = *str;
		w += smallFontWidth[gbStdFontFrame[(BYTE)tempstr[c]]] + FONT_KERN_SMALL;
		c++;
		str++;
	}
	if (w >= limit) {
		c--;
		while (tempstr[c] != ' ') {
			str--;
			c--;
		}
	}
	tempstr[c] = '\0';
	return str;
}

static void PrintText(const char* text, char lineSep, int limit)
{
	const char* s = text;
	// int i = 0, w;
	BYTE col;
	FILE* textFile = FileOpen("f:\\sample.txt", "wb");

	while (*s != '\0') {
		if (*s == '$') {
			s++;
			col = COL_RED;
		} else {
			col = COL_WHITE;
		}
		s = ReadTextLine(s, lineSep, limit);
		// w = GetSmallStringWidth(tempstr);

		// LogErrorF("%03d(%04d):%s", i++, w, tempstr);
		if (col == COL_RED)
			fputc('$', textFile);
		fputs(tempstr, textFile);
		fputc('\n', textFile);
		if (*s == lineSep) {
			s++;
		}
	}

	fclose(textFile);
}*/
#ifdef DEBUG_DATA
/* copy-paste from monster.cpp */
static bool IsSkel(int mt)
{
	return (mt >= MT_WSKELAX && mt <= MT_XSKELAX)
		|| (mt >= MT_WSKELBW && mt <= MT_XSKELBW)
		|| (mt >= MT_WSKELSD && mt <= MT_XSKELSD);
}

static int MonsterAiMissile(const MonsterAI &mai)
{
	if (mai.aiType == AI_ROUNDRANGED || mai.aiType == AI_RANGED || mai.aiType == AI_ROUNDRANGED2
		|| mai.aiType == AI_COUNSLR || mai.aiType == AI_ZHAR || mai.aiType == AI_LAZARUS) {
		int mm = mai.aiParam1;
		if (mm == MIS_LIGHTNINGC2)
			mm = MIS_LIGHTNING2;
		if (mm == MIS_LIGHTNINGC)
			mm = MIS_LIGHTNING;
		if (mm == MIS_APOCAC2)
			mm = MIS_EXAPOCA2;
		if (mm == MIS_INFERNOC)
			mm = MIS_INFERNO;
		if (mm == MIS_CBOLTC)
			mm = MIS_CBOLT;
		if (missiledata[mm].mFileNum == MFILE_NONE || missiledata[mm].mFileNum == MFILE_ACTOR)
			app_fatal("Unresolved ai-missile %d", mm);
		return mm;
	}
	return -1;
}

static bool MonsterAiSpecial(const MonsterAI &mai, int animNum)
{
	if (mai.aiType == AI_FALLEN || mai.aiType == AI_SKELKING || mai.aiType == AI_ROUNDRANGED || mai.aiType == AI_ROUNDRANGED2
		|| mai.aiType == AI_SNEAK || mai.aiType == AI_SCAV || mai.aiType == AI_MAGE || mai.aiType == AI_GARG || mai.aiType == AI_FAT
		|| mai.aiType == AI_COUNSLR || mai.aiType == AI_ZHAR || mai.aiType == AI_LAZARUS
#ifdef HELLFIRE
		|| mai.aiType == AI_HORKDMN
#endif
		|| ((mai.aiType == AI_SNAKE || mai.aiType == AI_RHINO || mai.aiType == AI_BAT) && animNum == MOFILE_RHINO)
		|| (mai.aiType == AI_RANGED && mai.aiParam2)
		|| (mai.aiType == AI_ROUND && mai.aiParam1))

		return true;

	return false;
}

static BYTE GetUniqueItemPower(const UniqItemData& ui, int index)
{
	switch (index) {
	case 1: return ui.UIPower1;
	case 2: return ui.UIPower2;
	case 3: return ui.UIPower3;
	case 4: return ui.UIPower4;
	case 5: return ui.UIPower5;
	case 6: return ui.UIPower6;
	default: ASSUME_UNREACHABLE; return 0;
	}
}

static int GetUniqueItemParamA(const UniqItemData& ui, int index)
{
	switch (index) {
	case 1: return ui.UIParam1a;
	case 2: return ui.UIParam2a;
	case 3: return ui.UIParam3a;
	case 4: return ui.UIParam4a;
	case 5: return ui.UIParam5a;
	case 6: return ui.UIParam6a;
	default: ASSUME_UNREACHABLE; return 0;
	}
}

static int GetUniqueItemParamB(const UniqItemData& ui, int index)
{
	switch (index) {
	case 1: return ui.UIParam1b;
	case 2: return ui.UIParam2b;
	case 3: return ui.UIParam3b;
	case 4: return ui.UIParam4b;
	case 5: return ui.UIParam5b;
	case 6: return ui.UIParam6b;
	default: ASSUME_UNREACHABLE; return 0;
	}
}

static bool HasUniqueItemReq(const UniqItemData& ui, BYTE pow)
{
	int i, dv = 0;
	for (i = 1; i < 6; i++) {
		if (pow == IPL_STR && GetUniqueItemPower(ui, i) == IPL_REQSTR) {
			dv = GetUniqueItemParamA(ui, i);
			break;
		}
	}
	for (i = 0; i < NUM_IDI; i++) {
		const ItemData& ids = AllItemList[i];
		if (ids.iUniqType == ui.UIUniqType) {
			int minv;
			switch (pow) {
			case IPL_STR: minv = ids.iMinStr; break;
			case IPL_MAG: minv = ids.iMinMag; break;
			case IPL_DEX: minv = ids.iMinDex; break;
			default:ASSUME_UNREACHABLE; break;
			}
			if (minv + dv != 0)
				return true;
		}
	}
	return false;
}

#define MIS_VELO_SHIFT      0
#define MIS_BASE_VELO_SHIFT 16
#define MIS_SHIFTEDVEL(x)   ((x) << MIS_VELO_SHIFT)

/*static bool lessCrawlTableEntry(const POS32 *a, const POS32 *b)
{
	if (abs(a->y) != abs(b->y))
		return abs(a->y) > abs(b->y);
	if (abs(a->x) != abs(b->x))
		return abs(a->x) < abs(b->x);
	if (a->y != b->y) {
		return a->y > b->y;
	}
	return a->x < b->x;
}

static bool lessCrawlTableEntryDist(const POS32 *a, const POS32 *b)
{
	int da = a->x * a->x + a->y * a->y;
	int db = b->x * b->x + b->y * b->y;
	return da < db;
}

static bool lessCrawlTableEntryClockWise(const POS32 *a, const POS32 *b)
{
	int clockA = (a->x < 0 ? (a->y < 0 ? 3 : 2) : (a->y < 0 ? 0 : 1));
	int clockB = (b->x < 0 ? (b->y < 0 ? 3 : 2) : (b->y < 0 ? 0 : 1));
	if (clockA != clockB)
		return clockA < clockB;
	if (clockA & 1) {
		if (clockA >= 2) {
			if (a->x != b->x)
				return a->x < b->x;
			return a->y > b->y;
		} else {
			if (a->x != b->x)
				return a->x > b->x;
			return a->y < b->y;
		}
	} else {
		if (clockA >= 2) {
			if (a->x != b->x)
				return a->x > b->x;
			return a->y > b->y;
		} else {
			if (a->x != b->x)
				return a->x < b->x;
			return a->y < b->y;
		}
	}
}

static void sortCrawlTable(POS32 *table, unsigned entries, bool (cmpFunc)(const POS32 *a, const POS32 *b))
{
	if (entries <= 1)
		return;
	sortCrawlTable(&table[0], entries - entries / 2, cmpFunc);
	sortCrawlTable(&table[entries - entries / 2], entries / 2, cmpFunc);

	unsigned cl = 0;
	unsigned cr = entries - entries / 2;
	unsigned el = cr;
	unsigned er = entries;

	POS32 *tmp = (POS32 *)malloc(sizeof(table[0]) * entries);

	for (unsigned i = 0; i < entries; i++) {
		if (cr >= er || (cl < el && !cmpFunc(&table[cr], &table[cl]))) {
			tmp[i] = table[cl];
			cl++;
		} else {
			tmp[i] = table[cr];
			cr++;
		}
	}
	memcpy(table, tmp, sizeof(table[0]) * entries);
}

static void recreateCrawlTable()
{
	constexpr int version = 2;
	constexpr int r = version == 0 ? 18 : 15;
	int crns[r + 1];
	memset(crns, 0, sizeof(crns));
	POS32 ctableentries[r + 1][2 * 2 * r * 4];
	memset(ctableentries, 0x80, sizeof(ctableentries));
	int dx, dy, tx, ty, dist, total = 0;
	dx = r + 1;
	dy = r + 1;
	for (tx = dx - r; tx <= dx + r; tx++) {
		for (ty = dx - r; ty <= dy + r; ty++) {
			dist = std::max(abs(tx - dx), abs(ty - dy));
			if (abs(tx - dx) == abs(ty - dy) && (tx != dx || ty != dy))
				dist++;
			if (version != 0 && dist > 3) {
				dist = (tx - dx) * (tx - dx) + (ty - dy) * (ty - dy);
				dist = (int)(sqrt((double)dist) + 0.5f);
				// if (dist == 1 && (tx != dx || ty != dy))
				//	dist++;
			}

			if (dist <= r) {
				ctableentries[dist][crns[dist]].x = tx - dx;
				ctableentries[dist][crns[dist]].y = ty - dy;
				crns[dist]++;
				total++;
			}
		}
	}
	for (int n = 0; n <= r; n++) {
		if (version == 0)
			sortCrawlTable(ctableentries[n], crns[n], lessCrawlTableEntry);
		if (version == 1)
			sortCrawlTable(ctableentries[n], crns[n], lessCrawlTableEntryDist);
		if (version == 2)
			sortCrawlTable(ctableentries[n], crns[n], n == r ? lessCrawlTableEntryClockWise : lessCrawlTableEntryDist);
	}
	LogErrorF("const int8_t CrawlTable[%d] = {", total * 2 + r + 1);
	LogErrorF("	// clang-format off");
	int cursor = 0;
	for (int n = 0; n <= r; n++) {
		int ii = crns[n];
		if (ii < 100)
			LogErrorF("	%d,\t\t\t\t\t\t\t\t\t\t// %2d - %d", ii, n, cursor);
		else if (ii < 128)
			LogErrorF("	%d,\t\t\t\t\t\t\t\t\t// %2d - %d", ii, n, cursor);
		else
			LogErrorF("	(int8_t)%d,\t\t\t\t\t\t\t// %2d - %d", ii, n, cursor);
		cursor++;
		cursor += 2 * ii;
		tempstr[0] = '\0';
		for (int i = 0; i < ii; i++) {
			if ((i % 4) == 0) {
				copy_cstr(tempstr, "\t");
			}
			snprintf(tempstr, sizeof(tempstr), "%s%3d,%3d,  ", tempstr, ctableentries[n][i].x, ctableentries[n][i].y);
			if ((i % 4) == 3) {
				tempstr[strlen(tempstr) - 2] = '\0';
				LogErrorF(tempstr);
				tempstr[0] = '\0';
			}
		}
		if (tempstr[0] != '\0') {
			tempstr[strlen(tempstr) - 2] = '\0';
			LogErrorF(tempstr);
			tempstr[0] = '\0';
		}
	}
	LogErrorF("	// clang-format on");
	LogErrorF("};");

	snprintf(tempstr, sizeof(tempstr), "const int CrawlNum[%d] = {", r + 1);
	cursor = 0;
	for (int n = 0; n <= r; n++) {
		snprintf(tempstr, sizeof(tempstr), "%s %d,", tempstr, cursor);
		cursor++;
		cursor += 2 * crns[n];
	}
	tempstr[strlen(tempstr) - 1] = '\0';
	snprintf(tempstr, sizeof(tempstr), "%s };", tempstr);
	LogErrorF(tempstr);
}*/
#endif // DEBUG_DATA
void ValidateData()
{
#ifdef DEBUG_DATA
	int i;
#endif
#if !defined(NONET) && DEBUG_MODE
	// dvlnet
	{
		net::packet_factory pktfty;
		plr_t plr_self = 0;
		plr_t plr_other = 1;
		plr_t plr_mask = (1 << 0) | (1 << 1);
		net::packet* pkt;
		turn_t turn = 0;
		cookie_t cookie = 123456;
		const BYTE dynData[16] = "lwkejfwip";
		const BYTE (&addr)[16] = dynData;
		const BYTE (&addrs)[16] = dynData;
		SNetGameData gameData;
#ifdef ZEROTIER
		SNetZtGame ztGameData;
#endif
		pkt = pktfty.make_out_packet<net::PT_MESSAGE>(plr_self, net::PLR_BROADCAST, dynData, (unsigned)sizeof(dynData));
		if (!pkt->validate()) {
			app_fatal("PT_MESSAGE is invalid");
		}
		delete pkt;
		pkt = pktfty.make_out_packet<net::PT_TURN>(plr_self, net::PLR_BROADCAST, turn, dynData, (unsigned)sizeof(dynData));
		if (!pkt->validate()) {
			app_fatal("PT_TURN is invalid");
		}
		delete pkt;
		pkt = pktfty.make_out_packet<net::PT_JOIN_REQUEST>(plr_self, net::PLR_BROADCAST, cookie);
		if (!pkt->validate()) {
			app_fatal("PT_JOIN_REQUEST is invalid");
		}
		delete pkt;
		pkt = pktfty.make_out_packet<net::PT_JOIN_ACCEPT>(net::PLR_MASTER, net::PLR_BROADCAST, cookie, plr_other, (const BYTE*)&gameData, plr_mask, turn, addrs, (unsigned)sizeof(addrs));
		if (!pkt->validate()) {
			app_fatal("PT_JOIN_ACCEPT is invalid");
		}
		delete pkt;
		pkt = pktfty.make_out_packet<net::PT_CONNECT>(plr_self, net::PLR_BROADCAST, net::PLR_MASTER, turn, addr, (unsigned)sizeof(addr));
		if (!pkt->validate()) {
			app_fatal("PT_CONNECT is invalid");
		}
		delete pkt;
		pkt = pktfty.make_out_packet<net::PT_DISCONNECT>(plr_self, net::PLR_BROADCAST, plr_other);
		if (!pkt->validate()) {
			app_fatal("PT_DISCONNECT is invalid");
		}
		delete pkt;
#ifdef ZEROTIER
		pkt = pktfty.make_out_packet<net::PT_INFO_REQUEST>(plr_self, net::PLR_BROADCAST);
		if (!pkt->validate()) {
			app_fatal("PT_INFO_REQUEST is invalid");
		}
		delete pkt;
		pkt = pktfty.make_out_packet<net::PT_INFO_REPLY>(plr_self, plr_other, (const BYTE*)&ztGameData);
		if (!pkt->validate()) {
			app_fatal("PT_INFO_REPLY is invalid");
		}
		delete pkt;
#endif
	}
#endif // !NONET && DEBUG_MODE
	// text
	//PrintText(gszHelpText, '|', LTPANEL_WIDTH - 2 * 7);
#ifdef DEBUG_DATA
	for (i = 0; i < lengthof(gbStdFontFrame); i++) {
		if (gbStdFontFrame[i] >= lengthof(smallFontWidth))
			app_fatal("Width of the small font %d ('%c') is undefined (frame number: %d).", i, i, gbStdFontFrame[i]);
	}
	for (i = 0; i < lengthof(smallFontWidth); i++) {
		if (smallFontWidth[i] > 13)
			app_fatal("Width of the small font %d is too high.", i); // required by DrawSkillIcons
	}

	if (GetHugeStringWidth("Pause") != 135)
		app_fatal("DrawPause expects hardcoded width 135.");

	// cursors
	for (i = 0; i < lengthof(InvItemWidth); i++) {
		if (i != CURSOR_NONE && InvItemWidth[i] == 0)
			app_fatal("Invalid (zero) cursor width at %d.", i);
	}
	for (i = 0; i < lengthof(InvItemHeight); i++) {
		if (i != CURSOR_NONE && InvItemHeight[i] == 0)
			app_fatal("Invalid (zero) cursor height at %d.", i);
	}
	// meta-data
	for (i = 0; i < NUM_DIRS; i++) {
		int tx = DBORDERY, ty = DBORDERY;
		int newx = tx + offset_x[i];
		int newy = ty + offset_y[i];
		assert(OPPOSITE(i) == GetDirection(newx, newy, tx, ty)); // required by MonTeleport
	}
	// recreateCrawlTable();
	// - CrawlNum
	for (int n = 0; n < lengthof(CrawlNum); n++) {
		int a = CrawlNum[n];
		int e = CrawlTable[a];
		int b;
		if (n < lengthof(CrawlNum) - 1) {
			b = CrawlNum[n + 1];
		} else {
			b = lengthof(CrawlTable);
		}
		if (a + 1 + 2 * e != b)
			app_fatal("CrawlNum mismatch %d vs %d (idx=%d)", a + 1 + 2 * e, b, n);
	}
	// - CrawlTable
	for (int i = 0; i < lengthof(CrawlTable); i++) {
		int k = 0;
		for (; k < lengthof(CrawlNum); k++) {
			if (CrawlNum[k] == i)
				break;
		}
		if (k >= lengthof(CrawlNum)) {
			for (int n = i + 2; n < lengthof(CrawlTable); n++) {
				k = 0;
				for (; k < lengthof(CrawlNum); k++) {
					if (CrawlNum[k] == n)
						break;
				}
				if (k >= lengthof(CrawlNum)) {
					if (CrawlTable[n] == CrawlTable[i] && CrawlTable[n + 1] == CrawlTable[i + 1])
						app_fatal("Duplicate entry (%d:%d) in CrawlTable (@ %d and %d)", CrawlTable[n], CrawlTable[n + 1], n, i);
					n++;
				}
			}
			i++;
		}
	}
	assert(CrawlTable[CrawlNum[4]] == 24); // required by MAI_Scav
	assert(CrawlTable[CrawlNum[3]] == 24); // required by AddNovaC
	{	// sfx
		static const int snSFX[3][NUM_CLASSES] = {
			// clang-format off
#ifdef HELLFIRE
			{ PS_WARR52, PS_ROGUE52, PS_MAGE52, PS_MONK52, PS_ROGUE52, PS_WARR52 },
			{ PS_WARR49, PS_ROGUE49, PS_MAGE49, PS_MONK49, PS_ROGUE49, PS_WARR49 },
			{ PS_WARR50, PS_ROGUE50, PS_MAGE50, PS_MONK50, PS_ROGUE50, PS_WARR50 },
#else
			{ PS_WARR52, PS_ROGUE52, PS_MAGE52 },
			{ PS_WARR49, PS_ROGUE49, PS_MAGE49 },
			{ PS_WARR50, PS_ROGUE50, PS_MAGE50 },
#endif
			// clang-format on
		};
		for (i = 0; i < NUM_CLASSES; i++) {
			assert(sfxdata[sgSFXSets[SFXS_PLR_08][i]].bFlags & sfx_STREAM); // required by TalkToTowner
			assert(sfxdata[sgSFXSets[SFXS_PLR_09][i]].bFlags & sfx_STREAM); // required by TalkToTowner
			for (int n = 0; n < lengthof(snSFX); n++) {
				assert(sfxdata[snSFX[n][i]].bFlags & sfx_STREAM); // required by CowSFX
			}
		}
		for (i = 0; i < lengthof(minitxtdata); i++) {
			int n = minitxtdata[i].sfxnr;
			if (minitxtdata[i].txtstr && minitxtdata[i].txtstr[0] == '\0')
				app_fatal("Scrolling text of minitext %d is empty.", n, i);
			if (minitxtdata[i].txtsfxset) {
				if ((unsigned)n >= NUM_SFXS)
					app_fatal("Sfx-set (%d) of minitext %d is invalid (%s).", n, i, minitxtdata[i].txtstr == NULL ? "(null)" : minitxtdata[i].txtstr);
				for (int c = 0; c < NUM_CLASSES; c++) {
					if (!(sfxdata[sgSFXSets[n][c]].bFlags & sfx_STREAM))
						app_fatal("Sfx-set (%d) of minitext %d is not streamed (%s) for class %d.", n, i, minitxtdata[i].txtstr == NULL ? "(null)" : minitxtdata[i].txtstr, c); // required by MonDoTalk, SpawnLoot
				}
			} else {
				if ((unsigned)n >= NUM_SFXS)
					app_fatal("Sfx (%d) of minitext %d is invalid (%s).", n, i, minitxtdata[i].txtstr == NULL ? "(null)" : minitxtdata[i].txtstr);
				if (!(sfxdata[n].bFlags & sfx_STREAM))
					app_fatal("Sfx (%d) of minitext %d is not streamed (%s).", n, i, minitxtdata[i].txtstr == NULL ? "(null)" : minitxtdata[i].txtstr); // required by MonDoTalk, SpawnLoot
			}
		}
		assert(sfxdata[USFX_GARBUD4].bFlags & sfx_STREAM); // required by MAI_Garbud
		assert(sfxdata[USFX_ZHAR2].bFlags & sfx_STREAM);   // required by MAI_Zhar
		assert(sfxdata[USFX_SNOT3].bFlags & sfx_STREAM);   // required by MAI_SnotSpil
		assert(sfxdata[USFX_LAZ1].bFlags & sfx_STREAM);    // required by MAI_Lazarus
		assert(sfxdata[USFX_LACH3].bFlags & sfx_STREAM);   // required by MAI_Lachdanan
		assert(sfxdata[USFX_WARLRD1].bFlags & sfx_STREAM); // required by MAI_Warlord
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
		if (AllLevels[i].dMonDensity > UINT_MAX / (DSIZEX * DSIZEY))
			app_fatal("Too high dMonDensity on level %s (%d)", AllLevels[i].dLevelName, i); // required by InitMonsters
	}
#endif // DEBUG_DATA
	{
	BYTE lvlMask = 1 << AllLevels[questlist[Q_BLOOD]._qdlvl].dType;
	assert(objectdata[OBJ_TORCHL1].oLvlTypes & lvlMask); // required by SyncPedestal
	assert(objectdata[OBJ_TORCHL2].oLvlTypes & lvlMask); // required by SyncPedestal
	}
	// monsters
	assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_KNOCKBACK)); // required by MonHitByMon
	assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_BLEED)); // required by MonHitByMon and MonHitByPlr
	assert(monsterdata[MT_GOLEM].mSelFlag == 0); // required by CheckCursMove
	assert(monsterdata[MT_GBAT].mAI.aiType == AI_BAT); // required by MAI_Bat
#ifdef HELLFIRE
	assert(missiledata[MIS_HORKDMN].mFileNum == MFILE_SPAWNS); // required by MAI_Horkdemon/InitMonsterGFX
#endif
#ifdef DEBUG_DATA
	for (i = 0; i < NUM_MTYPES; i++) {
		const MonsterData& md = monsterdata[i];
		if (strlen(md.mName) > sizeof(infostr) - 1)
			app_fatal("Too long name for %s, %d (maximum is %d).", md.mName, i, sizeof(infostr)); // required by DrawInfoStr
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
		if (md.mAI.aiType == AI_GARG && !(md.mFlags & MFLAG_NOSTONE))
			app_fatal("AI_GARG might override stoned state (%s, %d)", md.mName, i); // required by MAI_Garg
		if (md.mFlags & MFLAG_GARG_STONE) {
			if (md.mFlags & MFLAG_HIDDEN)
				app_fatal("Both GARG_STONE and HIDDEN flags are set for %s (%d).", md.mName, i); // required ProcessMonsters
			if (md.mAI.aiType != AI_GARG)
				app_fatal("GARG_STONE flag is not supported by the AI of %s (%d).", md.mName, i);
		}
		// check missile animations of the monsters
		int mm = MonsterAiMissile(md.mAI);
		if (mm >= 0) {
			bool hiddenAnim = missiledata[mm].mFileNum >= NUM_FIXMFILE;
			if (i == MT_NMAGMA || i == MT_YMAGMA || i == MT_BMAGMA || i == MT_WMAGMA) {
				if (missiledata[mm].mFileNum != MFILE_MAGBALL)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_RTHIN || i == MT_NTHIN || i == MT_XTHIN || i == MT_GTHIN) {
				if (missiledata[mm].mFileNum != MFILE_THINLGHT)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_NACID || i == MT_RACID || i == MT_BACID || i == MT_XACID
#ifdef HELLFIRE
					|| i == MT_SPIDLORD
#endif
				) {
				if (missiledata[mm].mFileNum != MFILE_ACIDBF)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_ACID) // MFILE_ACIDSPLA, MFILE_ACIDPUD ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_ACID, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_GSUCC) {
				if (missiledata[mm].mFileNum != MFILE_SCUBMISB)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_SNOWWICH) // MFILE_SCBSEXPB ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_SNOWWICH, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_RSUCC) {
				if (missiledata[mm].mFileNum != MFILE_SCUBMISD)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_HLSPWN) // MFILE_SCBSEXPD ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_HLSPWN, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_BSUCC) {
				if (missiledata[mm].mFileNum != MFILE_SCUBMISC)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_SOLBRNR) // MFILE_SCBSEXPC ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_SOLBRNR, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_SMAGE || i == MT_OMAGE) {
				if (missiledata[mm].mFileNum != MFILE_MAGEMIS)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_MAGE) // MFILE_MAGEEXP ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_MAGE, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_DIABLO) {
				if (missiledata[mm].mFileNum != MFILE_FIREPLAR)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
#ifdef HELLFIRE
			} else if (i == MT_SKLWING) {
				/*if (mm != MIS_BONEDEMON) // MFILE_EXORA1_B ?
					app_fatal("AI_...");
				if (missiledata[mm].mFileNum != MFILE_MS_ORA_B)
					app_fatal("AI_...");*/
			} else if (i == MT_BONEDEMN) {
				if (missiledata[mm].mFileNum != MFILE_MS_ORA_B)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_BONEDEMON) // MFILE_EXORA1_B ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_BONEDEMON, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_PSYCHORB) {
				if (missiledata[mm].mFileNum != MFILE_MS_ORA)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_PSYCHORB) // MFILE_EXORA1 ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_PSYCHORB, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_NECRMORB) {
				if (missiledata[mm].mFileNum != MFILE_MS_REB_B)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_NECROMORB) // MFILE_EXYEL2_B ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_NECROMORB, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_LICH) {
				if (missiledata[mm].mFileNum != MFILE_MS_ORA_A)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_LICH) // MFILE_EXORA1_A ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_LICH, md.mName, i); // required by InitMonsterGFX
			} else if (i == MT_ARCHLICH) {
				if (missiledata[mm].mFileNum != MFILE_MS_YEB_A)
					app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
				else if (!hiddenAnim)
					app_fatal("Missile %d animation for monster %s (%d) is no longer optional.", mm, md.mName, i); // required by InitMonsterGFX
				if (mm != MIS_ARCHLICH) // MFILE_EXYEL2_A ?
					app_fatal("Explosion for Missile %d animation for monster %s (%d) is no longer necessary.", MIS_ARCHLICH, md.mName, i); // required by InitMonsterGFX
#endif
			} else if (missiledata[mm].mFileNum >= NUM_FIXMFILE) {
				app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", mm, md.mName, i); // required by InitMonsterGFX
			}
#ifdef HELLFIRE
		} else if (md.mAI.aiType == AI_HORKDMN) {
			if (i != MT_HORKDMN)
				app_fatal("Missile %d animation for monster %s (%d) might not be loaded.", MFILE_SPAWNS, md.mName, i); // required by InitMonsterGFX
#endif
		}
		if (md.mAI.aiInt > UINT8_MAX - HELL_LEVEL_BONUS / 16) // required by InitMonsterStats
			app_fatal("Too high aiInt %d for %s (%d).", md.mLevel, md.mName, i);
		if (md.mAI.aiType == AI_MAGE && md.mAI.aiInt < 1)// required by MAI_Mage (RETREAT)
			app_fatal("Too low aiInt %d for %s (%d).", md.mLevel, md.mName, i);
		if (md.mLevel == 0) // required by InitMonsterStats
			app_fatal("Invalid mLevel %d for %s (%d).", md.mLevel, md.mName, i);
		if (md.mLevel > UINT8_MAX - HELL_LEVEL_BONUS) // required by InitMonsterStats
			app_fatal("Too high mLevel %d for %s (%d).", md.mLevel, md.mName, i);
		if (md.mLevel + HELL_LEVEL_BONUS > CF_LEVEL && (md.mFlags & MFLAG_NODROP) == 0)
			app_fatal("Invalid mLevel %d for %s (%d). Too high to set the level of item-drop.", md.mLevel, md.mName, i);
		if (md.moFileNum == MOFILE_DIABLO && !(md.mFlags & MFLAG_NOCORPSE))
			app_fatal("MOFILE_DIABLO does not have corpse animation but MFLAG_NOCORPSE is not set for %s (%d).", md.mName, i);
		if (md.moFileNum == MOFILE_GOLEM && i != MT_GOLEM)
			app_fatal("Animation is not initialized properly for %s (%d).", md.mName, i); // required by InitMonsterGFX
		if (lengthof(monfiledata) <= md.moFileNum)
			app_fatal("Invalid moFileNum %d for %s (%d). Must not be more than %d.", md.mLevel, md.mName, i, lengthof(monfiledata));
		const MonFileData& mfd = monfiledata[md.moFileNum];
		if (mfd.moAnimFrameLen[MA_STAND] == 0) {
			app_fatal("Missing stand animation for %s (%d).", md.mName, i); // required by InitMonster, etc...
		}
		if (mfd.moAnimFrameLen[MA_WALK] == 0) {
			app_fatal("Missing walk animation for %s (%d).", md.mName, i); // required by MAI_*, etc...
		}
		if (mfd.moAnimFrameLen[MA_ATTACK] == 0) {
			app_fatal("Missing attack animation for %s (%d).", md.mName, i); // required by MAI_*, etc...
		}
		if (md.mFlags & MFLAG_NOGETHIT) {
			if (mfd.moAnimFrameLen[MA_GOTHIT] != 0) {
				app_fatal("Unused got-hit animation for %s (%d).", md.mName, i);
			}
		} else {
			if (mfd.moAnimFrameLen[MA_GOTHIT] == 0) {
				app_fatal("Missing got-hit animation for %s (%d).", md.mName, i); // required by MonHitByPlr / MonHitByMon
			}
		}
		if (mfd.moAnimFrameLen[MA_DEATH] == 0) {
			app_fatal("Missing death animation for %s (%d).", md.mName, i); // required by MonInitKill
		}
		BYTE afnumReq = 0, altDamReq = 0;
		if (md.mAI.aiType == AI_ROUNDRANGED || md.mAI.aiType == AI_ROUNDRANGED2 || (md.mAI.aiType == AI_RANGED && md.mAI.aiParam2) // required for MonStartRSpAttack / MonDoRSpAttack
#ifdef HELLFIRE
		  || md.mAI.aiType == AI_HORKDMN
#endif
			) {
			afnumReq |= 2;
		}
		if (md.mAI.aiType == AI_FAT || (md.mAI.aiType == AI_ROUND && md.mAI.aiParam1) || md.mAI.aiType == AI_GARBUD || md.mAI.aiType == AI_SCAV || md.mAI.aiType == AI_GARG) { // required for MonStartSpAttack / MonDoSpAttack
			afnumReq |= 1;
		}
		if ((md.mAI.aiType == AI_FALLEN || md.mAI.aiType == AI_GOLUM || IsSkel(i)) && mfd.moSndSpecial) { // required for MonStartSpStand
			afnumReq |= 2;
		}
		if (md.mAI.aiType == AI_RHINO || md.mAI.aiType == AI_SNAKE) { // required for MissToMonst
			altDamReq |= 1; // requires mMaxDamage2
		}
		if (afnumReq != 0) {
			if (afnumReq & 2) {
				// moAFNum2 required
				if (mfd.moAFNum2 == 0) {
					app_fatal("moAFNum2 is not set for %s (%d).", md.mName, i);
				}
			}
			if (afnumReq & 1) {
				// moAFNum2 optional
				if (mfd.moAFNum2 != 0) {
					altDamReq |= 3; // requires mMaxDamage2 and mHit2
				}
			}
		} else {
#if DEV_MODE
			if (mfd.moAFNum2 != 0)
				LogErrorF("moAFNum2 is set for %s (%d), but it is not used.", md.mName, i);
#endif
		}
		if (altDamReq != 0) {
			if (altDamReq & 1) {
				if (md.mMaxDamage2 == 0)
					app_fatal("mMaxDamage2 is not set for %s (%d).", md.mName, i);
			}
			if (altDamReq & 2) {
				if (md.mHit2 == 0)
					app_fatal("mHit2 is not set for %s (%d).", md.mName, i);
			}
		} else {
#if DEV_MODE
			if (md.mHit2 != 0)
				LogErrorF("mHit2 is set for %s (%d), but it is not used.", md.mName, i);
			if (md.mMaxDamage2 != 0)
				LogErrorF("mMaxDamage2 is set (%d) for %s (%d), but it is not used.", md.mMaxDamage2, md.mName, i);
#endif
		}
		if (md.mHit > INT_MAX /*- HELL_TO_HIT_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitMonsterStats
			app_fatal("Too high mHit %d for %s (%d).", md.mHit, md.mName, i);
		if (md.mHit2 > INT_MAX /*- HELL_TO_HIT_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitMonsterStats
			app_fatal("Too high mHit2 %d for %s (%d).", md.mHit2, md.mName, i);
		if (md.mMagic > INT_MAX /*- HELL_MAGIC_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitMonsterStats
			app_fatal("Too high mMagic %d for %s (%d).", md.mMagic, md.mName, i);
		if (md.mMaxDamage == 0)
			app_fatal("mMaxDamage is not set for %s (%d).", md.mName, i);
		if (md.mMaxDamage2 == 0 && (md.mAI.aiType == AI_ROUND || md.mAI.aiType == AI_FAT || md.mAI.aiType == AI_RHINO || md.mAI.aiType == AI_SNAKE))
			app_fatal("mMaxDamage2 is not set for %s (%d).", md.mName, i);
		if (md.mMaxDamage2 != 0 && (md.mAI.aiType == AI_SCAV || md.mAI.aiType == AI_GARG))
			app_fatal("Fake special attack of %s (%d) might hurt someone because mMaxDamage2 is set.", md.mName, i);
		if (md.mMinDamage > md.mMaxDamage)
			app_fatal("Too high mMinDamage %d for %s (%d).", md.mMinDamage, md.mName, i);
		if (md.mMinDamage2 > md.mMaxDamage2)
			app_fatal("Too high mMinDamage2 %d for %s (%d).", md.mMinDamage2, md.mName, i);
		if (md.mMaxDamage > INT_MAX / (md.mLevel + HELL_LEVEL_BONUS)) // required by InitMonsterStats
			app_fatal("Too high mMaxDamage %d for %s (%d).", md.mMaxDamage, md.mName, i);
		if (md.mMaxDamage2 > INT_MAX / (md.mLevel + HELL_LEVEL_BONUS)) // required by InitMonsterStats
			app_fatal("Too high mMaxDamage2 %d for %s (%d).", md.mMaxDamage2, md.mName, i);
		if (md.mArmorClass > INT_MAX /*- HELL_AC_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitMonsterStats
			app_fatal("Too high mArmorClass %d for %s (%d).", md.mArmorClass, md.mName, i);
		if (md.mEvasion > INT_MAX /*- HELL_EVASION_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitMonsterStats
			app_fatal("Too high mEvasion %d for %s (%d).", md.mEvasion, md.mName, i);
		if (md.mMinHP <= 0)
			app_fatal("Invalid mMinHP %d for %s (%d)", md.mMinHP, md.mName, i);
		if (md.mMinHP > md.mMaxHP)
			app_fatal("Too high mMinHP %d for %s (%d)", md.mMinHP, md.mName, i);
		if (md.mMaxHP > INT_MAX / ((md.mLevel + HELL_LEVEL_BONUS) * (MAX_PLRS / 2 + 1))) // required by InitMonsterStats
			app_fatal("Too high mMaxHP %d for %s (%d)", md.mMaxHP, md.mName, i);
		if (md.mExp > UINT_MAX / ((md.mLevel + HELL_LEVEL_BONUS) * (MAX_PLRS / 2 + 1))) // required by InitMonsterStats
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
		//if (md.moAnimFrames[MA_STAND] > 0x7FFF) // required by InitMonster
		//	app_fatal("Too many(%d) stand-frames for %s (%d).", md.moAnimFrames[MA_STAND], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_STAND] >= 0x7FFF) // required by InitMonster
			app_fatal("Too long(%d) standing animation for %s (%d).", md.moAnimFrameLen[MA_STAND], md.moGfxFile, i);
		//if (md.moAnimFrames[MA_WALK] > lengthof(MWVel)) // required by MonWalkDir
		//	app_fatal("Too many(%d) walk-frames for %s (%d).", md.moAnimFrames[MA_WALK], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_WALK] != 1) // required by MonWalkDir
			app_fatal("Invalid framelen for the the walking animation for %s (%d).", md.moAnimFrameLen[MA_WALK], md.moGfxFile, i);
		//if (md.moAnimFrameLen[MA_WALK] * md.moAnimFrames[MA_WALK] >= SQUELCH_LOW)
		//	app_fatal("Too long(%d) walking animation for %s (%d) to finish before relax.", md.moAnimFrameLen[MA_WALK] * md.moAnimFrames[MA_WALK], md.moGfxFile, i);
		//if (md.moAnimFrameLen[MA_ATTACK] * md.moAnimFrames[MA_ATTACK] >= SQUELCH_LOW)
		//	app_fatal("Too long(%d) attack animation for %s (%d) to finish before relax.", md.moAnimFrameLen[MA_ATTACK] * md.moAnimFrames[MA_ATTACK], md.moGfxFile, i);
		//if (md.moAnimFrameLen[MA_SPECIAL] * md.moAnimFrames[MA_SPECIAL] >= SQUELCH_LOW)
		//	app_fatal("Too long(%d) special animation for %s (%d) to finish before relax.", md.moAnimFrameLen[MA_SPECIAL] * md.moAnimFrames[MA_SPECIAL], md.moGfxFile, i);
		if (md.moAnimFrameLen[MA_SPECIAL] == 0 && md.moAFNum2 != 0)
			app_fatal("moAFNum2 is set for %s (%d), but it has no special animation.", md.moGfxFile, i);
		// if ((md.moAnimFrames[MA_SPECIAL] == 0) != (md.moAnimFrameLen[MA_SPECIAL] == 0))
		//	app_fatal("Inconsistent moAnimFrames/moAnimFrameLen settings for the special animation %s (%d).", md.moGfxFile, i);
		// check if the special animation is used
		bool spUsed = false;
		for (int n = 0; n < NUM_MTYPES; n++) {
			const MonsterData& msd = monsterdata[n];
			if (msd.moFileNum != i) continue;
			if (IsSkel(n) || n == MT_GOLEM || MonsterAiSpecial(msd.mAI, i)) {
				if (md.moAnimFrameLen[MA_SPECIAL] == 0)
					app_fatal("Missing special animation for monster %s (%d)", msd.mName, n); // required by MAI_*, SpawnSkeleton, ActivateSpawn, SpawnGolem, SyncRhinoAnim
				spUsed = true;
			}
		}
		for (int n = 0; uniqMonData[n].mtype != MT_INVALID; n++) {
			const UniqMonData& um = uniqMonData[n];
			if (monsterdata[um.mtype].moFileNum != i) continue;
			if (MonsterAiSpecial(um.mAI, i)) {
				if (md.moAnimFrameLen[MA_SPECIAL] == 0)
					app_fatal("Missing special animation for unique monster %s (%d)", um.mName, n); // required by MAI_*, SpawnSkeleton, ActivateSpawn, SpawnGolem, SyncRhinoAnim
				spUsed = true;
			}
		}
		if (md.moAnimFrameLen[MA_SPECIAL] != 0 && !spUsed) {
			app_fatal("Unused special animation for %s (%d)", md.moGfxFile, i);
		}
	}
#endif
	// umt checks for GetLevelMTypes
#ifdef HELLFIRE
	assert(uniqMonData[UMT_HORKDMN].mtype == MT_HORKDMN);
	assert(uniqMonData[UMT_DEFILER].mtype == MT_DEFILER);
	assert(uniqMonData[UMT_NAKRUL].mtype == MT_NAKRUL);
#endif
	assert(uniqMonData[UMT_BUTCHER].mtype == MT_CLEAVER);
	assert(uniqMonData[UMT_GARBUD].mtype == MT_NGOATMC);
	assert(uniqMonData[UMT_ZHAR].mtype == MT_NMAGE);
	assert(uniqMonData[UMT_SNOTSPIL].mtype == MT_BFALLSP);
	assert(uniqMonData[UMT_LACHDAN].mtype == MT_GBLACK);
	assert(uniqMonData[UMT_WARLORD].mtype == MT_BBLACK);
	// umt checks for PlaceQuestMonsters
	assert(uniqMonData[UMT_LAZARUS].mtype == MT_BMAGE);
	assert(uniqMonData[UMT_BLACKJADE].mtype == MT_RSUCC);
	assert(uniqMonData[UMT_RED_VEX].mtype == MT_RSUCC);
#ifdef HELLFIRE
	{  // umt checks for WakeNakrul
		constexpr int targetRes = MORS_SLASH_PROTECTED | MORS_BLUNT_PROTECTED | MORS_PUNCTURE_PROTECTED | MORS_MAGIC_RESIST | MORS_FIRE_RESIST | MORS_LIGHTNING_RESIST | MORS_ACID_RESIST;
		assert(uniqMonData[UMT_NAKRUL].mMagicRes2 != targetRes);
		assert(uniqMonData[UMT_NAKRUL].mMagicRes != targetRes);
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_SLASH_IMMUNE) >= (targetRes & MORS_SLASH_IMMUNE));
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_BLUNT_IMMUNE) >= (targetRes & MORS_BLUNT_IMMUNE));
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_PUNCTURE_IMMUNE) >= (targetRes & MORS_PUNCTURE_IMMUNE));
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_FIRE_IMMUNE) >= (targetRes & MORS_FIRE_IMMUNE));
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_LIGHTNING_IMMUNE) >= (targetRes & MORS_LIGHTNING_IMMUNE));
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_MAGIC_IMMUNE) >= (targetRes & MORS_MAGIC_IMMUNE));
		assert((uniqMonData[UMT_NAKRUL].mMagicRes & MORS_ACID_IMMUNE) >= (targetRes & MORS_ACID_IMMUNE));
	}
#endif
#ifdef DEBUG_DATA
	bool sklwingBoned = false;
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
		 && (lvl != DLV_HELL4 || (um.mtype != MT_BMAGE && um.mtype != MT_GBLACK))
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
		if (um.mAI.aiType == AI_GARG && !(monsterdata[um.mtype].mFlags & MFLAG_NOSTONE))
			app_fatal("Unique AI_GARG might override stoned state (%s, %d)", um.mName, i); // required by MAI_Garg
		if (um.mAI.aiInt > UINT8_MAX - HELL_LEVEL_BONUS / 16) // required by InitUniqueMonster
			app_fatal("Too high aiInt %d for %s (%d).", um.muLevel, um.mName, i);
		if (um.muLevel == 0) // required by InitUniqueMonster
			app_fatal("Invalid muLevel %d for %s (%d).", um.muLevel, um.mName, i);
		if (um.muLevel > UINT8_MAX - HELL_LEVEL_BONUS) // required by InitUniqueMonster
			app_fatal("Too high muLevel %d for %s (%d).", um.muLevel, um.mName, i);
		if (um.muLevel + HELL_LEVEL_BONUS > CF_LEVEL && (monsterdata[um.mtype].mFlags & MFLAG_NODROP) == 0)
			app_fatal("Invalid muLevel %d for %s (%d). Too high in hell to set the level of item-drop.", um.muLevel, um.mName, i);
		if ((um.mUnqFlags & UMF_LEADER) != 0 && ((um.mUnqFlags & UMF_GROUP) == 0))
			app_fatal("Unique monster %s (%d) is a leader without group.", um.mName, i);
		if ((um.mUnqFlags & UMF_LIGHT) != 0 && i != UMT_LACHDAN)
			app_fatal("Unique monster %s (%d) has light, but its movement is not supported.", um.mName, i); // required by DeltaLoadLevel, LevelDeltaLoad, LoadLevel, SaveLevel, SetMapMonsters, MonChangeMap, MonStartWalk2, MonPlace, MonDoWalk, MonDoFadein, MonDoFadeout, MI_Rhino
		if (um.mUnqHit + monsterdata[um.mtype].mHit > INT_MAX /*- HELL_TO_HIT_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitUniqueMonster
			app_fatal("Too high mUnqHit %d for %s (%d).", um.mUnqHit, um.mName, i);
		if (um.mUnqHit2 + monsterdata[um.mtype].mHit2 > INT_MAX /*- HELL_TO_HIT_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitUniqueMonster
			app_fatal("Too high mUnqHit2 %d for %s (%d).", um.mUnqHit2, um.mName, i);
		if (um.mUnqMag + monsterdata[um.mtype].mMagic > INT_MAX /*- HELL_MAGIC_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitUniqueMonster
			app_fatal("Too high mUnqMag %d for %s (%d).", um.mUnqMag, um.mName, i);
		if (um.mMaxDamage == 0 && monsterdata[um.mtype].mMaxDamage != 0)
			if (um.mAI.aiType != AI_LACHDAN) {
				app_fatal("mMaxDamage is not set for unique monster %s (%d).", um.mName, i);
			} else {
				DoLog("mMaxDamage is not set for unique monster %s (%d).", um.mName, i);
			}
		if (um.mMaxDamage2 == 0 && ((um.mAI.aiType == AI_ROUND && um.mAI.aiParam1) || um.mAI.aiType == AI_FAT || um.mAI.aiType == AI_RHINO || um.mAI.aiType == AI_SNAKE))
			app_fatal("mMaxDamage2 is not set for unique monster %s (%d).", um.mName, i);
		if (um.mMaxDamage2 != 0 && (um.mAI.aiType == AI_SCAV || um.mAI.aiType == AI_GARG))
			app_fatal("Fake special attack of the unique monster %s (%d) might hurt someone because mMaxDamage2 is set.", um.mName, i);
		if (um.mMinDamage > um.mMaxDamage)
			app_fatal("Too high mMinDamage %d for unique monster %s (%d).", um.mMinDamage, um.mName, i);
		if (um.mMinDamage2 > um.mMaxDamage2)
			app_fatal("Too high mMinDamage2 %d for unique monster %s (%d).", um.mMinDamage2, um.mName, i);
		if (um.mMaxDamage > INT_MAX / (um.muLevel + HELL_LEVEL_BONUS)) // required by InitUniqueMonster
			app_fatal("Too high mMaxDamage %d for unique monster %s (%d).", um.mMaxDamage, um.mName, i);
		if (um.mMaxDamage2 > INT_MAX / (um.muLevel + HELL_LEVEL_BONUS)) // required by InitUniqueMonster
			app_fatal("Too high mMaxDamage2 %d for unique monster %s (%d).", um.mMaxDamage2, um.mName, i);
		if (um.mUnqAC + monsterdata[um.mtype].mArmorClass > INT_MAX /*- HELL_AC_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitUniqueMonster
			app_fatal("Too high mUnqAC %d for %s (%d).", um.mUnqAC, um.mName, i);
		if (um.mUnqEva + monsterdata[um.mtype].mEvasion > INT_MAX /*- HELL_EVASION_BONUS */- HELL_LEVEL_BONUS * 5 / 2) // required by InitUniqueMonster
			app_fatal("Too high mUnqEva %d for %s (%d).", um.mUnqEva, um.mName, i);
		if (um.mmaxhp > INT_MAX / ((um.muLevel + HELL_LEVEL_BONUS) * (MAX_PLRS / 2 + 1))) // required by InitUniqueMonster
			app_fatal("Too high mmaxhp %d for %s (%d)", um.mmaxhp, um.mName, i);
		if (monsterdata[um.mtype].mExp > UINT_MAX / (um.muLevel + HELL_LEVEL_BONUS)) // required by InitUniqueMonster
			app_fatal("Too high mExp %d for unique monster %s (%d)", monsterdata[um.mtype].mExp, um.mName, i);
		int umm = MonsterAiMissile(um.mAI);
		if (umm >= 0 && missiledata[umm].mFileNum >= NUM_FIXMFILE) {
			int bmm = MonsterAiMissile(monsterdata[um.mtype].mAI);
			if (umm != bmm) {
#ifdef HELLFIRE
				if (um.mtype == MT_SKLWING && umm == MIS_BONEDEMON)
					sklwingBoned = true;
				else
#endif
					app_fatal("Missile %d animation for unique monster %s (%d) might not be loaded.", umm, um.mName, i); // required by InitMonsterGFX
			}
#ifdef HELLFIRE
		} else if (um.mAI.aiType == AI_HORKDMN) {
			if (um.mtype != MT_HORKDMN)
				app_fatal("Missile %d animation for unique monster %s (%d) might not be loaded.", MFILE_SPAWNS, um.mName, i); // required by InitMonsterGFX
#endif
		}
		uint16_t res = monsterdata[um.mtype].mMagicRes;
		uint16_t resU = um.mMagicRes;
		for (int j = 0; j < 8; j++, res >>= 2, resU >>= 2) {
			if ((res & 3) > (resU & 3)) {
				DoLog("Warn: Weak muMagicRes %d (%d) for %s (%d): worse than mMagicRes %d.", um.mMagicRes, j, um.mName, i, monsterdata[um.mtype].mMagicRes);
			}
		}
		if (um.mmaxhp < monsterdata[um.mtype].mMaxHP)
			DoLog("Warn: Low mmaxhp %d for %s (%d): lower than mMaxHP %d.", um.mmaxhp, um.mName, i, monsterdata[um.mtype].mMaxHP);
	}
#ifdef HELLFIRE
	if (!sklwingBoned || missiledata[MIS_BONEDEMON].mFileNum < NUM_FIXMFILE)
		app_fatal("Loading optional missile for MT_SKLWING monsters is no longer necessary."); // required by InitMonsterGFX
#endif
#endif
	// items
	if (AllItemList[IDI_HEAL].iMiscId != IMISC_HEAL)
		app_fatal("IDI_HEAL is not a heal potion, its miscId is %d, iminlvl %d.", AllItemList[IDI_HEAL].iMiscId, AllItemList[IDI_HEAL].iMinMLvl);
	if (AllItemList[IDI_FULLHEAL].iMiscId != IMISC_FULLHEAL)
		app_fatal("IDI_FULLHEAL is not a heal potion, its miscId is %d, iminlvl %d.", AllItemList[IDI_FULLHEAL].iMiscId, AllItemList[IDI_FULLHEAL].iMinMLvl);
	if (AllItemList[IDI_MANA].iMiscId != IMISC_MANA)
		app_fatal("IDI_MANA is not a mana potion, its miscId is %d, iminlvl %d.", AllItemList[IDI_MANA].iMiscId, AllItemList[IDI_MANA].iMinMLvl);
	if (AllItemList[IDI_FULLMANA].iMiscId != IMISC_FULLMANA)
		app_fatal("IDI_FULLMANA is not a mana potion, its miscId is %d, iminlvl %d.", AllItemList[IDI_FULLMANA].iMiscId, AllItemList[IDI_FULLMANA].iMinMLvl);
	if (AllItemList[IDI_REJUV].iMiscId != IMISC_REJUV)
		app_fatal("IDI_REJUV is not a rejuv potion, its miscId is %d, iminlvl %d.", AllItemList[IDI_REJUV].iMiscId, AllItemList[IDI_REJUV].iMinMLvl);
	if (AllItemList[IDI_FULLREJUV].iMiscId != IMISC_FULLREJUV)
		app_fatal("IDI_FULLREJUV is not a rejuv potion, its miscId is %d.", AllItemList[IDI_FULLREJUV].iMiscId, AllItemList[IDI_FULLREJUV].iMinMLvl);
	if (AllItemList[IDI_BOOK1].iMiscId != IMISC_BOOK)
		app_fatal("IDI_BOOK1 is not a book, its miscId is %d, iminlvl %d.", AllItemList[IDI_BOOK1].iMiscId, AllItemList[IDI_BOOK1].iMinMLvl);
	if (AllItemList[IDI_BOOK4].iMiscId != IMISC_BOOK)
		app_fatal("IDI_BOOK4 is not a book, its miscId is %d, iminlvl %d.", AllItemList[IDI_BOOK4].iMiscId, AllItemList[IDI_BOOK4].iMinMLvl);
	if (AllItemList[IDI_CAMPAIGNMAP].iMiscId != IMISC_MAP)
		app_fatal("IDI_CAMPAIGNMAP is not a map, its miscId is %d, iminlvl %d.", AllItemList[IDI_CAMPAIGNMAP].iMiscId, AllItemList[IDI_CAMPAIGNMAP].iMinMLvl);
	if (AllItemList[IDI_CAMPAIGNMAP].iDurability != 1) // required because of affixes and map level
		app_fatal("IDI_CAMPAIGNMAP stack-size is not one, its miscId is %d, iminlvl %d.", AllItemList[IDI_CAMPAIGNMAP].iMiscId, AllItemList[IDI_CAMPAIGNMAP].iMinMLvl);
	if (AllItemList[IDI_CAMPAIGNMAP].iValue != (1 << (MAXCAMPAIGNSIZE - 6)) - 1) // required by InitCampaignMap
		app_fatal("IDI_CAMPAIGNMAP base value is invalid (%d vs. %d).", AllItemList[IDI_CAMPAIGNMAP].iValue, (1 << (MAXCAMPAIGNSIZE - 2)) - 1);
	static_assert(IDI_BOOK4 - IDI_BOOK1 == 3, "Invalid IDI_BOOK indices.");
	if (AllItemList[IDI_CLUB].iCurs != ICURS_CLUB)
		app_fatal("IDI_CLUB is not a club, its cursor is %d, iminlvl %d.", AllItemList[IDI_CLUB].iCurs, AllItemList[IDI_CLUB].iMinMLvl);
	if (AllItemList[IDI_DROPSHSTAFF].iUniqType != UITYPE_SHORTSTAFF)
		app_fatal("IDI_DROPSHSTAFF is not a short staff, its utype is %d, iminlvl %d.", AllItemList[UITYPE_SHORTSTAFF].iUniqType, AllItemList[UITYPE_SHORTSTAFF].iMinMLvl);
#ifdef DEBUG_DATA
	int minAmu, minLightArmor, minMediumArmor, minHeavyArmor; //, maxStaff = 0;
	minAmu = minLightArmor = minMediumArmor = minHeavyArmor = MAXCHARLEVEL;
	int rnddrops[ILVLMAX + 1][NUM_IARS][10];
	rnddrops[0][0][0] = 0;
	for (i = 0; i < NUM_IDI; i++) {
		const ItemData& ids = AllItemList[i];
		if (ids.iName == NULL) {
			if (i >= IDI_RNDDROP_FIRST || ids.iRnd != 0)
				app_fatal("Invalid iRnd value for nameless item (%d)", i);
			continue;
		}
		if (strlen(ids.iName) > 32 - 1)
			app_fatal("Too long name for %s (%d)", ids.iName, i); // required by SetItemData
		rnddrops[0][0][0] += ids.iRnd;
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
		if (ids.iMinMLvl == 0 && ids.itype != ITYPE_MISC && ids.itype != ITYPE_GOLD
		 && (i >= IDI_RNDDROP_FIRST || ids.iUniqType == UITYPE_NONE || ids.itype == ITYPE_STAFF /* required by DoWhittle */)) // required by DoClean
			app_fatal("iMinMLvl field is not set for %s (%d).", ids.iName, i);
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
			case IMISC_MAP:
				break;
			default:
				app_fatal("Usable item %s (%d) with miscId %d is not handled by SyncUseItem.", ids.iName, i, ids.iMiscId);
			}
			if (ids.iDurability == 0)
				app_fatal("Usable item %s (%d) with miscId %d must have a non-zero durablity(stacksize).", ids.iName, i, ids.iMiscId);
		} else {
			switch (ids.iMiscId) {
			case IMISC_HEAL:
			case IMISC_FULLHEAL:
			case IMISC_MANA:
			case IMISC_FULLMANA:
			case IMISC_REJUV:
			case IMISC_FULLREJUV:
			case IMISC_SCROLL:
				app_fatal("Non-Usable item %s (%d) with miscId %d is not handled by UseBeltItem.", ids.iName, i, ids.iMiscId);
				break;
			}
		}
		if (ids.iClass == ICLASS_QUEST && ids.iLoc != ILOC_UNEQUIPABLE)
			app_fatal("Quest item %s (%d) must be unequippable, not %d", ids.iName, i, ids.iLoc);
		if (ids.iClass == ICLASS_QUEST && ids.itype != ITYPE_MISC)
			app_fatal("Quest item %s (%d) must be have 'misc' itype, otherwise it might be sold at vendors.", ids.iName, i);
	}
#if 0
	LogErrorF("Max drop %d vs %d", rnddrops[0][0][0], ITEM_RNDDROP_MAX);
#endif
#if UNOPTIMIZED_RNDITEMS
	if (rnddrops[0][0][0] > ITEM_RNDDROP_MAX)
		app_fatal("Too many drop options: %d. Maximum is %d", rnddrops[0][0][0], ITEM_RNDDROP_MAX);
#else
	if (rnddrops[0][0][0] > 0x7FFF)
		app_fatal("Too many drop options: %d. Maximum is %d", rnddrops[0][0][0], 0x7FFF);
#endif
	if (minLightArmor > 1)
		app_fatal("No light armor for OperateArmorStand. Current minimum is level %d", minLightArmor);
	if (minMediumArmor > 10)
		app_fatal("No medium armor for OperateArmorStand. Current minimum is level %d", minMediumArmor);
	if (minHeavyArmor > 24)
		app_fatal("No heavy armor for OperateArmorStand. Current minimum is level %d", minHeavyArmor);
#ifdef HELLFIRE
	if (uniqMonData[UMT_HORKDMN].muLevel < minAmu)
		app_fatal("No amulet for THEODORE. Current minimum is level %d, while the monster level is %d.", minAmu, uniqMonData[UMT_HORKDMN].muLevel);
#endif
	memset(rnddrops, 0, sizeof(rnddrops));
	i = 0;
	for (const AffixData* pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++, i++) {
		const BYTE pow = pres->PLPower;
		for (int ii = 0; ii < lengthof(pres->PLRanges); ii++) {
			for (int n = pres->PLRanges[ii].from; n <= pres->PLRanges[ii].to; n++) {
				int cnt = pres->PLDouble ? 2 : 1;
				if (pres->PLIType & PLT_MISC)
					rnddrops[n][ii][0] += cnt;
				if (pres->PLIType & PLT_BOW)
					rnddrops[n][ii][1] += cnt;
				if (pres->PLIType & (PLT_STAFF | PLT_CHRG))
					rnddrops[n][ii][2] += cnt;
				if (pres->PLIType & PLT_MELEE)
					rnddrops[n][ii][3] += cnt;
				if (pres->PLIType & PLT_SHLD)
					rnddrops[n][ii][4] += cnt;
				if (pres->PLIType & PLT_HELM)
					rnddrops[n][ii][5] += cnt;
				if (pres->PLIType & PLT_LARMOR)
					rnddrops[n][ii][6] += cnt;
				if (pres->PLIType & PLT_MARMOR)
					rnddrops[n][ii][7] += cnt;
				if (pres->PLIType & PLT_HARMOR)
					rnddrops[n][ii][8] += cnt;
				if (pres->PLIType & PLT_MAP)
					rnddrops[n][ii][9] += cnt;
			}
		}
		if (pres->PLParam2 < pres->PLParam1)
			app_fatal("Invalid PLParam-range set for %d. prefix (power:%d, pparam:%d-%d)", i, pow, pres->PLParam1, pres->PLParam2);
		if (pres->PLParam2 - pres->PLParam1 >= 0x7FFF) // required by SaveItemPower
			app_fatal("PLParam-range too high for %d. prefix (power:%d, pparam:%d-%d)", i, pow, pres->PLParam1, pres->PLParam2);
		if (pres->PLParam1 == 0 && pres->PLParam2 == 0
		 && pow != IPL_INDESTRUCTIBLE && pow != IPL_NOMANA && pow != IPL_KNOCKBACK && pow != IPL_STUN && pow != IPL_NO_BLEED && pow != IPL_BLEED && pow != IPL_PENETRATE_PHYS
		 && pow != IPL_SETDAM && pow != IPL_ONEHAND && pow != IPL_ALLRESZERO && pow != IPL_DRAINLIFE && pow != IPL_SETAC && pow != IPL_MANATOLIFE && pow != IPL_LIFETOMANA)
			app_fatal("Invalid(zero) PLParams set for %d. prefix (power:%d)", i, pow);

		if (pres->PLMinVal != pres->PLMaxVal) {
			if (pres->PLMaxVal < pres->PLMinVal)
				app_fatal("Invalid PL*Val-range set for %d. prefix (power:%d, pparam:%d-%d)", i, pow, pres->PLParam1, pres->PLParam2); // required by PLVal
			if (pres->PLParam2 == pres->PLParam1)
				app_fatal("Unused PL*Val-range set for %d. prefix (power:%d, pparam:%d-%d)", i, pow, pres->PLParam1, pres->PLParam2);
			if ((pres->PLMaxVal - pres->PLMinVal) >= INT_MAX / (pres->PLParam2 - pres->PLParam1))
				app_fatal("Too hight PL*Vals set for %d. prefix (power:%d, pparam:%d-%d)", i, pow, pres->PLParam1, pres->PLParam2); // required by PLVal
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
			if (pres->PLParam2 >= 3 && (pres->PLIType & PLT_BOW)) {
				app_fatal("Too high PLParam2 set for %d. prefix (power:%d, pparam2:%d)", i, pres->PLPower, pres->PLParam2); // required by MissMonHitByPlr and MissPlrHitByPlr
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
			if (pres->PLParam1 <= -100 || pres->PLParam2 > 200) {
				app_fatal("PLParam too high for %d. prefix (power:%d, pparam2:%d)", i, pres->PLPower, pres->PLParam2);
			}
		}
		if (pres->PLPower == IPL_CHARGES) {
			for (int n = 0; n < NUM_SPELLS; n++) {
				const SpellData& sd = spelldata[n];
				if (sd.sStaffLvl != SPELL_NA && sd.sStaffMax * pres->PLParam2 > UCHAR_MAX) { // required by (Un)PackPkItem
					app_fatal("PLParam too high for %d. prefix (power:%d, pparam2:%d) to be used for staff with spell %d.", i, pres->PLPower, pres->PLParam2, n);
				}
			}
		}
		if (pres->PLIType == PLT_MAP) {
			if (pres->PLPower == IPL_ACP) {
				if (pres->PLParam1 < -2) { // required by InitCampaignMap
					app_fatal("(Map-)PLParam too low for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
				}
				if (pres->PLParam2 > UCHAR_MAX - 2) { // required by InitCampaignMap
					app_fatal("(Map-)PLParam too high for %d. prefix (power:%d, pparam2:%d)", i, pres->PLPower, pres->PLParam2);
				}
			}
			if (pres->PLPower == IPL_LIGHT) {
				if (pres->PLParam1 < -(MAXCAMPAIGNSIZE - (1 + 6))) { // required by InitCampaignMap and GetItemPower
					app_fatal("(Map-)PLParam too low for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
				}
				if (pres->PLParam2 > 6) { // required by InitCampaignMap and GetItemPower
					app_fatal("(Map-)PLParam too high for %d. prefix (power:%d, pparam2:%d)", i, pres->PLPower, pres->PLParam2);
				}
			}
			if (pres->PLMultVal != 1) {
				app_fatal("(Map-)PLMultVal invalid for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
			if (pres->PLMinVal != 0) {
				app_fatal("(Map-)PLMinVal invalid for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
			if (pres->PLMaxVal != 0) {
				app_fatal("(Map-)PLMaxVal invalid for %d. prefix (power:%d, pparam1:%d)", i, pres->PLPower, pres->PLParam1);
			}
		}
	}
	int maxAffix = -1;
	for (int ii = 0; ii < NUM_IARS; ii++) {
		const char* loc = ii == IAR_DROP ? "drop" : ii == IAR_SHOP ? "shop" : "craft";
		for (int n = 0; n <= ILVLMAX; n++) {
			for (int k = 0; k < 10; k++) {
				int drops = rnddrops[n][ii][k];
				if (drops > maxAffix) {
					maxAffix = drops;
				}
				if (drops > std::min(ITEM_RNDAFFIX_MAX, 0x7FFF))
					app_fatal("Too many prefix options: %d (lvl%d for %s type%d), . Maximum is %d", drops, n, loc, k, std::min(ITEM_RNDAFFIX_MAX, 0x7FFF));
			}
		}
	}
	memset(rnddrops, 0, sizeof(rnddrops));
	const AffixData* sufs = PL_Suffix;
	for (i = 0; sufs->PLPower != IPL_INVALID; sufs++, i++) {
		const BYTE pow = sufs->PLPower;
		for (int ii = 0; ii < lengthof(sufs->PLRanges); ii++) {
			for (int n = sufs->PLRanges[ii].from; n <= sufs->PLRanges[ii].to; n++) {
				int cnt = sufs->PLDouble ? 2 : 1;
				if (sufs->PLIType & PLT_MISC)
					rnddrops[n][ii][0] += cnt;
				if (sufs->PLIType & PLT_BOW)
					rnddrops[n][ii][1] += cnt;
				if (sufs->PLIType & (PLT_STAFF | PLT_CHRG))
					rnddrops[n][ii][2] += cnt;
				if (sufs->PLIType & PLT_MELEE)
					rnddrops[n][ii][3] += cnt;
				if (sufs->PLIType & PLT_SHLD)
					rnddrops[n][ii][4] += cnt;
				if (sufs->PLIType & PLT_HELM)
					rnddrops[n][ii][5] += cnt;
				if (sufs->PLIType & PLT_LARMOR)
					rnddrops[n][ii][6] += cnt;
				if (sufs->PLIType & PLT_MARMOR)
					rnddrops[n][ii][7] += cnt;
				if (sufs->PLIType & PLT_HARMOR)
					rnddrops[n][ii][8] += cnt;
				if (sufs->PLIType & PLT_MAP)
					rnddrops[n][ii][9] += cnt;
			}
		}
		if (sufs->PLDouble)
			app_fatal("Invalid PLDouble set for %d. suffix (power:%d, pparam1:%d)", i, pow, sufs->PLParam1);
		if (sufs->PLParam2 < sufs->PLParam1)
			app_fatal("Invalid PLParam-range set for %d. suffix (power:%d, pparam:%d-%d)", i, pow, sufs->PLParam1, sufs->PLParam2);
		if (sufs->PLParam2 - sufs->PLParam1 >= 0x7FFF) // required by SaveItemPower
			app_fatal("PLParam-range too high for %d. suffix (power:%d, pparam:%d-%d)", i, pow, sufs->PLParam1, sufs->PLParam2);
		if (sufs->PLParam1 == 0 && sufs->PLParam2 == 0
		 && pow != IPL_INDESTRUCTIBLE && pow != IPL_NOMANA && pow != IPL_KNOCKBACK && pow != IPL_STUN && pow != IPL_NO_BLEED && pow != IPL_BLEED && pow != IPL_PENETRATE_PHYS
		 && pow != IPL_SETDAM && pow != IPL_ONEHAND && pow != IPL_ALLRESZERO && pow != IPL_DRAINLIFE && pow != IPL_SETAC && pow != IPL_MANATOLIFE && pow != IPL_LIFETOMANA)
			app_fatal("Invalid(zero) PLParams set for %d. suffix (power:%d)", i, pow);

		if (sufs->PLMinVal != sufs->PLMaxVal) {
			if (sufs->PLMaxVal < sufs->PLMinVal)
				app_fatal("Invalid PL*Val-range set for %d. suffix (power:%d, pparam:%d-%d)", i, pow, sufs->PLParam1, sufs->PLParam2); // required by PLVal
			if (sufs->PLParam2 == sufs->PLParam1)
				app_fatal("Unused PL*Val-range set for %d. suffix (power:%d, pparam:%d-%d)", i, pow, sufs->PLParam1, sufs->PLParam2);
			if ((sufs->PLMaxVal - sufs->PLMinVal) >= INT_MAX / (sufs->PLParam2 - sufs->PLParam1))
				app_fatal("Too hight PL*Vals set for %d. suffix (power:%d, pparam:%d-%d)", i, pow, sufs->PLParam1, sufs->PLParam2); // required by PLVal
		}

		if (sufs->PLPower == IPL_FASTATTACK) {
			if (sufs->PLParam1 < 1 || sufs->PLParam2 > 4) {
				app_fatal("Invalid PLParam set for %d. suffix (power:%d, pparam1:%d)", i, sufs->PLPower, sufs->PLParam1);
			}
			if (sufs->PLParam2 >= 3 && (sufs->PLIType & PLT_BOW)) {
				app_fatal("Too high PLParam2 set for %d. suffix (power:%d, pparam2:%d)", i, sufs->PLPower, sufs->PLParam2); // required by MissMonHitByPlr and MissPlrHitByPlr
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
			if (sufs->PLParam1 <= -100 || sufs->PLParam2 > 200) {
				app_fatal("PLParam too high for %d. suffix (power:%d, pparam2:%d)", i, sufs->PLPower, sufs->PLParam2);
			}
		}
		for (const AffixData* pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {
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
			if ((pres->PLPower == IPL_TOHIT || pres->PLPower == IPL_TOHIT_DAMP) &&
				(sufs->PLPower == IPL_TOHIT || sufs->PLPower == IPL_TOHIT_DAMP)) {
				app_fatal("IPL_TOHIT/IPL_TOHIT_DAMP might be set for the same item.");
			}
			if ((pres->PLPower == IPL_DAMP || pres->PLPower == IPL_TOHIT_DAMP || pres->PLPower == IPL_CRYSTALLINE) &&
				(sufs->PLPower == IPL_DAMP || sufs->PLPower == IPL_TOHIT_DAMP || sufs->PLPower == IPL_CRYSTALLINE)) {
				app_fatal("IPL_DAMP/IPL_TOHIT_DAMP/IPL_CRYSTALLINE might be set for the same item.");
			}
			if ((pres->PLPower == IPL_SETAC || pres->PLPower == IPL_ACMOD) &&
				(sufs->PLPower == IPL_SETAC || sufs->PLPower == IPL_ACMOD)) {
				app_fatal("IPL_SETAC/IPL_ACMOD might be set for the same item.");
			}
		}
	}
	for (int ii = 0; ii < NUM_IARS; ii++) {
		const char* loc = ii == IAR_DROP ? "drop" : ii == IAR_SHOP ? "shop" : "craft";
		for (int n = 0; n <= ILVLMAX; n++) {
			for (int k = 0; k < 10; k++) {
				int dropts = rnddrops[n][ii][k];
				if (dropts > maxAffix) {
					maxAffix = dropts;
				}
				if (dropts > std::min(ITEM_RNDAFFIX_MAX, 0x7FFF))
					app_fatal("Too many suffix options: %d (lvl%d for %s type%d), . Maximum is %d", dropts, n, loc, k, std::min(ITEM_RNDAFFIX_MAX, 0x7FFF));
			}
		}
	}
#if 0
	LogErrorF("Max affix %d vs %d", maxAffix, ITEM_RNDAFFIX_MAX);
#endif
#if DEV_MODE
	for (i = 1; i < MAXCHARLEVEL; i++) {
		int a = 0, b = 0, c = 0, w = 0;
		for (const AffixData* pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {
			if (pres->PLRanges[IAR_SHOP].to >= i + 8 && pres->PLRanges[IAR_SHOP].from <= i + 8 && pres->PLOk) {
				c++;
			}
			if (pres->PLRanges[IAR_SHOP].to >= i && pres->PLRanges[IAR_SHOP].from <= i && pres->PLOk) {
				a++;
			}
			if (pres->PLRanges[IAR_DROP].to >= i && pres->PLRanges[IAR_DROP].from <= i) {
				b++;
				if (!pres->PLOk)
					w++;
			}
		}
		int as = 0, bs = 0, cs = 0, ws = 0;
		for (const AffixData* pres = PL_Suffix; pres->PLPower != IPL_INVALID; pres++) {
			if (pres->PLRanges[IAR_SHOP].to >= i + 8 && pres->PLRanges[IAR_SHOP].from <= i + 8 && pres->PLOk) {
				cs++;
			}
			if (pres->PLRanges[IAR_SHOP].to >= i && pres->PLRanges[IAR_SHOP].from <= i && pres->PLOk) {
				as++;
			}
			if (pres->PLRanges[IAR_DROP].to >= i && pres->PLRanges[IAR_DROP].from <= i) {
				bs++;
				if (!pres->PLOk)
					ws++;
			}
		}
		LogErrorF("Affix for lvl%2d: shop(%d:%d) loot(%d:%d/%d:%d) boy(%d:%d)", i, a, as, b, bs, w, ws, c, cs);
	}
#endif
	// unique items
	for (i = 0; i < NUM_UITEM; i++) {
		const UniqItemData& ui = UniqueItemList[i];
		for (int n = 1; n <= 6; n++) {
			const BYTE pow = GetUniqueItemPower(ui, n);
			const int paramA = GetUniqueItemParamA(ui, n);
			const int paramB = GetUniqueItemParamB(ui, n);
			if (pow != IPL_INVALID) {
				if (paramB < paramA)
					app_fatal("Invalid UIParam%d-range set for '%s' %d.", n, ui.UIName, i);
				if (paramB - paramA >= 0x7FFF) // required by SaveItemPower
					app_fatal("UIParam%d-range too high for '%s' %d.", n, ui.UIName, i);
				if (paramA == 0 && paramB == 0
				 && pow != IPL_INDESTRUCTIBLE && pow != IPL_NOMANA && pow != IPL_KNOCKBACK && pow != IPL_STUN && pow != IPL_NO_BLEED && pow != IPL_BLEED && pow != IPL_PENETRATE_PHYS
				 && pow != IPL_SETDAM && pow != IPL_ONEHAND && pow != IPL_ALLRESZERO && pow != IPL_DRAINLIFE && pow != IPL_SETAC && pow != IPL_MANATOLIFE && pow != IPL_LIFETOMANA)
					app_fatal("Invalid UIParam%d set for '%s' %d.", n, ui.UIName, i);

				for (int m = n + 1; m <= 6; m++) {
					if (GetUniqueItemPower(ui, m) == pow)
						app_fatal("SaveItemPower does not support the same affix multiple times on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else {
				if (n == 1)
					app_fatal("Unique item '%s' %d does not have any affix", ui.UIName, i);
				for (int m = n + 1; m <= 6; m++) {
					if (GetUniqueItemPower(ui, m) != IPL_INVALID)
						app_fatal("Unique item '%s' %d ignores its set affix%d, because UIPower%d is IPL_INVALID.", ui.UIName, i);
				}
			}
			if (pow == IPL_ATTRIBS) {
				for (int m = 1; m <= 6; m++) {
					const BYTE opow = GetUniqueItemPower(ui, m);
					if (opow == IPL_STR || opow == IPL_MAG || opow == IPL_DEX || opow == IPL_VIT)
						app_fatal("SaveItemPower does not support IPL_ATTRIBS and IPL_STR/IPL_MAG/IPL_DEX/IPL_VIT modifiers at the same time on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else if (pow == IPL_ALLRES) {
				for (int m = 1; m <= 6; m++) {
					const BYTE opow = GetUniqueItemPower(ui, m);
					if (opow == IPL_FIRERES || opow == IPL_LIGHTRES || opow == IPL_MAGICRES || opow == IPL_ACIDRES)
						app_fatal("SaveItemPower does not support IPL_ALLRES and IPL_FIRERES/IPL_LIGHTRES/IPL_MAGICRES/IPL_ACIDRES modifiers at the same time on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else if (pow == IPL_TOHIT) {
				for (int m = 1; m <= 6; m++) {
					const BYTE opow = GetUniqueItemPower(ui, m);
					if (opow == IPL_TOHIT_DAMP)
						app_fatal("SaveItemPower does not support IPL_TOHIT and IPL_TOHIT_DAMP modifiers at the same time on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else if (pow == IPL_SETDAM && GetUniqueItemParamA(ui, n) == 0 && GetUniqueItemParamB(ui, n) == 0) {
				for (int m = 1; m <= 6; m++) {
					const BYTE opow = GetUniqueItemPower(ui, m);
					if (opow == IPL_DAMMOD || opow == IPL_DAMP || opow == IPL_TOHIT_DAMP || opow == IPL_CRYSTALLINE)
						app_fatal("SaveItemPower does not support IPL_SETDAM (0) and IPL_DAMMOD/IPL_DAMP/IPL_TOHIT_DAMP/IPL_CRYSTALLINE modifiers at the same time on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else if (pow == IPL_DAMP || pow == IPL_TOHIT_DAMP || pow == IPL_CRYSTALLINE) {
				for (int m = 1; m <= 6; m++) {
					const BYTE opow = GetUniqueItemPower(ui, m);
					if (n != m && (opow == IPL_DAMP || opow == IPL_TOHIT_DAMP || opow == IPL_CRYSTALLINE))
						app_fatal("SaveItemPower does not support IPL_DAMP/IPL_TOHIT_DAMP/IPL_CRYSTALLINE modifiers at the same time on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else if (pow == IPL_SETAC) {
				for (int m = 1; m <= 6; m++) {
					const BYTE opow = GetUniqueItemPower(ui, m);
					if (opow == IPL_ACMOD)
						app_fatal("SaveItemPower does not support IPL_SETAC and IPL_ACMOD modifiers at the same time on '%s' %d, %dvs%d.", ui.UIName, i, n, m);
				}
			} else if (pow == IPL_FASTATTACK) {
				if (GetUniqueItemParamA(ui, n) < 1 || GetUniqueItemParamB(ui, n) > 4)
					app_fatal("Invalid UIParam%d set for '%s' %d.", n, ui.UIName, i);
				if (GetUniqueItemParamB(ui, n) >= 3) {
					for (int n = 0; n < NUM_IDI; n++) {
						if (AllItemList[n].iUniqType == ui.UIUniqType && AllItemList[n].itype == ITYPE_BOW) {
							app_fatal("Too high UIParam%d set for '%s' %d.", n, ui.UIName, i); // required by MissMonHitByPlr and MissPlrHitByPlr
						}
					}
				}
			} else if (pow == IPL_FASTRECOVER) {
				if (GetUniqueItemParamA(ui, n) < 1 || GetUniqueItemParamB(ui, n) > 3)
					app_fatal("Invalid UIParam%d set for '%s' %d.", n, ui.UIName, i);
			} else if (pow == IPL_FASTCAST) {
				if (GetUniqueItemParamA(ui, n) < 1 || GetUniqueItemParamB(ui, n) > 3)
					app_fatal("Invalid UIParam%d set for '%s' %d.", n, ui.UIName, i);
			} else if (pow == IPL_FASTWALK) {
				if (GetUniqueItemParamA(ui, n) < 1 || GetUniqueItemParamB(ui, n) > 3)
					app_fatal("Invalid UIParam%d set for '%s' %d.", n, ui.UIName, i);
			} else if (pow == IPL_DUR) {
				if (GetUniqueItemParamA(ui, n) <= -100 || GetUniqueItemParamB(ui, n) > 200)
					app_fatal("Invalid UIParam%d set for '%s' %d.", n, ui.UIName, i);
			} else if (pow == IPL_REQSTR) {
				for (int n = 0; n < NUM_IDI; n++) {
					if (AllItemList[n].iUniqType == ui.UIUniqType) {
						if (AllItemList[n].iMinStr < -paramA)
							app_fatal("Too low UIParam%d set for '%s' %d.", n, ui.UIName, i); // required by iMinStr
						if (UCHAR_MAX - AllItemList[n].iMinStr < paramB)
							app_fatal("Too high UIParam%d set for '%s' %d.", n, ui.UIName, i); // required by iMinStr
					}
				}
			} else if ((pow == IPL_STR || pow == IPL_MAG || pow == IPL_DEX) && GetUniqueItemParamA(ui, n) < 0 && HasUniqueItemReq(ui, pow)) {
				for (int m = 1; m <= 6; m++) {
					BYTE pw = GetUniqueItemPower(ui, m);
					if (pow != pw && (pw == IPL_STR || pw == IPL_MAG || pw == IPL_DEX) && GetUniqueItemParamB(ui, m) > 0) {
						// str/mag/dex +- modifiers
						for (int ii = i + 1; ii < NUM_UITEM; ii++) {
							const UniqItemData& oui = UniqueItemList[ii];
							if (!HasUniqueItemReq(oui, pw))
								continue;
							for (int o = 1; o <= 6; o++) {
								if (GetUniqueItemPower(oui, o) == pow && GetUniqueItemParamB(ui, o) > 0) {
									for (int p = 1; p <= 6; p++) {
										if (GetUniqueItemPower(oui, p) == pw && GetUniqueItemParamA(ui, p) < 0) {
											app_fatal("Unique items '%s' %d and '%s' %d might help each other to equip.", ui.UIName, i, oui.UIName, ii);
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}
		int n = 0;
		for ( ; n < NUM_IDI; n++) {
			if (AllItemList[n].iUniqType == ui.UIUniqType)
				break;
		}
		if (n == NUM_IDI)
			app_fatal("Missing base type for '%s' %d.", ui.UIName, i);
	}
#endif // DEBUG_DATA
	assert(itemfiledata[ItemCAnimTbl[ICURS_MAGIC_ROCK]].iAnimLen == 10); // required by ProcessItems
#ifdef DEBUG_DATA
	for (i = 0; i < NUM_IFILE; i++) {
		const ItemFileData& id = itemfiledata[i];
		if (id.idSFX != SFX_NONE) {
			if (id.idSFX >= NUM_SFXS)
				app_fatal("Invalid idSFX %d for %s (%d)", id.idSFX, id.ifName, i);
			if ((id.iAnimLen >> 1) < 2)
				app_fatal("Too short iAnimLen %d for %s (%d)", id.iAnimLen, id.ifName, i); // required by ProcessItems
		}
		if (id.iiSFX != SFX_NONE) {
			if (id.iiSFX >= NUM_SFXS)
				app_fatal("Invalid iiSFX %d for %s (%d)", id.iiSFX, id.ifName, i);
		}
	}
	// objects
	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		const ObjFileData& od = objfiledata[i];
		if (od.oAnimFlag != OAM_NONE) {
			if (od.oAnimFlag == OAM_SINGLE)
				app_fatal("Incorrect oAnimFlag %d for %s (%d)", od.oAnimFlag, od.ofName, i);
			if (od.oAnimFrameLen <= 0)
				app_fatal("Invalid oAnimFrameLen %d for %s (%d)", od.oAnimFrameLen, od.ofName, i);
			if (od.oAnimLen <= 1) // required by SetupObject
				app_fatal("Invalid oAnimLen %d for %s (%d)", od.oAnimLen, od.ofName, i);
			if (od.oAnimLen >= 0x7FFF) // required by SetupObject
				app_fatal("Too high oAnimLen %d for %s (%d)", od.oAnimLen, od.ofName, i);
		}
		if (od.oSFXCnt != 0) {
			for (int n = 0; n < NUM_OBJECTS; n++) {
				const ObjectData& obd = objectdata[n];
				if (obd.ofindex != i) continue;
				if (n == OBJ_SHRINEL || n == OBJ_SHRINER || n == OBJ_GOATSHRINE || n == OBJ_CAULDRON) continue;
				if (od.oSFXCnt == 1 &&
					(n == OBJ_BARREL || n == OBJ_BARRELEX
#ifdef HELLFIRE
				 || n == OBJ_URN || n == OBJ_URNEX || n == OBJ_POD || n == OBJ_PODEX
#endif
					)) continue;

				if (od.oSFXCnt == 2 &&
					(n == OBJ_L1LDOOR || n == OBJ_L1RDOOR || n == OBJ_L2LDOOR || n == OBJ_L2RDOOR || n == OBJ_L3LDOOR || n == OBJ_L3RDOOR
#ifdef HELLFIRE
				 || n == OBJ_L5LDOOR || n == OBJ_L5RDOOR
#endif
					)) continue;
				if (obd.ofindex == i && n != OBJ_SHRINEL && n != OBJ_SHRINER && n != OBJ_GOATSHRINE && n != OBJ_CAULDRON)
					app_fatal("Unsupported oSFXCnt for %s (%d) used by object %d", od.ofName, i, n);
			}
		}
	}
	for (i = 0; i < NUM_OBJECTS; i++) {
		const ObjectData& od = objectdata[i];
		if (od.oModeFlags & OMF_RESERVED) {
			app_fatal("Invalid oModeFlags for %d.", i);
		}
		if (((od.oModeFlags & OMF_ACTIVE) != 0) != (od.oSelFlag != 0)) {
			app_fatal("Inconsistent oModeFlags and oSelFlag for %d.", i);
		}
		if (od.oLightRadius > MAX_LIGHT_RAD) {
			app_fatal("Light radius is too high for %d. object.", i);
		}
	}
#endif // DEBUG_DATA
	assert(objectdata[OBJ_L1RDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
	assert(objectdata[OBJ_L2LDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
	assert(objectdata[OBJ_L2RDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
	assert(objectdata[OBJ_L3LDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
	assert(objectdata[OBJ_L3RDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
#ifdef HELLFIRE
	assert(objectdata[OBJ_L5LDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
	assert(objectdata[OBJ_L5RDOOR].oSelFlag == objectdata[OBJ_L1LDOOR].oSelFlag); //  required by OpenDoor, CloseDoor
#endif

	// spells
	assert(SFX_VALID(spelldata[SPL_DISARM].sSFX)); // required by On_DISARMXY
	assert(SFX_VALID(spelldata[SPL_TELEKINESIS].sSFX)); // required by DoTelekinesis
	assert(spelldata[SPL_RESURRECT].sManaCost == 0); // required by GetItemSpell
	assert(!(spelldata[SPL_HEAL].sUseFlags & SFLAG_DUNGEON)); // required by UseBeltItem
	assert(spelldata[SPL_TELEPORT].sSkillFlags & SDFLAG_TARGETED); // required by AddTeleport
#define OBJ_TARGETING_CURSOR(x) ((x) == CURSOR_NONE || (x) == CURSOR_DISARM)
	assert(OBJ_TARGETING_CURSOR(spelldata[SPL_DISARM].scCurs)); // required by TryIconCurs
	assert(OBJ_TARGETING_CURSOR(spelldata[SPL_DISARM].spCurs)); // required by TryIconCurs
#define PLR_TARGETING_CURSOR(x) ((x) == CURSOR_NONE || (x) == CURSOR_HEALOTHER || (x) == CURSOR_RESURRECT)
	assert(PLR_TARGETING_CURSOR(spelldata[SPL_HEALOTHER].scCurs)); // required by TryIconCurs
	assert(PLR_TARGETING_CURSOR(spelldata[SPL_HEALOTHER].spCurs)); // required by TryIconCurs
	assert(PLR_TARGETING_CURSOR(spelldata[SPL_RESURRECT].scCurs)); // required by TryIconCurs
	assert(PLR_TARGETING_CURSOR(spelldata[SPL_RESURRECT].spCurs)); // required by TryIconCurs
#define ITEM_TARGETING_CURSOR(x) ((x) == CURSOR_NONE || (x) == CURSOR_IDENTIFY || (x) == CURSOR_REPAIR || (x) == CURSOR_RECHARGE || (x) == CURSOR_OIL)
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_IDENTIFY].scCurs)); // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_IDENTIFY].spCurs)); // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_OIL].scCurs));      // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_OIL].spCurs));      // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_REPAIR].scCurs));   // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_REPAIR].spCurs));   // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_RECHARGE].scCurs)); // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_RECHARGE].spCurs)); // required by TryIconCurs and CheckCursMove
#ifdef HELLFIRE
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_BUCKLE].scCurs));  // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_BUCKLE].spCurs));  // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_WHITTLE].scCurs)); // required by TryIconCurs and CheckCursMove
	assert(ITEM_TARGETING_CURSOR(spelldata[SPL_WHITTLE].spCurs)); // required by TryIconCurs and CheckCursMove
#endif
#define SPEC_TARGETING_CURSOR(x) ((x) == CURSOR_NONE || (x) == CURSOR_TELEKINESIS)
	assert(SPEC_TARGETING_CURSOR(spelldata[SPL_TELEKINESIS].scCurs)); // required by TryIconCurs
	assert(SPEC_TARGETING_CURSOR(spelldata[SPL_TELEKINESIS].spCurs)); // required by TryIconCurs
#ifdef DEBUG_DATA
	bool hasBookSpell = false, hasStaffSpell = false, hasScrollSpell = false, hasRuneSpell = false;
	int bookSpells = 0, staffSpells = 0, scrollSpells = 0, runeSpells = 0;
	for (i = 0; i < NUM_SPELLS; i++) {
		const SpellData& sd = spelldata[i];
		SkillDetails skd;
		if (sd.sNameText != NULL) {
			int w = GetSmallStringWidth(sd.sNameText);
			if (w > (SKILLDETAILS_PNL_WIDTH - 2 * BOXBORDER_WIDTH))
				app_fatal("Name of %s (%d) is too wide.", sd.sNameText, i); // required by DrawSkillDetails
			if (w > (SKILLBOOK_PNL_WIDTH - (2 * SBOOK_CELWIDTH + SBOOK_X_OFFSET + 2 * BOXBORDER_WIDTH)))
				app_fatal("Name of %s (%d) is too wide.", sd.sNameText, i); // required by DrawSpellBook
		}

		GetSkillDetails(i, 0, &skd);
		if (i == SPL_DISARM
		 || i == SPL_HEALOTHER || i == SPL_RESURRECT
		 || i == SPL_IDENTIFY || i == SPL_OIL || i == SPL_REPAIR || i == SPL_RECHARGE
#ifdef HELLFIRE
			|| i == SPL_BUCKLE || i == SPL_WHITTLE
#endif
		 || i == SPL_TELEKINESIS
		) {
			; // should have been tested above -> skip
		} else {
			if (sd.scCurs != CURSOR_NONE && sd.scCurs != CURSOR_TELEPORT)
				app_fatal("Invalid scCurs %d for %s (%d)", sd.scCurs, sd.sNameText, i); // required by TryIconCurs
			if (sd.spCurs != CURSOR_NONE && sd.spCurs != CURSOR_TELEPORT)
				app_fatal("Invalid spCurs %d for %s (%d)", sd.spCurs, sd.sNameText, i); // required by TryIconCurs
		}
		if (SPELL_RUNE(i)) {
			if (sd.sBookLvl != SPELL_NA)
				app_fatal("Invalid sBookLvl %d for %s (%d)", sd.sBookLvl, sd.sNameText, i);
			if (sd.sStaffLvl != SPELL_NA)
				app_fatal("Invalid sStaffLvl %d for %s (%d)", sd.sStaffLvl, sd.sNameText, i);
			if (sd.sScrollLvl < RUNE_MIN)
				app_fatal("Invalid sScrollLvl %d for %s (%d)", sd.sScrollLvl, sd.sNameText, i);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d)", sd.sStaffCost, sd.sNameText, i);
			runeSpells++;
			continue;
		}
		if (sd.sBookLvl != SPELL_NA) {
			if (sd.sType != STYPE_NONE && sd.sType != STYPE_FIRE && sd.sType != STYPE_MAGIC && sd.sType != STYPE_LIGHTNING)
				app_fatal("Invalid sType %d for %s (%d)", sd.sType, sd.sNameText, i);
			if (sd.sBookLvl < BOOK_MIN)
				app_fatal("Invalid sBookLvl %d for %s (%d)", sd.sBookLvl, sd.sNameText, i);
			if (sd.sBookCost <= 0)
				app_fatal("Invalid sBookCost %d for %s (%d)", sd.sBookCost, sd.sNameText, i);
			bookSpells++;
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
			staffSpells++;
		}
		if (sd.sScrollLvl != SPELL_NA) {
			if (sd.sScrollLvl < SCRL_MIN)
				app_fatal("Invalid sScrollLvl %d for %s (%d)", sd.sScrollLvl, sd.sNameText, i);
			if (sd.sStaffCost <= 0)
				app_fatal("Invalid sStaffCost %d for %s (%d)", sd.sStaffCost, sd.sNameText, i);
			if ((sd.sSkillFlags & SDFLAG_TARGETED) && sd.scCurs == CURSOR_NONE)
				app_fatal("Targeted skill %s (%d) does not have scCurs.", sd.sNameText, i);
			scrollSpells++;
		}
		if (sd.sMissile != 0 && missiledata[sd.sMissile].mFileNum != MFILE_NONE && missiledata[sd.sMissile].mFileNum >= NUM_FIXMFILE)
			app_fatal("Skill %s (%d) uses a dynamically loaded missile (%d).", sd.sNameText, i, missiledata[sd.sMissile].mFileNum);
		if (sd.sMissile != 0 && sd.sType == STYPE_NONE && !(sd.sUseFlags & SFLAG_RANGED)) // required by On_SKILLXY, On_SKILLMON, On_SKILLPLR
			app_fatal("Skill %s (%d) supposed to use a missile, but neither sType nor the SFLAG_RANGED-flag is set.", sd.sNameText, i);
		//if (!(sd.sUseFlags & SFLAG_DUNGEON) && sd.sType != STYPE_NONE && sd.sType != STYPE_MAGIC && i != SPL_NULL)
		//	app_fatal("GFX is not loaded in town for skill %s (%d).", sd.sNameText, i); // required by InitPlayerGFX
		if (sd.sType != STYPE_NONE && !SFX_VALID(sd.sSFX))
			app_fatal("Skill %s (%d) does not have a valid sfx-id.", sd.sNameText, i); // required by On_SKILLXY, On_SKILLRXY, On_SKILLMON, On_SKILLPLR
		if ((sd.sMissile == MIS_OPITEM || sd.sMissile == MIS_REPAIR) && !SFX_VALID(sd.sSFX))
			app_fatal("Item-Skill %s (%d) does not have a valid sfx-id.", sd.sNameText, i); // required by On_OPERATEITEM
	}
	if (!bookSpells)
		app_fatal("No book spell for GetBookSpell.");
	if (!staffSpells)
		app_fatal("No staff spell for GetStaffSpell.");
	if (!scrollSpells)
		app_fatal("No scroll spell for GetScrollSpell.");
#ifdef HELLFIRE
	if (!runeSpells)
		app_fatal("No rune spell for GetRuneSpell.");
#endif
	const int abilitySpells = 5;
	if (bookSpells + staffSpells + scrollSpells + runeSpells + abilitySpells > 2 * NUM_SPELLS) {
		app_fatal("Too many spells for DrawSkillList (%d, %d, %d, %d, %d vs %d).", bookSpells, staffSpells, scrollSpells, runeSpells, abilitySpells, 2 * NUM_SPELLS);
	}

	// missiles
	for (i = 0; i < NUM_MISTYPES; i++) {
		const MissileData& md = missiledata[i];
		if (md.mAddProc == NULL)
			app_fatal("Missile %d has no valid mAddProc.", i);
		/*if ((md.mAddProc == AddBleed || md.mAddProc == AddBloodBoil || md.mAddProc == AddFireexp || md.mAddProc == AddInferno || md.mAddProc == AddMisexp)
		 && md.mdRange != misfiledata[md.mFileNum].mfAnimFrameLen * misfiledata[md.mFileNum].mfAnimLen[0])
			app_fatal("Animated-Missile %d has invalid duration (%d, expected %d).", i, md.mdRange, misfiledata[md.mFileNum].mfAnimFrameLen * misfiledata[md.mFileNum].mfAnimLen[0]);*/
		if ((md.mProc == MI_Misexp || md.mProc == MI_MiniExp || md.mProc == MI_LongExp || md.mProc == MI_Bleed || md.mProc == MI_BloodBoil || md.mProc == MI_Inferno || md.mProc == MI_Acidsplat
#ifdef HELLFIRE
			 || md.mProc == MI_HorkSpawn
#endif
			)
		 && md.mdRange != misfiledata[md.mFileNum].mfAnimFrameLen * misfiledata[md.mFileNum].mfAnimLen[0]) {
			if (md.mAddProc != AddAttract)
				app_fatal("Animated-Missile %d has invalid duration (%d, expected %d).", i, md.mdRange, misfiledata[md.mFileNum].mfAnimFrameLen * misfiledata[md.mFileNum].mfAnimLen[0]);
			else if (md.mdRange != misfiledata[md.mFileNum].mfAnimFrameLen * misfiledata[md.mFileNum].mfAnimLen[0] /2u)
				app_fatal("Animated-Missile %d has invalid duration (%d, expected %d).", i, md.mdRange, misfiledata[md.mFileNum].mfAnimFrameLen * misfiledata[md.mFileNum].mfAnimLen[0] / 2u);
		}
		if (md.mProc == MI_ExtExp
		 && md.mdRange < misfiledata[MFILE_SHATTER1].mfAnimFrameLen * misfiledata[MFILE_SHATTER1].mfAnimLen[0]) {
			app_fatal("Animated-Missile %d has invalid duration (%d, expected at least %d).", i, md.mdRange, misfiledata[MFILE_SHATTER1].mfAnimFrameLen * misfiledata[MFILE_SHATTER1].mfAnimLen[0]);
		}
		if (md.mAddProc == AddCharge && md.mdPrSpeed != (int)(MIS_SHIFTEDVEL(16) / M_SQRT2))
			app_fatal("Charge-Missile %d has invalid projectile-speed (%d, expected %d).", i, md.mdPrSpeed, (int)(MIS_SHIFTEDVEL(16) / M_SQRT2));
		if (md.mAddProc == AddMisexp) {
			assert(misfiledata[md.mFileNum].mfAnimFrameLen == 1);
		}
		if (md.mAddProc == AddTelekinesis) {
			for (int n = 0; n < NUM_SPELLS; n++) {
				if (spelldata[n].sMissile == i)
					assert(spelldata[n].sSkillFlags & SDFLAG_TARGETED);
			}
		}
		if (md.mProc == NULL)
			app_fatal("Missile %d has no valid mProc.", i);
		if (md.mProc == MI_Misexp) {
			for (int j = 0; j < misfiledata[md.mFileNum].mfAnimFAmt; j++) {
				assert(misfiledata[md.mFileNum].mfAnimLen[j] < 16 /* lengthof(ExpLight) */);
			}
		}
		if (md.mProc == MI_MiniExp) {
			for (int j = 0; j < misfiledata[md.mFileNum].mfAnimFAmt; j++) {
				assert(misfiledata[md.mFileNum].mfAnimLen[j] < 11 /* lengthof(ExpLight) */);
			}
		}
		if (md.mProc == MI_Inferno) {
			for (int j = 0; j < misfiledata[md.mFileNum].mfAnimFAmt; j++) {
				assert(misfiledata[md.mFileNum].mfAnimLen[j] < 24);
			}
		}
		if (md.mProc == MI_Cbolt) {
			assert(md.mdPrSpeed == missiledata[MIS_CBOLT].mdPrSpeed);
		}
		if (md.mProc == MI_Chain) {
			assert(md.mdPrSpeed == missiledata[MIS_CHAIN].mdPrSpeed);
		}
		if (md.mProc == MI_Elemental) {
			assert(md.mdPrSpeed == missiledata[MIS_ELEMENTAL].mdPrSpeed);
		}
		if (md.mProc == MI_Mage) {
			assert(md.mdPrSpeed == missiledata[MIS_MAGE].mdPrSpeed);
		}
		if (md.mProc == MI_Acidpud)
			assert(md.mFileNum == MFILE_ACIDPUD);
		if (md.mProc == MI_BloodBoil) {
			assert(md.mAddProc == AddBloodBoil);
			// assert(md.mFileNum == MFILE_BLODBURS);
		}
		if (md.mProc == MI_Firewall || md.mProc == MI_FireWave)
			assert(md.mFileNum == MFILE_FIREWAL);
		if (md.mProc == MI_Flash)
			assert(md.mFileNum == MFILE_BLUEXFR);
		if (md.mProc == MI_Flash2)
			assert(md.mFileNum == MFILE_BLUEXBK);
		if (md.mProc == MI_Guardian) {
			assert(md.mFileNum == MFILE_GUARD);
			assert(misfiledata[md.mFileNum].mfAnimFAmt == 3);
		}
		if (md.mProc == MI_Portal) {
			for (int j = 0; j < 16; j++) {
				assert(misfiledata[md.mFileNum].mfAnimLen[j] == misfiledata[MFILE_PORTAL].mfAnimLen[j]);
			}
		}
		if (md.mProc == MI_Shroud)
			assert(md.mFileNum == MFILE_SHROUD);
		if (md.mProc == MI_Wind)
			assert(md.mFileNum == MFILE_WIND);
		if (md.mProc == MI_Acidpud || md.mProc == MI_Firewall || md.mProc == MI_FireWave || md.mProc == MI_Flash || md.mProc == MI_Flash2
		 || md.mProc == MI_Guardian || md.mProc == MI_Portal || md.mProc == MI_Shroud || md.mProc == MI_Wind) {
			if (misfiledata[md.mFileNum].mfAnimFrameLen[j] != 1)
				app_fatal("Animated-Missile %d depending on mfAnimFrameLen is not a single stepper.", i);
		}
		if (md.mProc == MI_Shroud || md.mProc == MI_FireWave || md.mProc == MI_Portal || md.mProc == MI_Firewall || md.mProc == MI_Acidpud || md.mProc == MI_Wind) {
			assert(misfiledata[md.mFileNum].mfAnimFAmt == 2);
		}
		if ((md.mProc == MI_Acidpud || md.mProc == MI_Flash2) != misfiledata[md.mFileNum].mfPreFlag)
			app_fatal("Missile %d wont render correctly due to misconfigured preflag.", i);
		if (md.mdFlags & MIF_ARROW) {
			if (md.mAddProc != AddArrow)
				app_fatal("Arrow-Missile %d is not added by AddArrow proc.", i); // required to initialize MISDIST / MISHIT
			if (md.mProc != MI_Arrow && md.mProc != MI_AsArrow)
				app_fatal("Arrow-Missile %d is not handled by MI_*Arrow proc.", i); // required to maintain MISDIST
			if (i != MIS_ARROW && i != MIS_PBARROW && i != MIS_ASARROW && i != MIS_MLARROW && i != MIS_PCARROW)
				app_fatal("Arrow-Missile %d is not handled in MissMonHitByPlr and in MissPlrHitByPlr.", i);
		} else {
			if (md.mAddProc == AddApocaC2 && !(md.mdFlags & MIF_AREA))
				app_fatal("Magic-Missile %d damage-direction is not handled in MissDirection.", i);
		}
	}
	for (i = 0; i < NUM_MFILE; i++) {
		const MisFileData& mfd = misfiledata[i];
		if (i != MFILE_NONE && !mfd.mfDrawFlag)
			app_fatal("Missile-File %d is not rendered.", i);
		if (mfd.mfAnimFAmt < 0)
			app_fatal("Missile-File %d has negative mfAnimFAmt.", i);
		if (mfd.mfAnimFAmt == 0) {
			if (i != MFILE_NONE && i != MFILE_ACTOR)
				app_fatal("Missile-File %d without animation.", i);
		} else if (mfd.mfAnimXOffset != (mfd.mfAnimWidth - TILE_WIDTH) / 2)
			app_fatal("Missile-File %d is not drawn to the center. Width: %d, Offset: %d", i, mfd.mfAnimWidth, mfd.mfAnimXOffset);
		if (mfd.mfAnimFAmt > NUM_DIRS && mfd.mfAnimFAmt != 16)
			app_fatal("Missile-File %d has invalid mfAnimFAmt.", i); // required by AddMissile
		if (mfd.mfAnimFrameLen == 0) {
			if (mfd.mfAnimFlag && mfd.mfAnimFAmt != 0)
				app_fatal("Missile-File %d has invalid mfAnimFrameLen.", i);
		} else {
			if (!mfd.mfAnimFlag) {
				app_fatal("Missile-File %d has unused mfAnimFrameLen setting.", i);
			}
		}
		for (int n = 0; n < 16; n++) {
			if (n < mfd.mfAnimFAmt) {
				if (mfd.mfAnimLen[n] == 0 /*&& mfd.mfAnimFlag*/) {
					app_fatal("Missile-File %d has invalid mfAnimLen.", i, n);
				}
			} else {
				if (mfd.mfAnimLen[n] != 0) {
					app_fatal("Missile-File %d has unused mfAnimLen setting (%d).", i, n);
				}
			}
		}
	}
#endif // DEBUG_DATA
	assert((missiledata[MIS_ASARROW].mdFlags & MIF_SHROUD) == 0); // required by MI_AsArrow
	assert((missiledata[MIS_ARROW].mdFlags & MIF_ARROW) != 0);   // required by MissMonHitByPlr, MissPlrHitByPlr
	assert((missiledata[MIS_PBARROW].mdFlags & MIF_ARROW) != 0); // required by MissMonHitByPlr, MissPlrHitByPlr
	assert((missiledata[MIS_ASARROW].mdFlags & MIF_ARROW) != 0); // required by MissMonHitByPlr, MissPlrHitByPlr
	assert((missiledata[MIS_MLARROW].mdFlags & MIF_ARROW) != 0); // required by MissMonHitByPlr, MissPlrHitByPlr
	assert((missiledata[MIS_PCARROW].mdFlags & MIF_ARROW) != 0); // required by MissMonHitByPlr, MissPlrHitByPlr
	assert(missiledata[MIS_EXFIRE].mdPrSpeed == 0);              // required by AddElementalExplosion
	assert(missiledata[MIS_EXLGHT].mdPrSpeed == 0);              // required by AddElementalExplosion
	assert(missiledata[MIS_EXMAGIC].mdPrSpeed == 0);             // required by AddElementalExplosion
	assert(missiledata[MIS_EXACID].mdPrSpeed == 0);              // required by AddElementalExplosion
	assert(missiledata[MIS_FLASH2].mdPrSpeed == 0);              // required by AddFlash
	assert(missiledata[MIS_INFERNO].mdPrSpeed == 0);             // required by MI_InfernoC
	assert(missiledata[MIS_ACIDPUD].mdPrSpeed == 0);             // required by MI_Acidsplat
	assert(missiledata[MIS_FIREWALL].mdPrSpeed == 0);            // required by MI_Meteor, MI_WallC, AddRingC
	assert(missiledata[MIS_LIGHTNING].mdPrSpeed == 0);           // required by MI_LightningC
	assert(missiledata[MIS_LIGHTNING2].mdPrSpeed == 0);          // required by MI_LightningC
	assert(missiledata[MIS_BLOODBOIL].mdPrSpeed == 0);           // required by MI_BloodBoilC
	assert(missiledata[MIS_SWAMP].mdPrSpeed == 0);               // required by MI_BloodBoilC
	assert(missiledata[MIS_STONE].mdPrSpeed == 0);               // required by MI_Rune
	assert(misfiledata[MFILE_LGHNING].mfAnimLen[0] == misfiledata[MFILE_THINLGHT].mfAnimLen[0]); // required by AddLightning
	assert(misfiledata[MFILE_MINILTNG].mfAnimLen[0] == misfiledata[MFILE_LGHNING].mfAnimLen[0]); // required by MI_Pulse
	assert(misfiledata[MFILE_FIREWAL].mfAnimLen[0] < 14 /* lengthof(FireWallLight) */);          // required by MI_Firewall
	assert(missiledata[MIS_FIREWALL].mlSFX == LS_WALLLOOP);                                      // required by MI_Firewall
	assert(missiledata[MIS_FIREWALL].mlSFXCnt == 1);                                             // required by MI_Firewall
	assert(misfiledata[MFILE_WIND].mfAnimLen[0] == 12);                                          // required by AddWind + GetDamageAmt to set/calculate damage
	assert(misfiledata[MFILE_RPORTAL].mfAnimLen[0] < 17 /* lengthof(ExpLight) */);               // required by MI_Portal
	assert(misfiledata[MFILE_PORTAL].mfAnimLen[0] < 17 /* lengthof(ExpLight) */);                // required by MI_Portal
	assert(misfiledata[MFILE_PORTAL].mfAnimLen[0] == misfiledata[MFILE_RPORTAL].mfAnimLen[0]);   // required by MI_Portal
	assert(misfiledata[MFILE_FIREWAL].mfAnimLen[0] < 14 /* lengthof(FireWallLight) */);          // required by MI_FireWave
	assert(misfiledata[MFILE_FIREBA].mfAnimFrameLen == 1);                                       // required by MI_Meteor
	assert(((1 + misfiledata[MFILE_GUARD].mfAnimLen[0]) >> 1) <= MAX_LIGHT_RAD);                 // required by MI_Guardian
	assert(misfiledata[MFILE_LGHNING].mfAnimFAmt == 1);                                          // required by MI_Cbolt
	assert(misfiledata[MFILE_SHATTER1].mfAnimFAmt == 1);                                         // required by MI_Stone
	assert(misfiledata[MFILE_ARROWS].mfAnimLen[0] == 16);                                        // required by AddArrow
	assert(misfiledata[MFILE_FARROW].mfAnimFAmt == 16);                                          // required by AddArrow
	assert(misfiledata[MFILE_LARROW].mfAnimFAmt == 16);                                          // required by AddArrow
	assert(misfiledata[MFILE_MARROW].mfAnimFAmt == 16);                                          // required by AddArrow
	assert(misfiledata[MFILE_PARROW].mfAnimFAmt == 16);                                          // required by AddArrow
	assert(misfiledata[missiledata[MIS_EXFIRE].mFileNum].mfAnimFAmt < NUM_DIRS);                 // required by AddElementalExplosion
	assert(misfiledata[missiledata[MIS_EXLGHT].mFileNum].mfAnimFAmt < NUM_DIRS);                 // required by AddElementalExplosion
	assert(misfiledata[missiledata[MIS_EXMAGIC].mFileNum].mfAnimFAmt < NUM_DIRS);                // required by AddElementalExplosion
	assert(misfiledata[missiledata[MIS_EXACID].mFileNum].mfAnimFAmt < NUM_DIRS);                 // required by AddElementalExplosion
	assert(misfiledata[missiledata[MIS_ACIDPUD].mFileNum].mfAnimFAmt < NUM_DIRS);                // required by MI_Acidsplat
	assert(misfiledata[missiledata[MIS_EXACIDP].mFileNum].mfAnimFAmt < NUM_DIRS);                // required by MI_Acid
	assert(misfiledata[missiledata[MIS_LIGHTNING].mFileNum].mfAnimFAmt < NUM_DIRS);              // required by MI_LightningC
	assert(misfiledata[missiledata[MIS_LIGHTNING2].mFileNum].mfAnimFAmt < NUM_DIRS);             // required by MI_LightningC
	assert(misfiledata[missiledata[MIS_FIREWAVE].mFileNum].mfAnimFAmt < NUM_DIRS);               // required by AddFireWaveC
	assert(misfiledata[missiledata[MIS_FIREWALL].mFileNum].mfAnimFAmt < NUM_DIRS);               // required by AddRingC, MI_WallC, MI_Meteor
	assert(misfiledata[missiledata[MIS_LIGHTBALL].mFileNum].mfAnimFAmt < NUM_DIRS);              // required by AddNovaC
	assert(misfiledata[missiledata[MIS_BLEED].mFileNum].mfAnimFAmt < NUM_DIRS);                  // required by MonHitByPlr, PlrHitByAny
	assert(misfiledata[missiledata[MIS_FIREBOLT].mFileNum].mfAnimFAmt == 16 && missiledata[MIS_FIREBOLT].mdPrSpeed != 0); // required by Sentfire
//	assert(monfiledata[MOFILE_SNAKE].moAnimFrames[MA_ATTACK] == 13);                             // required by MI_Rhino
	assert(monfiledata[MOFILE_SNAKE].moAnimFrameLen[MA_ATTACK] == 1);                            // required by MI_Rhino
	assert(monfiledata[MOFILE_MAGMA].moAnimFrameLen[MA_SPECIAL] == 1);                           // required by MonDoRSpAttack
	// requirements by InitMonsterGFX
	assert((int)MFILE_MAGBALL >= (int)NUM_FIXMFILE);
	// assert((int)MFILE_KRULL >= (int)NUM_FIXMFILE);
	assert((int)MFILE_THINLGHT >= (int)NUM_FIXMFILE);
	assert((int)MFILE_ACIDBF >= (int)NUM_FIXMFILE);
	assert((int)MFILE_ACIDSPLA >= (int)NUM_FIXMFILE);
	assert((int)MFILE_ACIDPUD >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SCUBMISB >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SCBSEXPB >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SCUBMISD >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SCBSEXPD >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SCUBMISC >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SCBSEXPC >= (int)NUM_FIXMFILE);
	assert((int)MFILE_MAGEMIS >= (int)NUM_FIXMFILE);
	assert((int)MFILE_MAGEEXP >= (int)NUM_FIXMFILE);
	assert((int)MFILE_FIREPLAR >= (int)NUM_FIXMFILE);
#ifdef HELLFIRE
	assert((int)MFILE_MS_ORA_B >= (int)NUM_FIXMFILE);
	assert((int)MFILE_EXORA1_B >= (int)NUM_FIXMFILE);
	assert((int)MFILE_MS_ORA >= (int)NUM_FIXMFILE);
	assert((int)MFILE_EXORA1 >= (int)NUM_FIXMFILE);
	assert((int)MFILE_MS_REB_B >= (int)NUM_FIXMFILE);
	assert((int)MFILE_EXYEL2_B >= (int)NUM_FIXMFILE);
	assert((int)MFILE_SPAWNS >= (int)NUM_FIXMFILE);
	assert((int)MFILE_MS_ORA_A >= (int)NUM_FIXMFILE);
	assert((int)MFILE_EXORA1_A >= (int)NUM_FIXMFILE);
	assert((int)MFILE_MS_YEB_A >= (int)NUM_FIXMFILE);
	assert((int)MFILE_EXYEL2_A >= (int)NUM_FIXMFILE);
#endif
	// -- requirements by InitMonsterGFX end
	// players
	assert(PlrAnimFrameLens[PGX_WALK] == 1); // required by PlrDoWalk
	assert(PlrAnimFrameLens[PGX_ATTACK] == 1); // required by PlrDoAttack, PlrDoRangeAttack
	assert(PlrAnimFrameLens[PGX_FIRE] == 1 && PlrAnimFrameLens[PGX_LIGHTNING] == 1 && PlrAnimFrameLens[PGX_MAGIC] == 1); // required by PlrDoSpell
	assert(PlrAnimFrameLens[PGX_DEATH] > 1); // required by PlrDoDeath
#ifdef DEBUG_DATA
	int wal = -1;
	for (i = 0; i < NUM_CLASSES; i++) {
		int pnum = 0;
		plr._pClass = i;
		for (int k = 0; k < 2; k++) {
			currLvl._dType = k == 0 ? DTYPE_CATHEDRAL : DTYPE_TOWN;
			for (int n = ANIM_ID_UNARMED; n <= ANIM_ID_STAFF; n++) {
				plr._pgfxnum = n;
				SetPlrAnims(0);
				if (wal < 0)
					wal = plr._pAnims[PGX_WALK].paFrames;
				else if (wal != (int)plr._pAnims[PGX_WALK].paFrames)
					app_fatal("Inconsistent walk-animation for class %d with anim %d in %s", i, n, currLvl._dType == DTYPE_TOWN ? "town" : "dungeon"); // required by StartWalk
				if (n != ANIM_ID_BOW && plr._pAFNum == 0) {
					app_fatal("Invalid attack-actionframe number for class %d with anim %d in %s", i, n, currLvl._dType == DTYPE_TOWN ? "town" : "dungeon"); // required by PlrDoAttack
				}
				if (plr._pAnims[PGX_ATTACK].paFrames < plr._pAFNum) {
					app_fatal("Invalid attack-animation setting for class %d with anim %d in %s", i, n, currLvl._dType == DTYPE_TOWN ? "town" : "dungeon");
				}
				if (plr._pAnims[PGX_FIRE].paFrames < plr._pSFNum) {
					app_fatal("Invalid skill-animation (fire) setting for class %d with anim %d in %s", i, n, currLvl._dType == DTYPE_TOWN ? "town" : "dungeon");
				}
				if (plr._pAnims[PGX_LIGHTNING].paFrames < plr._pSFNum) {
					app_fatal("Invalid skill-animation (fire) setting for class %d with anim %d in %s", i, n, currLvl._dType == DTYPE_TOWN ? "town" : "dungeon");
				}
				if (plr._pAnims[PGX_MAGIC].paFrames < plr._pSFNum) {
					app_fatal("Invalid skill-animation (fire) setting for class %d with anim %d in %s", i, n, currLvl._dType == DTYPE_TOWN ? "town" : "dungeon");
				}
			}
		}
	}
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
#endif // DEBUG_DATA
}
#endif /* DEBUG_MODE || DEV_MODE */

#if DEV_MODE
void LogErrorF(const char* msg, ...)
{
	char tmp[256];
	//snprintf(tmp, sizeof(tmp), "f:\\logdebug%d_%d.txt", mypnum, SDL_ThreadID());
	snprintf(tmp, sizeof(tmp), "f:\\logdebug%d.txt", mypnum);
	FILE* f0 = FileOpen(tmp, "a+");
	if (f0 == NULL)
		return;

	va_list va;

	va_start(va, msg);

	vsnprintf(tmp, sizeof(tmp), msg, va);

	va_end(va);

	fputs(tmp, f0);

	using namespace std::chrono;
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	snprintf(tmp, sizeof(tmp), " @ %llu", ms.count());
	// snprintf(tmp, sizeof(tmp), " @ %u", gdwGameLogicTurn);
	fputs(tmp, f0);

	fputc('\n', f0);

	fclose(f0);
}

std::vector<std::string> errorMsgQueue;
void LogErrorQ(const char* msg, ...)
{
	char tmp[256];

	va_list va;

	va_start(va, msg);

	vsnprintf(tmp, sizeof(tmp), msg, va);

	va_end(va);

	using namespace std::chrono;
	milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	snprintf(tmp, sizeof(tmp), "%s @ %llu", tmp, ms.count());
	// snprintf(tmp, sizeof(tmp), "%s @ %u", tmp, gdwGameLogicTurn);

	errorMsgQueue.push_back(tmp);
}

void LogDumpQ()
{
	char tmp[256];
	snprintf(tmp, sizeof(tmp), "f:\\logdebug%d.txt", mypnum);
	FILE* f0 = FileOpen(tmp, "a+");
	if (f0 == NULL)
		return;

	for (const std::string &msg : errorMsgQueue) {
		fputs(msg.c_str(), f0);

		fputc('\n', f0);
	}

	errorMsgQueue.clear();

	fclose(f0);
}

#endif /* DEV_MODE */

DEVILUTION_END_NAMESPACE
