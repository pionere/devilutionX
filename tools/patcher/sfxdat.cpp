/**
 * @file sfxdat.cpp
 *
 * Implementation of data related to sounds.
 */
#include "all.h"
#include "sfxdat.h"
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"

DEVILUTION_BEGIN_NAMESPACE

/** List of all sounds, except monsters and music */
const SFXStruct sgSFX[61] = {
	// clang-format off
//_sfx_id           bFlags,                   pszName
/*PS_WALK1*/     { sfx_MISC,                  "Sfx\\Misc\\Walk1.wav" },
/*PS_WALK2*///   { sfx_MISC,                  "Sfx\\Misc\\Walk2.wav" },
/*PS_WALK3*///   { sfx_MISC,                  "Sfx\\Misc\\Walk3.wav" },
/*PS_WALK4*///   { sfx_MISC,                  "Sfx\\Misc\\Walk4.wav" },
/*PS_BFIRE*/     { sfx_MISC,                  "Sfx\\Misc\\BFire.wav" },
/*PS_FMAG*///    { sfx_MISC,                  "Sfx\\Misc\\Fmag.wav" },
/*PS_TMAG*///    { sfx_MISC,                  "Sfx\\Misc\\Tmag.wav" },
/*PS_LGHIT*///   { sfx_MISC,                  "Sfx\\Misc\\Lghit.wav" },
/*PS_LGHIT1*///  { sfx_MISC,                  "Sfx\\Misc\\Lghit1.wav" },
/*PS_SWING*/     { sfx_MISC,                  "Sfx\\Misc\\Swing.wav" },
/*PS_SWING2*/    { sfx_MISC,                  "Sfx\\Misc\\Swing2.wav" },
/*PS_DEAD*/      { sfx_MISC,                  "Sfx\\Misc\\Dead.wav" }, // Aaauh...
/*IS_STING1*///  { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\Sting1.wav" },
/*IS_FBALLBOW*///{ sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\FBallBow.wav" },
/*IS_QUESTDN*/   { sfx_STREAM,                "Sfx\\Misc\\Questdon.wav" },
/*IS_ARMRFKD*/// { sfx_MISC,                  "Sfx\\Items\\Armrfkd.wav" },
/*IS_BARLFIRE*/  { sfx_MISC,                  "Sfx\\Items\\Barlfire.wav" },
/*IS_BARREL*/    { sfx_MISC,                  "Sfx\\Items\\Barrel.wav" },
/*IS_POPPOP8*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\PodPop8.wav" },
/*IS_POPPOP5*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\PodPop5.wav" },
/*IS_POPPOP3*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\UrnPop3.wav" },
/*IS_POPPOP2*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\UrnPop2.wav" },
/*IS_CRCLOS*/    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\Crclos.wav" },
/*IS_CROPEN*/    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\Cropen.wav" },
/*IS_BHIT*///    { sfx_MISC,                  "Sfx\\Items\\Bhit.wav" },
/*IS_BHIT1*///   { sfx_MISC,                  "Sfx\\Items\\Bhit1.wav" },
/*IS_CHEST*/     { sfx_MISC,                  "Sfx\\Items\\Chest.wav" },
/*IS_DOORCLOS*/  { sfx_MISC,                  "Sfx\\Items\\Doorclos.wav" },
/*IS_DOOROPEN*/  { sfx_MISC,                  "Sfx\\Items\\Dooropen.wav" },
/*IS_FANVL*/     { sfx_MISC,                  "Sfx\\Items\\Flipanvl.wav" },
/*IS_FAXE*/      { sfx_MISC,                  "Sfx\\Items\\Flipaxe.wav" },
/*IS_FBLST*/     { sfx_MISC,                  "Sfx\\Items\\Flipblst.wav" },
/*IS_FBODY*/     { sfx_MISC,                  "Sfx\\Items\\Flipbody.wav" },
/*IS_FBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Flipbook.wav" },
/*IS_FBOW*/      { sfx_MISC,                  "Sfx\\Items\\Flipbow.wav" },
/*IS_FCAP*/      { sfx_MISC,                  "Sfx\\Items\\Flipcap.wav" },
/*IS_FHARM*/     { sfx_MISC,                  "Sfx\\Items\\Flipharm.wav" },
/*IS_FLARM*/     { sfx_MISC,                  "Sfx\\Items\\Fliplarm.wav" },
/*IS_FMAG*///    { sfx_MISC,                  "Sfx\\Items\\Flipmag.wav" },
/*IS_FMAG1*///   { sfx_MISC,                  "Sfx\\Items\\Flipmag1.wav" },
/*IS_FMUSH*/     { sfx_MISC,                  "Sfx\\Items\\Flipmush.wav" },
/*IS_FPOT*/      { sfx_MISC,                  "Sfx\\Items\\Flippot.wav" },
/*IS_FRING*/     { sfx_MISC,                  "Sfx\\Items\\Flipring.wav" },
/*IS_FROCK*/     { sfx_MISC,                  "Sfx\\Items\\Fliprock.wav" },
/*IS_FSCRL*/     { sfx_MISC,                  "Sfx\\Items\\Flipscrl.wav" },
/*IS_FSHLD*/     { sfx_MISC,                  "Sfx\\Items\\Flipshld.wav" },
/*IS_FSIGN*///   { sfx_MISC,                  "Sfx\\Items\\Flipsign.wav" },
/*IS_FSTAF*/     { sfx_MISC,                  "Sfx\\Items\\Flipstaf.wav" },
/*IS_FSWOR*/     { sfx_MISC,                  "Sfx\\Items\\Flipswor.wav" },
/*IS_GOLD*/      { sfx_MISC,                  "Sfx\\Items\\Gold.wav" },
/*IS_HLMTFKD*/// { sfx_MISC,                  "Sfx\\Items\\Hlmtfkd.wav" },
/*IS_IANVL*/     { sfx_MISC,                  "Sfx\\Items\\Invanvl.wav" },
/*IS_IAXE*/      { sfx_MISC,                  "Sfx\\Items\\Invaxe.wav" },
/*IS_IBLST*/     { sfx_MISC,                  "Sfx\\Items\\Invblst.wav" },
/*IS_IBODY*/     { sfx_MISC,                  "Sfx\\Items\\Invbody.wav" },
/*IS_IBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Invbook.wav" },
/*IS_IBOW*/      { sfx_MISC,                  "Sfx\\Items\\Invbow.wav" },
/*IS_ICAP*/      { sfx_MISC,                  "Sfx\\Items\\Invcap.wav" },
/*IS_IGRAB*/     { sfx_MISC,                  "Sfx\\Items\\Invgrab.wav" },
/*IS_IHARM*/     { sfx_MISC,                  "Sfx\\Items\\Invharm.wav" },
/*IS_ILARM*/     { sfx_MISC,                  "Sfx\\Items\\Invlarm.wav" },
/*IS_IMUSH*/     { sfx_MISC,                  "Sfx\\Items\\Invmush.wav" },
/*IS_IPOT*/      { sfx_MISC,                  "Sfx\\Items\\Invpot.wav" },
/*IS_IRING*/     { sfx_MISC,                  "Sfx\\Items\\Invring.wav" },
/*IS_IROCK*/     { sfx_MISC,                  "Sfx\\Items\\Invrock.wav" },
/*IS_ISCROL*/    { sfx_MISC,                  "Sfx\\Items\\Invscrol.wav" },
/*IS_ISHIEL*/    { sfx_MISC,                  "Sfx\\Items\\Invshiel.wav" },
/*IS_ISIGN*/     { sfx_MISC,                  "Sfx\\Items\\Invsign.wav" },
/*IS_ISTAF*/     { sfx_MISC,                  "Sfx\\Items\\Invstaf.wav" },
/*IS_ISWORD*/    { sfx_MISC,                  "Sfx\\Items\\Invsword.wav" },
/*IS_LEVER*/     { sfx_MISC,                  "Sfx\\Items\\Lever.wav" },
/*IS_MAGIC*/     { sfx_MISC,                  "Sfx\\Items\\Magic.wav" },
/*IS_MAGIC1*/    { sfx_MISC,                  "Sfx\\Items\\Magic1.wav" },
/*IS_RBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Readbook.wav" },
/*IS_SARC*/      { sfx_MISC,                  "Sfx\\Items\\Sarc.wav" },
/*IS_SHLDFKD*/// { sfx_MISC,                  "Sfx\\Items\\Shielfkd.wav" },
/*IS_SWRDFKD*/// { sfx_MISC,                  "Sfx\\Items\\Swrdfkd.wav" },
/*IS_TITLEMOV*/  { sfx_UI,                    "Sfx\\Items\\Titlemov.wav" },
/*IS_TITLSLCT*/  { sfx_UI,                    "Sfx\\Items\\Titlslct.wav" },
	// clang-format on
};

DEVILUTION_END_NAMESPACE

#endif // NOSOUND
