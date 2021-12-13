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
extern int gnSoundVolume;
extern int gnMusicVolume;

#ifndef NOSOUND
void snd_play_snd(SoundSample* pSnd, int lVolume, int lPan);
void sound_file_load(const char *path, SoundSample* pSnd);
void InitSound();
void FreeSound();
void RestartMixer();
void music_stop();
void music_start(int nTrack);
void sound_disable_music();
void sound_start_music();
void sound_set_music_volume(int volume);
void sound_set_sound_volume(int volume);
#else
inline void snd_play_snd(SoundSample* pSnd, int lVolume, int lPan) { }
inline void InitSound() { }
inline void FreeSound() { }
inline void music_stop() { }
inline void music_start(int nTrack) { }
inline void sound_disable_music() { }
inline void sound_start_music() { }
inline void sound_set_music_volume(int volume) { }
inline void sound_set_sound_volume(int volume)  { }
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __SOUND_H__ */
