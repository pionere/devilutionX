#include "ttf_render_wrapped.h"

#include <cstddef>
#include <cstring>
#include "utils/sdl_compat.h"
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

namespace {

SDL_bool CharacterIsDelimiter(char c, const char *delimiters)
{
	while (*delimiters != '\0') {
		if (c == *delimiters)
			return SDL_TRUE;
		++delimiters;
	}
	return SDL_FALSE;
}

} // namespace

// Based on SDL 2.0.12 TTF_RenderUTF8_Blended_Wrapped
SDL_Surface *RenderUTF8_Solid_Wrapped(TTF_Font *font, const char *text, SDL_Color fg, Uint32 wrapLength, const int xAlign)
{
	int width, height;
	SDL_Surface *textbuf;
	const int lineSpace = 2;
	char *str, **strLines;

	/* Get the dimensions of the text surface */
	if (TTF_SizeUTF8(font, text, &width, &height) < 0 || width == 0) {
		TTF_SetError("Text has zero width");
		return NULL;
	}

	unsigned numLines = 1;
	str = NULL;
	strLines = NULL;
	if (wrapLength > 0 && *text != '\0') {
		const char *wrapDelims = " \t\r\n";
		int w, h;
		char *spot, *tok, *nextTok, *end;
		char delim;
		const unsigned strLen = std::strlen(text);

		numLines = 0;

		str = SDL_stack_alloc(char, strLen + 1);
		if (str == NULL) {
			TTF_SetError("Out of memory");
			return NULL;
		}

		std::memcpy(str, text, strLen + 1);
		tok = str;
		end = str + strLen;
		do {
			strLines = (char **)SDL_realloc(strLines, (numLines + 1) * sizeof(*strLines));
			if (strLines == NULL) {
				TTF_SetError("Out of memory");
				return NULL;
			}
			strLines[numLines++] = tok;

			/* Look for the end of the line */
			if ((spot = SDL_strchr(tok, '\r')) != NULL || (spot = SDL_strchr(tok, '\n')) != NULL) {
				if (*spot == '\r') {
					++spot;
				}
				if (*spot == '\n') {
					++spot;
				}
			} else {
				spot = end;
			}
			nextTok = spot;

			/* Get the longest string that will fit in the desired space */
			for (;;) {
				/* Strip trailing whitespace */
				while (spot > tok && CharacterIsDelimiter(spot[-1], wrapDelims) == SDL_TRUE) {
					--spot;
				}
				if (spot == tok) {
					if (CharacterIsDelimiter(*spot, wrapDelims) == SDL_TRUE) {
						*spot = '\0';
					}
					break;
				}
				delim = *spot;
				*spot = '\0';

				TTF_SizeUTF8(font, tok, &w, &h);
				if ((Uint32)w <= wrapLength) {
					break;
				}
				/* Back up and try again... */
				*spot = delim;

				while (spot > tok && (CharacterIsDelimiter(spot[-1], wrapDelims) == SDL_FALSE)) {
					--spot;
				}
				if (spot > tok) {
					nextTok = spot;
				}
			}
			tok = nextTok;
		} while (tok < end);
	}

	if (strLines == NULL) {
		SDL_stack_free(str);
		return TTF_RenderUTF8_Solid(font, text, fg);
	}

	/* Create the target surface */
	textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE, (numLines > 1) ? wrapLength : width, height * numLines + (lineSpace * (numLines - 1)), 8, 0, 0, 0, 0);
	if (textbuf == NULL) {
		if (strLines != NULL)
			SDL_free(strLines);
		SDL_stack_free(str);
		return NULL;
	}

	/* Fill the palette with the foreground color */
	SDL_Palette *palette = textbuf->format->palette;
	palette->colors[0].r = 255 - fg.r;
	palette->colors[0].g = 255 - fg.g;
	palette->colors[0].b = 255 - fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	SDLC_SetColorKey(textbuf, 0);

	// Reduced space between lines to roughly match Diablo.
	const int lineskip = TTF_FontLineSkip(font) * 7 / 10; // avoids forced int > float > int conversion
	SDL_Rect dest = { 0, 0, 0, 0 };
	for (unsigned line = 0; line < numLines; line++) {
		text = strLines[line];
		if (text == NULL || *text == '\0') {
			dest.y += lineskip;
			continue;
		}
		SDL_Surface *tmp = TTF_RenderUTF8_Solid(font, text, fg);
		if (tmp == NULL) {
			SDL_Log("%s", TTF_GetError());
			SDL_FreeSurface(textbuf);
			SDL_free(strLines);
			SDL_stack_free(str);
			return NULL;
		}

		dest.w = static_cast<Uint16>(tmp->w);
		dest.h = static_cast<Uint16>(tmp->h);

		switch (xAlign) {
		case TextAlignment_END:
			dest.x = textbuf->w - tmp->w;
			break;
		case TextAlignment_CENTER:
			dest.x = (textbuf->w - tmp->w) / 2;
			break;
		case TextAlignment_BEGIN:
			dest.x = 0;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		SDL_BlitSurface(tmp, NULL, textbuf, &dest);
		dest.y += lineskip;
		SDL_FreeSurface(tmp);
	}
	SDL_free(strLines);
	SDL_stack_free(str);
	return textbuf;
}

DEVILUTION_END_NAMESPACE
