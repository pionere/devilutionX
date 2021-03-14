#include "text.h"

DEVILUTION_BEGIN_NAMESPACE

unsigned GetArtStrWidth(const char *str, unsigned size)
{
	unsigned strWidth = 0;

	for (unsigned i = 0; i < strlen(str); i++) {
		BYTE w = FontTables[size][*(BYTE *)&str[i] + 2];
		if (w)
			strWidth += w;
		else
			strWidth += FontTables[size][0];
	}

	return strWidth;
}

void WordWrapArtStr(char *text, unsigned width)
{
	const unsigned len = strlen(text);
	unsigned lineStart = 0;
	for (unsigned i = 0; i <= len; i++) {
		if (text[i] == '\n') {
			lineStart = i + 1;
			continue;
		} else if (text[i] != ' ' && i != len) {
			continue;
		}

		if (i != len)
			text[i] = '\0';
		if (GetArtStrWidth(&text[lineStart], AFT_SMALL) <= width) {
			if (i != len)
				text[i] = ' ';
			continue;
		}

		unsigned j;
		for (j = i; j >= lineStart; j--) {
			if (text[j] == ' ') {
				break; // Scan for previous space
			}
		}

		if (j == lineStart) { // Single word longer then width
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
