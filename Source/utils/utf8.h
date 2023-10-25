#pragma once

#ifdef USE_SDL1

char* utf8_to_latin1(const char* in);
// TODO: implement latin1_to_utf8 for SDL1
#define latin1_to_utf8(in) strdup(in)

#else

#define utf8_to_latin1(in) SDL_iconv_string("8859-1", "UTF8", in, strlen(in))
#define latin1_to_utf8(in) SDL_iconv_string("UTF8", "8859-1", in, strlen(in))

#endif