#include "text.h"

#include <string>
#include "all.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

void WordWrapArtStr(char* text, unsigned width, unsigned size)
{
	const int len = (unsigned)strlen(text);
	int (*pStrWidth)(const char* str);
	int lineStart = 0;
	switch (size) {
	case AFT_SMALL: pStrWidth = GetSmallStringWidth; break;
	case AFT_BIG:   pStrWidth = GetBigStringWidth;   break;
	case AFT_HUGE:  pStrWidth = GetHugeStringWidth;  break;
	default:
		ASSUME_UNREACHABLE
	}
	for (int i = 0; i <= len; i++) {
		if (text[i] == '\n') {
			lineStart = i + 1;
			continue;
		}
		if (text[i] != ' ' && text[i] != '\0') {
			continue;
		}

		//if (i != len)
			text[i] = '\0';
		if (pStrWidth(&text[lineStart]) <= width) {
			if (i != len)
				text[i] = ' ';
			continue;
		}

		int j;
		for (j = i; j >= lineStart; j--) {
			if (text[j] == ' ') {
				break; // Scan for previous space
			}
		}

		if (j < lineStart) { // Single word longer than width
			if (i == len)
				break;
			j = i;
		}

		if (i != len)
			text[i] = ' ';
		text[j] = '\n';
		lineStart = j + 1;
	}
}

DEVILUTION_END_NAMESPACE
