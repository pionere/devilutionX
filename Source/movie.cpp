/**
 * @file movie.cpp
 *
 * Implementation of video playback.
 */
#include "all.h"
#include "../SourceX/display.h"

DEVILUTION_BEGIN_NAMESPACE

/** Should the movie continue playing. */
bool gbMoviePlaying = false;

/**
 * @brief Start playback of a given video.
 * @param pszMovie The file name of the video
 * @param movieFlags flags to control the playback, see movie_flags enum.
 */
void play_movie(const char *pszMovie, int movieFlags)
{
	HANDLE video_stream;

	gbMoviePlaying = true;
	sound_disable_music();
	stream_stop();
	effects_play_sound("Sfx\\Misc\\blank.wav");

	video_stream = SVidPlayBegin(pszMovie, (movieFlags & MOV_LOOP) ? 0x100C0808 : 0x10280808);
	MSG Msg;
	while (video_stream != NULL) {
		while (FetchMessage(&Msg)) {
			switch (Msg.message) {
			case DVL_WM_KEYDOWN:
				if (Msg.wParam == DVL_VK_ESCAPE)
					break;
			case DVL_WM_LBUTTONDOWN:
			case DVL_WM_RBUTTONDOWN:
				if (movieFlags & MOV_SKIP)
					break;
				continue;
			case DVL_WM_QUIT:
				SVidPlayEnd();
				diablo_quit(0);
				break;
			default:
				continue;
			}
			gbMoviePlaying = false;
			break;
		}
		if (!SVidPlayContinue() || !gbMoviePlaying) {
			SVidPlayEnd();
			break;
		}
	}
	sound_start_music();
	gbMoviePlaying = false;
	SDL_GetMouseState(&MouseX, &MouseY);
	OutputToLogical(&MouseX, &MouseY);
}

DEVILUTION_END_NAMESPACE
