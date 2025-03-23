#pragma once

namespace dvl {

#ifdef SCREEN_READER_INTEGRATION
void InitScreenReader();
void FreeScreenReader();
void SpeakText(const char* text);
#endif

} // namespace dvl
