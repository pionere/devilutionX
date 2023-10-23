/**
 * @file diablo.h
 *
 * Interface of functions from the DiabloUI to the core game.
 */
#pragma once

#include <vector>

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/* Defined in effects.cpp */
/*#ifndef NOSOUND
void PlaySFX(int psfx, int rndCnt = 1);
#else
inline void PlaySFX(int psfx, int rndCnt = 1) { if (rndCnt > 1) random_low(165, rndCnt); }
#endif*/

/* Defined in pfile.cpp */
void pfile_ui_load_hero_infos(std::vector<_uiheroinfo>& hero_infos);
int pfile_ui_create_save(_uiheroinfo* heroinfo);
void pfile_ui_delete_save(_uiheroinfo* hero_info);

/* Defined in multi.cpp */
void multi_ui_handle_events(SNetEvent* pEvt);
int multi_ui_handle_turn();

/* Defined in plrmsg.cpp */
/*extern bool gbTalkflag;
void DrawPlrMsg(bool onTop);
void StartPlrMsg();
void VersionPlrMsg();
void StopPlrMsg();
void plrmsg_HandleMouseReleaseEvent();
bool plrmsg_presskey(int vkey);
void plrmsg_CatToText(const char* inBuf);
void plrmsg_HandleMouseMoveEvent();*/

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE
