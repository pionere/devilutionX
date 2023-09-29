#pragma once

#ifdef USE_SDL1

char* utf8_to_latin1(const char* in);

#else

#define utf8_to_latin1(in) SDL_iconv_string("8859-1", "UTF8", in, strlen(in))

#endif