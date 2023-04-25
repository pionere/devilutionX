#include <SDL.h>
#include <SDL_main.h>
#ifdef __SWITCH__
#include "platform/switch/network.h"
#include "platform/switch/random.hpp"
#include "platform/switch/romfs.hpp"
#endif
#ifdef __3DS__
#include "platform/ctr/system.h"
#endif
#ifdef __vita__
#include <psp2/power.h>
#endif

#include "all.h"

//#ifdef __ANDROID__
//int SDL_main(int argc, char** argv)
//#else
//int main(int argc, char** argv)
//#endif
extern "C" int main(int argc, char** argv)
{
#ifdef __SWITCH__
	switch_romfs_init();
	randombytes_switchrandom_init();
#endif
#ifdef __3DS__
	ctr_sys_init();
#endif
#ifdef __vita__
	scePowerSetArmClockFrequency(444);
#endif
	const int result = dvl::DiabloMain(argc, argv);
	return result;
}
