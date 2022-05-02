/**
 * @file error.cpp
 *
 * Implementation of in-game message functions.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

char msgtable[NUM_EMSGS];
Uint32 msgdelay;
BYTE currmsg;
BYTE msgcnt;

/** Maps from error_id to error message. */
const char *const MsgStrings[NUM_EMSGS] = {
	"",													// EMSG_NONE
	"Multiplayer sync problem",
	"Loading...",
	"Saving...",
	"New strength is forged through destruction.",		// EMSG_SHRINE_HIDDEN
	"Keep your swords sharp but wit sharper.",			// EMSG_SHRINE_GLOOMY
	"Know your priorities.",							// EMSG_SHRINE_WEIRD
	"Time cannot diminish the power of steel.",			// EMSG_SHRINE_RELIGIOUS
	"While the spirit is vigilant the body thrives.",	// EMSG_SHRINE_MAGICAL
	"The powers of mana refocused renews.",				// EMSG_SHRINE_STONE
	"Beware of too much power in one hand.",	 		// EMSG_SHRINE_CREEPY
	"What once was opened now is closed.",				// EMSG_SHRINE_THAUMATURGIC
	"Intensity comes at the cost of wisdom.",			// EMSG_SHRINE_FASCINATING
	"Let the spirits guide you.",						// EMSG_SHRINE_SHIMMERING
	"Arcane power brings destruction.",					// EMSG_SHRINE_CRYPTIC
	"Crimson and Azure become as the sun.",				// EMSG_SHRINE_ELDRITCH
	"Teamwork is the essence of survival.",				// EMSG_SHRINE_EERIE1
	"A helping soul fills your spirit.",				// EMSG_SHRINE_EERIE2
	"Where avarice fails, patience gains reward.",		// EMSG_SHRINE_SPOOKY1
	"Blessed by a benevolent companion!",				// EMSG_SHRINE_SPOOKY2
	"Thinking about others brings people closer.",		// EMSG_SHRINE_QUIET1
	"Someone mentioned you in their prayer.",			// EMSG_SHRINE_QUIET2
	"Drink and be refreshed.",							// EMSG_SHRINE_DIVINE
	"Wherever you go, there you are.",					// EMSG_SHRINE_HOLY
	"Energy comes at the cost of wisdom.",				// EMSG_SHRINE_SACRED
	"Salvation comes at the cost of wisdom.",			// EMSG_SHRINE_ORNATE
	"Riches abound when least expected.",				// EMSG_SHRINE_SPIRITUAL
	"The way is made clear when viewed from above.",	// EMSG_SHRINE_SECLUDED
	"Mysteries are revealed in the light of reason.",	// EMSG_SHRINE_GLIMMERING
	"There can be only one.",					 		// EMSG_SHRINE_TAINTED
	"The warmth of a fireside calls.",					// EMSG_SHRINE_GLISTENING
	"Arcane knowledge gained!",							// EMSG_BONECHAMB
	"Some experience is gained by touch.",				// EMSG_SHRINE_SPARKLING
	"That which can break will.",						// EMSG_SHRINE_MURPHYS
#ifdef HELLFIRE
	"Let the sun guide your path.",						// EMSG_SHRINE_SOLAR
#endif
// obsolete messages
// "No automap available in town"
// "No multiplayer functions in demo",
// "Direct Sound Creation Failed",
// "Not enough space to save",
// "Copying to a hard disk is recommended",
// "Not available in shareware version",
// "No Pause in town",
// "No pause in multiplayer",
// "Those who defend seldom attack",
// "Magic is not always what it seems to be."
// "Knowledge and wisdom at the cost of self",
// "The sword of justice is swift and sharp"
// "Some are weakened as one grows strong",
// "Strength is bolstered by heavenly faith",
// "The hands of men may be guided by fate",
// "That which cannot be held cannot be harmed",
// "The essence of life flows from within",
// "Those who are last may yet be first",
// "Generosity brings its own rewards",
// "That which does not kill you..."
// "There's no place like home.",
// "Knowledge is power."
// "Give and you shall receive.",
// "Spiritual energy is restored."
//	"You feel more agile.",
//	"You feel stronger.",
//	"You feel wiser.",
//	"You feel refreshed.",
//	"You must be at least level 8 to use this.",
//	"You must be at least level 13 to use this.",
//	"You must be at least level 17 to use this.",
};

void InitDiabloMsg(BYTE e)
{
	unsigned i;

	for (i = 0; i < msgcnt; i++) {
		if (msgtable[i] == e)
			return;
	}

	msgtable[msgcnt] = e;
	msgcnt++;

	currmsg = msgtable[0];
	msgdelay = SDL_GetTicks();
}

void ClrDiabloMsg()
{
	memset(msgtable, 0, sizeof(msgtable));

	currmsg = EMSG_NONE;
	msgcnt = 0;
}

void DrawDiabloMsg()
{
	int i, x, y, sx, sy;

	//assert(currmsg != EMSG_NONE);
	//assert(msgcnt > 0);

	x = PANEL_X + 101;
	y = DIALOG_Y;

	CelDraw(x, y, pSTextSlidCels, 1);
	CelDraw(x + 426, y, pSTextSlidCels, 4);
	CelDraw(x, y + 48, pSTextSlidCels, 2);
	CelDraw(x + 426, y + 48, pSTextSlidCels, 3);

	sx = x + 8;
	for (i = 0; i < 35; i++) {
		CelDraw(sx, y, pSTextSlidCels, 5);
		CelDraw(sx, y + 48, pSTextSlidCels, 7);
		sx += 12;
	}
	sy = y + 12;
	for (i = 0; i < 3; i++) {
		CelDraw(x, sy, pSTextSlidCels, 6);
		CelDraw(x + 426, sy, pSTextSlidCels, 8);
		sy += 12;
	}

	trans_rect(x + 3, y - 8, 432, 54);

	SStrCopy(tempstr, MsgStrings[currmsg], sizeof(tempstr));
	PrintString(x, y + 24, x + PANEL_WIDTH - 2 * 101, tempstr, true, COL_GOLD, FONT_KERN_SMALL);

	if (msgdelay > 0 && msgdelay <= SDL_GetTicks() - 3500) {
		msgdelay = 0;
	}
	if (msgdelay == 0) {
		msgcnt--;
		if (msgcnt == 0) {
			currmsg = EMSG_NONE;
		} else {
			currmsg = msgtable[msgcnt];
			msgdelay = SDL_GetTicks();
		}
	}
}

DEVILUTION_END_NAMESPACE
