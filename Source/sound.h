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

#ifndef NOSOUND
bool snd_playing(SoundSample* pSnd);
void snd_play_snd(SoundSample* pSnd, int lVolume, int lPan);
SoundSample* sound_file_load(const char *path);
void sound_file_cleanup(SoundSample* sound_file);
void snd_init();
void music_stop();
void music_start(int nTrack);
void sound_disable_music();
void sound_start_music();
int sound_get_music_volume();
void sound_set_music_volume(int volume);
int sound_get_sound_volume();
void sound_set_sound_volume(int volume);
#else
inline bool snd_playing(SoundSample* pSnd) { return false; }
inline void snd_play_snd(SoundSample* pSnd, int lVolume, int lPan) { }
inline void snd_init() { }
inline void music_stop() { }
inline void music_start(int nTrack) { }
inline void sound_disable_music() { }
inline void sound_start_music() { }
inline void sound_set_music_volume(int volume) { }
inline void sound_set_sound_volume(int volume)  { }
inline int sound_get_music_volume() { return 0; }
inline int sound_get_sound_volume() { return 0; }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SOUND_H__ */
