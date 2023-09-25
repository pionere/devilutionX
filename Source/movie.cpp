/**
 * @file movie.cpp
 *
 * Implementation of video playback.
 */
#include "all.h"
#include "storm/storm_svid.h"
#include "utils/display.h"

DEVILUTION_BEGIN_NAMESPACE

static void GetMousePos(WPARAM wParam)
{
	MousePos.x = (int16_t)(wParam & 0xFFFF);
	MousePos.y = (int16_t)((wParam >> 16) & 0xFFFF);
}

/**
 * @brief Start playback of a given video.
 * @param pszMovie The file name of the video
 * @param movieFlags flags to control the playback, see movie_flags enum.
 * @return the reason why playback ended (movie_playback_result)
 */
int play_movie(const char* pszMovie, int movieFlags)
{
	int result = MPR_DONE;
	HANDLE video_stream;

	sound_disable_music();
	StopSFX();

	//video_stream = SVidPlayBegin(pszMovie, (movieFlags & MOV_LOOP) ? 0x100C0808 : 0x10280808);
	video_stream = SVidPlayBegin(pszMovie, movieFlags);
	MSG Msg;
	while (video_stream != NULL) {
		while (PeekMessage(&Msg)) {
			switch (Msg.message) {
			case DVL_WM_MOUSEMOVE:
				GetMousePos(Msg.wParam);
				continue;
			case DVL_WM_KEYDOWN:
				if (Msg.wParam == DVL_VK_ESCAPE) {
					result = MPR_CANCEL;
					break;
				}
			case DVL_WM_LBUTTONDOWN:
			case DVL_WM_RBUTTONDOWN:
				if (movieFlags & MOV_SKIP) {
					result = MPR_CANCEL;
					break;
				}
				continue;
			case DVL_WM_QUIT:
				if (gbRunGame) {
					NetSendCmd(CMD_DISCONNECT);
					gbRunGameResult = false;
				}
				result = MPR_QUIT;
				break;
			default:
				continue;
			}
			break;
		}
		if (!SVidPlayContinue() || result != MPR_DONE) {
			SVidPlayEnd();
			break;
		}
	}

	sound_restart_music();

	return result;
}

DEVILUTION_END_NAMESPACE
