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
#include "platform/vita/network.h"
#include "platform/vita/random.hpp"
#endif
#ifdef NXDK
#include <nxdk/mount.h>
#endif
#ifdef RUN_TESTS
#include <gtest/gtest.h>
#endif
#ifdef GPERF_HEAP_MAIN
#include <gperftools/heap-profiler.h>
#endif

#include "all.h"

#if DEBUG_MODE && !defined(__APPLE__)
extern "C" const char* __asan_default_options() // NOLINT(bugprone-reserved-identifier, readability-identifier-naming)
{
	return "halt_on_error=0";
}
#endif

//#ifdef __ANDROID__
//int SDL_main(int argc, char** argv)
//#else
//int main(int argc, char** argv)
//#endif
extern "C" int main(int argc, char** argv)
{
#ifdef RUN_TESTS
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif
#ifdef __SWITCH__
	switch_romfs_init();
	switch_enable_network();
	randombytes_switchrandom_init();
#endif
#ifdef __3DS__
	ctr_sys_init();
#endif
#ifdef __vita__
	scePowerSetArmClockFrequency(444);
	vita_enable_network();
	randombytes_vitarandom_init();
#endif
#ifdef NXDK
	nxMountDrive('E', "\\Device\\Harddisk0\\Partition1\\");
#endif
#ifdef GPERF_HEAP_MAIN
	HeapProfilerStart("main");
#endif
	const int result = dvl::DiabloMain(argc, argv);
#ifdef GPERF_HEAP_MAIN
	HeapProfilerStop();
#endif
	return result;
}
