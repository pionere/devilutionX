#include "screen_reader.hpp"

#ifdef SCREEN_READER_INTEGRATION

#ifdef _WIN32
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Tolk.h>

namespace dvl {

void InitScreenReader()
{
	Tolk_Load();
}

void FreeScreenReader()
{
	Tolk_Unload();
}

void SpeakText(const char* text)
{
	static const char* SpokenText;

	if (SpokenText == text)
		return;

	SpokenText = text;

	int textLen = MultiByteToWideChar(CP_UTF8, 0, SpokenText, -1, NULL, 0);
	wchar_t* wText = new wchar_t[textLen];
	if (MultiByteToWideChar(CP_UTF8, 0, SpokenText, -1, &wText[0], textLen) == textLen) {
		Tolk_Output(&wText[0], true);
	}
	free(wText);
}

} // namespace dvl

#else
#include <speech-dispatcher/libspeechd.h>

namespace dvl {

SPDConnection* Speechd;

void InitScreenReader()
{
	Speechd = spd_open(PROJECT_NAME, PROJECT_NAME, NULL, SPD_MODE_SINGLE);
}

void FreeScreenReader()
{
	spd_close(Speechd);
}

void SpeakText(const char* text)
{
	static const char* SpokenText;

	if (SpokenText == text)
		return;

	SpokenText = text;

	spd_say(Speechd, SPD_TEXT, SpokenText);
}

} // namespace dvl

#endif // _WIN32

#endif // SCREEN_READER_INTEGRATION
