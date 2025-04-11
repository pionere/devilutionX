#include "screen_reader.hpp"

#ifdef SCREEN_READER_INTEGRATION

#include "all.h"
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Tolk.h>
#else
#include <speech-dispatcher/libspeechd.h>
#endif

DEVILUTION_BEGIN_NAMESPACE

static std::string SpokenText;

#ifdef _WIN32
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
	if (SpokenText == text)
		return;

	SpokenText = text;

	int textLen = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t* wText = new wchar_t[textLen];
	if (MultiByteToWideChar(CP_UTF8, 0, text, -1, &wText[0], textLen) == textLen) {
		Tolk_Output(&wText[0], true);
	}
	free(wText);
}

#else

static SPDConnection* Speechd;

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
	if (SpokenText == text)
		return;

	SpokenText = text;

	spd_say(Speechd, SPD_TEXT, text);
}

#endif // _WIN32

DEVILUTION_END_NAMESPACE

#endif // SCREEN_READER_INTEGRATION
