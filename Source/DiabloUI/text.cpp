#include "text.h"

#include <string>

DEVILUTION_BEGIN_NAMESPACE

unsigned GetArtStrWidth(const char* str, unsigned size)
{
	unsigned strWidth = 0;

	for ( ; *str != '\0'; str++) {
		BYTE w = FontTables[size][*(BYTE*)str];
		strWidth += w;
	}

	return strWidth;
}

void WordWrapArtStr(char* text, unsigned width, unsigned size)
{
	const int len = strlen(text);
	int lineStart = 0;
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
		if (GetArtStrWidth(&text[lineStart], size) <= width) {
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
