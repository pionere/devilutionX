/**
 * @file all.h
 *
 * Include all application headers.
 */
#ifndef __ALL_H__
#define __ALL_H__

#include <algorithm>
#include <cmath>
#include <climits>
#include <cstring>

// We include `cinttypes` here so that it is included before `inttypes.h`
// to work around a bug in older GCC versions on some platforms,
// where including `inttypes.h` before `cinttypes` leads to missing
// defines for `PRIuMAX` et al. SDL transitively includes `inttypes.h`.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=97044
#include <cinttypes>

#include <SDL.h>

#include "utils/sdl2_to_1_2_backports.h"
#include "utils/sdl2_backports.h"
#include "utils/sdl_compat.h"
#include "utils/log.h"

#include "../../defs.h"
#include "../../enums.h"
#include "../../structs.h"

#include "engine/engine.h"
#include "miniwin/miniwin.h"
//#include "utils/thread.h"

//#include "diabloui.h"
#include "gameui.h"
#include "display.h"
#include "dx.h"
//#include "utils/paths.h"
//#include "plrctrls.h"
#include "sound.h"

#include "appfat.h"
//#include "automap.h"
//#include "capture.h"
//#include "codec.h"
#include "control.h"
#include "cursor.h"
//#include "debug.h"
#include "diablo.h"
//#include "doom.h"
//#include "drlg_l1.h"
//#include "drlg_l2.h"
//#include "drlg_l3.h"
//#include "drlg_l4.h"
#include "drlp_l1.h"
#include "drlp_l2.h"
#include "drlp_l3.h"
#include "drlp_l4.h"
//#include "dthread.h"
#include "encrypt.h"
//#include "error.h"
//#include "gamemenu.h"
//#include "gendung.h"
//#include "gmenu.h"
//#include "help.h"
#include "init.h"
//#include "interfac.h"
//#include "inv.h"
//#include "itemdat.h"
//#include "items.h"
#include "lighting.h"
//#include "loadsave.h"
#include "mainmenu.h"
//#include "minitext.h"
//#include "minitextdat.h"
//#include "misdat.h"
//#include "missiles.h"
//#include "monstdat.h"
//#include "monster.h"
//#include "movie.h"
//#include "msg.h"
//#include "multi.h"
//#include "nthread.h"
//#include "objdat.h"
//#include "objects.h"
//#include "pack.h"
#include "palette.h"
#include "patchdung.h"
//#include "path.h"
//#include "pfile.h"
//#include "player.h"
//#include "plrmsg.h"
//#include "portal.h"
//#include "questdat.h"
//#include "quests.h"
#include "scrollrt.h"
#include "sfx.h"
//#include "sha.h"
//#include "spelldat.h"
//#include "spells.h"
//#include "stores.h"
//#include "sync.h"
//#include "themes.h"
//#include "tmsg.h"
//#include "town.h"
#include "townp.h"
//#include "towners.h"
//#include "track.h"
//#include "trigs.h"

#include "storm/storm.h"

#endif /* __ALL_H__ */
