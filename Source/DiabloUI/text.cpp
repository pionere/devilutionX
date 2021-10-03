#include "text.h"

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
	const unsigned len = strlen(text);
	unsigned lineStart = 0;
	for (unsigned i = 0; i <= len; i++) {
		if (text[i] == '\n') {
			lineStart = i + 1;
			continue;
		}
		if (text[i] != ' ' && i != len) {
			continue;
		}

		if (i != len)
			text[i] = '\0';
		if (GetArtStrWidth(&text[lineStart], size) <= width) {
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

		if (j == lineStart) { // Single word longer than width
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
