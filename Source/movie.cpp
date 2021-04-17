/**
 * @file movie.cpp
 *
 * Implementation of video playback.
 */
#include "all.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

/** Should the movie continue playing. */
BOOLEAN movie_playing;

/**
 * @brief Start playback of a given video.
 * @param pszMovie The file name of the video
 * @param movieFlags flags to control the playback, see movie_flags enum.
 */
void play_movie(const char *pszMovie, int movieFlags)
{
	HANDLE video_stream = NULL;

	movie_playing = TRUE;
	sound_disable_music(TRUE);
	stream_stop();
	effects_play_sound("Sfx\\Misc\\blank.wav");

	SVidPlayBegin(pszMovie, (movieFlags & MOV_LOOP) ? 0x100C0808 : 0x10280808, &video_stream);
	MSG Msg;
	while (video_stream != NULL) {
		while (PeekMessage(&Msg)) {
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
				SVidPlayEnd(video_stream);
				diablo_quit(0);
				break;
			default:
				continue;
			}
			movie_playing = FALSE;
			break;
		}
		if (!SVidPlayContinue() || !movie_playing) {
			SVidPlayEnd(video_stream);
			break;
		}
	}
	sound_disable_music(FALSE);
	movie_playing = FALSE;
	SDL_GetMouseState(&MouseX, &MouseY);
	OutputToLogical(&MouseX, &MouseY);
}

DEVILUTION_END_NAMESPACE
