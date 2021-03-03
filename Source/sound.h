/**
 * @file sound.h
 *
 * Interface of functions setting up the audio pipeline.
 */
#ifndef __SOUND_H__
#define __SOUND_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern bool gbMusicOn;
extern bool gbSoundOn;

void snd_update(BOOL bStopAll);
void snd_stop_snd(TSnd *pSnd);
bool snd_playing(TSnd *pSnd);
void snd_play_snd(TSnd *pSnd, int lVolume, int lPan);
TSnd *sound_file_load(const char *path);
void sound_file_cleanup(TSnd *sound_file);
void snd_init();
void music_stop();
void music_start(int nTrack);
void sound_disable_music();
void sound_start_music();
int sound_get_music_volume();
void sound_set_music_volume(int volume);
int sound_get_sound_volume();
void sound_set_sound_volume(int volume);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SOUND_H__ */
