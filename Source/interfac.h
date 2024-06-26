/**
 * @file interfac.h
 *
 * Interface of load screens.
 */
#ifndef __INTERFAC_H__
#define __INTERFAC_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

void interface_msg_pump();
void ShowCutscene(unsigned uMsg);
void IncProgress();
void LoadGameLevel(int lvldir);
void EnterLevel(BYTE lvl);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __INTERFAC_H__ */
