/**
 * @file effects.cpp
 *
 * Implementation of functions for loading and playing sounds.
 */
#include "all.h"
#ifndef NOSOUND
#include <SDL_mixer.h>
#include "utils/soundsample.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

#ifdef NOSOUND
const int sgSFXSets[NUM_SFXSets][NUM_CLASSES] = { };
#else
/** Specifies the sound file and the playback state of the current sound effect. */
static SFXStruct* sgpStreamSFX = NULL;

/** Maps from monster sfx to monster sound letter. */
static const char MonstSndChar[NUM_MON_SFX] = { 'a', 'h', 'd', 's' };

/** List of all sounds, except monsters and music */
static SFXStruct sgSFX[NUM_SFXS] = {
	// clang-format off
//_sfx_id           bFlags,                   pszName,                       pSnd
/*PS_WALK1*/     { sfx_MISC,                  "Sfx\\Misc\\Walk1.wav",        { 0, NULL } },
/*PS_WALK2*///   { sfx_MISC,                  "Sfx\\Misc\\Walk2.wav",        { 0, NULL } },
/*PS_WALK3*///   { sfx_MISC,                  "Sfx\\Misc\\Walk3.wav",        { 0, NULL } },
/*PS_WALK4*///   { sfx_MISC,                  "Sfx\\Misc\\Walk4.wav",        { 0, NULL } },
/*PS_BFIRE*/     { sfx_MISC,                  "Sfx\\Misc\\BFire.wav",        { 0, NULL } },
/*PS_FMAG*///    { sfx_MISC,                  "Sfx\\Misc\\Fmag.wav",         { 0, NULL } },
/*PS_TMAG*///    { sfx_MISC,                  "Sfx\\Misc\\Tmag.wav",         { 0, NULL } },
/*PS_LGHIT*///   { sfx_MISC,                  "Sfx\\Misc\\Lghit.wav",        { 0, NULL } },
/*PS_LGHIT1*///  { sfx_MISC,                  "Sfx\\Misc\\Lghit1.wav",       { 0, NULL } },
/*PS_SWING*/     { sfx_MISC,                  "Sfx\\Misc\\Swing.wav",        { 0, NULL } },
/*PS_SWING2*/    { sfx_MISC,                  "Sfx\\Misc\\Swing2.wav",       { 0, NULL } },
/*PS_DEAD*/      { sfx_MISC,                  "Sfx\\Misc\\Dead.wav",         { 0, NULL } }, // Aaauh...
/*IS_STING1*///  { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\Sting1.wav",       { 0, NULL } },
/*IS_FBALLBOW*///{ sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\FBallBow.wav",     { 0, NULL } },
/*IS_QUESTDN*/   { sfx_STREAM,                "Sfx\\Misc\\Questdon.wav",     { 0, NULL } },
/*IS_ARMRFKD*/// { sfx_MISC,                  "Sfx\\Items\\Armrfkd.wav",     { 0, NULL } },
/*IS_BARLFIRE*/  { sfx_MISC,                  "Sfx\\Items\\Barlfire.wav",    { 0, NULL } },
/*IS_BARREL*/    { sfx_MISC,                  "Sfx\\Items\\Barrel.wav",      { 0, NULL } },
/*IS_POPPOP8*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\PodPop8.wav",     { 0, NULL } },
/*IS_POPPOP5*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\PodPop5.wav",     { 0, NULL } },
/*IS_POPPOP3*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\UrnPop3.wav",     { 0, NULL } },
/*IS_POPPOP2*/   { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\UrnPop2.wav",     { 0, NULL } },
/*IS_CRCLOS*/    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\Crclos.wav",      { 0, NULL } },
/*IS_CROPEN*/    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Items\\Cropen.wav",      { 0, NULL } },
/*IS_BHIT*///    { sfx_MISC,                  "Sfx\\Items\\Bhit.wav",        { 0, NULL } },
/*IS_BHIT1*///   { sfx_MISC,                  "Sfx\\Items\\Bhit1.wav",       { 0, NULL } },
/*IS_CHEST*/     { sfx_MISC,                  "Sfx\\Items\\Chest.wav",       { 0, NULL } },
/*IS_DOORCLOS*/  { sfx_MISC,                  "Sfx\\Items\\Doorclos.wav",    { 0, NULL } },
/*IS_DOOROPEN*/  { sfx_MISC,                  "Sfx\\Items\\Dooropen.wav",    { 0, NULL } },
/*IS_FANVL*/     { sfx_MISC,                  "Sfx\\Items\\Flipanvl.wav",    { 0, NULL } },
/*IS_FAXE*/      { sfx_MISC,                  "Sfx\\Items\\Flipaxe.wav",     { 0, NULL } },
/*IS_FBLST*/     { sfx_MISC,                  "Sfx\\Items\\Flipblst.wav",    { 0, NULL } },
/*IS_FBODY*/     { sfx_MISC,                  "Sfx\\Items\\Flipbody.wav",    { 0, NULL } },
/*IS_FBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Flipbook.wav",    { 0, NULL } },
/*IS_FBOW*/      { sfx_MISC,                  "Sfx\\Items\\Flipbow.wav",     { 0, NULL } },
/*IS_FCAP*/      { sfx_MISC,                  "Sfx\\Items\\Flipcap.wav",     { 0, NULL } },
/*IS_FHARM*/     { sfx_MISC,                  "Sfx\\Items\\Flipharm.wav",    { 0, NULL } },
/*IS_FLARM*/     { sfx_MISC,                  "Sfx\\Items\\Fliplarm.wav",    { 0, NULL } },
/*IS_FMAG*///    { sfx_MISC,                  "Sfx\\Items\\Flipmag.wav",     { 0, NULL } },
/*IS_FMAG1*///   { sfx_MISC,                  "Sfx\\Items\\Flipmag1.wav",    { 0, NULL } },
/*IS_FMUSH*/     { sfx_MISC,                  "Sfx\\Items\\Flipmush.wav",    { 0, NULL } },
/*IS_FPOT*/      { sfx_MISC,                  "Sfx\\Items\\Flippot.wav",     { 0, NULL } },
/*IS_FRING*/     { sfx_MISC,                  "Sfx\\Items\\Flipring.wav",    { 0, NULL } },
/*IS_FROCK*/     { sfx_MISC,                  "Sfx\\Items\\Fliprock.wav",    { 0, NULL } },
/*IS_FSCRL*/     { sfx_MISC,                  "Sfx\\Items\\Flipscrl.wav",    { 0, NULL } },
/*IS_FSHLD*/     { sfx_MISC,                  "Sfx\\Items\\Flipshld.wav",    { 0, NULL } },
/*IS_FSIGN*///   { sfx_MISC,                  "Sfx\\Items\\Flipsign.wav",    { 0, NULL } },
/*IS_FSTAF*/     { sfx_MISC,                  "Sfx\\Items\\Flipstaf.wav",    { 0, NULL } },
/*IS_FSWOR*/     { sfx_MISC,                  "Sfx\\Items\\Flipswor.wav",    { 0, NULL } },
/*IS_GOLD*/      { sfx_MISC,                  "Sfx\\Items\\Gold.wav",        { 0, NULL } },
/*IS_HLMTFKD*/// { sfx_MISC,                  "Sfx\\Items\\Hlmtfkd.wav",     { 0, NULL } },
/*IS_IANVL*/     { sfx_MISC,                  "Sfx\\Items\\Invanvl.wav",     { 0, NULL } },
/*IS_IAXE*/      { sfx_MISC,                  "Sfx\\Items\\Invaxe.wav",      { 0, NULL } },
/*IS_IBLST*/     { sfx_MISC,                  "Sfx\\Items\\Invblst.wav",     { 0, NULL } },
/*IS_IBODY*/     { sfx_MISC,                  "Sfx\\Items\\Invbody.wav",     { 0, NULL } },
/*IS_IBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Invbook.wav",     { 0, NULL } },
/*IS_IBOW*/      { sfx_MISC,                  "Sfx\\Items\\Invbow.wav",      { 0, NULL } },
/*IS_ICAP*/      { sfx_MISC,                  "Sfx\\Items\\Invcap.wav",      { 0, NULL } },
/*IS_IGRAB*/     { sfx_MISC,                  "Sfx\\Items\\Invgrab.wav",     { 0, NULL } },
/*IS_IHARM*/     { sfx_MISC,                  "Sfx\\Items\\Invharm.wav",     { 0, NULL } },
/*IS_ILARM*/     { sfx_MISC,                  "Sfx\\Items\\Invlarm.wav",     { 0, NULL } },
/*IS_IMUSH*/     { sfx_MISC,                  "Sfx\\Items\\Invmush.wav",     { 0, NULL } },
/*IS_IPOT*/      { sfx_MISC,                  "Sfx\\Items\\Invpot.wav",      { 0, NULL } },
/*IS_IRING*/     { sfx_MISC,                  "Sfx\\Items\\Invring.wav",     { 0, NULL } },
/*IS_IROCK*/     { sfx_MISC,                  "Sfx\\Items\\Invrock.wav",     { 0, NULL } },
/*IS_ISCROL*/    { sfx_MISC,                  "Sfx\\Items\\Invscrol.wav",    { 0, NULL } },
/*IS_ISHIEL*/    { sfx_MISC,                  "Sfx\\Items\\Invshiel.wav",    { 0, NULL } },
/*IS_ISIGN*/     { sfx_MISC,                  "Sfx\\Items\\Invsign.wav",     { 0, NULL } },
/*IS_ISTAF*/     { sfx_MISC,                  "Sfx\\Items\\Invstaf.wav",     { 0, NULL } },
/*IS_ISWORD*/    { sfx_MISC,                  "Sfx\\Items\\Invsword.wav",    { 0, NULL } },
/*IS_LEVER*/     { sfx_MISC,                  "Sfx\\Items\\Lever.wav",       { 0, NULL } },
/*IS_MAGIC*/     { sfx_MISC,                  "Sfx\\Items\\Magic.wav",       { 0, NULL } },
/*IS_MAGIC1*/    { sfx_MISC,                  "Sfx\\Items\\Magic1.wav",      { 0, NULL } },
/*IS_RBOOK*/     { sfx_MISC,                  "Sfx\\Items\\Readbook.wav",    { 0, NULL } },
/*IS_SARC*/      { sfx_MISC,                  "Sfx\\Items\\Sarc.wav",        { 0, NULL } },
/*IS_SHLDFKD*/// { sfx_MISC,                  "Sfx\\Items\\Shielfkd.wav",    { 0, NULL } },
/*IS_SWRDFKD*/// { sfx_MISC,                  "Sfx\\Items\\Swrdfkd.wav",     { 0, NULL } },
/*IS_TITLEMOV*/  { sfx_UI,                    "Sfx\\Items\\Titlemov.wav",    { 0, NULL } },
/*IS_TITLSLCT*/  { sfx_UI,                    "Sfx\\Items\\Titlslct.wav",    { 0, NULL } },
/*SFX_SILENCE*/  { sfx_UI,                    "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*IS_TRAP*/      { sfx_MISC,                  "Sfx\\Items\\Trap.wav",        { 0, NULL } },
/*IS_CAST1*///   { sfx_MISC,                  "Sfx\\Misc\\Cast1.wav",        { 0, NULL } },
/*IS_CAST10*///  { sfx_MISC,                  "Sfx\\Misc\\Cast10.wav",       { 0, NULL } },
/*IS_CAST12*///  { sfx_MISC,                  "Sfx\\Misc\\Cast12.wav",       { 0, NULL } },
/*IS_CAST2*/     { sfx_MISC,                  "Sfx\\Misc\\Cast2.wav",        { 0, NULL } },
/*IS_CAST3*///   { sfx_MISC,                  "Sfx\\Misc\\Cast3.wav",        { 0, NULL } },
/*IS_CAST4*/     { sfx_MISC,                  "Sfx\\Misc\\Cast4.wav",        { 0, NULL } },
/*IS_CAST5*///   { sfx_MISC,                  "Sfx\\Misc\\Cast5.wav",        { 0, NULL } },
/*IS_CAST6*/     { sfx_MISC,                  "Sfx\\Misc\\Cast6.wav",        { 0, NULL } },
/*IS_CAST7*///   { sfx_MISC,                  "Sfx\\Misc\\Cast7.wav",        { 0, NULL } },
/*IS_CAST8*/     { sfx_MISC,                  "Sfx\\Misc\\Cast8.wav",        { 0, NULL } },
/*IS_CAST9*///   { sfx_MISC,                  "Sfx\\Misc\\Cast9.wav",        { 0, NULL } },
/*LS_HEALING*/// { sfx_MISC,                  "Sfx\\Misc\\Healing.wav",      { 0, NULL } },
/*IS_REPAIR*/    { sfx_MISC,                  "Sfx\\Misc\\Repair.wav",       { 0, NULL } },
/*LS_ACID*/      { sfx_MISC,                  "Sfx\\Misc\\Acids1.wav",       { 0, NULL } },
/*LS_ACIDS*/     { sfx_MISC,                  "Sfx\\Misc\\Acids2.wav",       { 0, NULL } },
/*LS_APOC*///    { sfx_MISC,                  "Sfx\\Misc\\Apoc.wav",         { 0, NULL } },
/*LS_ARROWALL*///{ sfx_MISC,                  "Sfx\\Misc\\Arrowall.wav",     { 0, NULL } },
/*LS_BLODBOIL*///{ sfx_MISC,                  "Sfx\\Misc\\Bldboil.wav",      { 0, NULL } },
/*LS_BLODSTAR*/  { sfx_MISC,                  "Sfx\\Misc\\Blodstar.wav",     { 0, NULL } },
/*LS_BLSIMPT*/// { sfx_MISC,                  "Sfx\\Misc\\Blsimpt.wav",      { 0, NULL } },
/*LS_BONESP*/    { sfx_MISC,                  "Sfx\\Misc\\Bonesp.wav",       { 0, NULL } },
/*LS_BSIMPCT*/// { sfx_MISC,                  "Sfx\\Misc\\Bsimpct.wav",      { 0, NULL } },
/*LS_CALDRON*/   { sfx_MISC,                  "Sfx\\Misc\\Caldron.wav",      { 0, NULL } },
/*LS_CBOLT*/     { sfx_MISC,                  "Sfx\\Misc\\Cbolt.wav",        { 0, NULL } },
/*LS_CHLTNING*///{ sfx_MISC,                  "Sfx\\Misc\\Chltning.wav",     { 0, NULL } },
/*LS_DSERP*///   { sfx_MISC,                  "Sfx\\Misc\\DSerp.wav",        { 0, NULL } },
/*LS_ELECIMP1*/  { sfx_MISC,                  "Sfx\\Misc\\Elecimp1.wav",     { 0, NULL } },
/*LS_ELEMENTL*/  { sfx_MISC,                  "Sfx\\Misc\\Elementl.wav",     { 0, NULL } },
/*LS_ETHEREAL*/  { sfx_MISC,                  "Sfx\\Misc\\Ethereal.wav",     { 0, NULL } },
/*LS_FBALL*///   { sfx_MISC,                  "Sfx\\Misc\\Fball.wav",        { 0, NULL } },
/*LS_FBOLT1*/    { sfx_MISC,                  "Sfx\\Misc\\Fbolt1.wav",       { 0, NULL } },
/*LS_FBOLT2*///  { sfx_MISC,                  "Sfx\\Misc\\Fbolt2.wav",       { 0, NULL } },
/*LS_FIRIMP1*/// { sfx_MISC,                  "Sfx\\Misc\\Firimp1.wav",      { 0, NULL } },
/*LS_FIRIMP2*/   { sfx_MISC,                  "Sfx\\Misc\\Firimp2.wav",      { 0, NULL } },
/*LS_FLAMWAVE*/  { sfx_MISC,                  "Sfx\\Misc\\Flamwave.wav",     { 0, NULL } },
/*LS_FLASH*///   { sfx_MISC,                  "Sfx\\Misc\\Flash.wav",        { 0, NULL } },
/*LS_FOUNTAIN*/  { sfx_MISC,                  "Sfx\\Misc\\Fountain.wav",     { 0, NULL } },
/*LS_GOLUM*/     { sfx_MISC,                  "Sfx\\Misc\\Golum.wav",        { 0, NULL } },
/*LS_GOLUMDED*///{ sfx_MISC,                  "Sfx\\Misc\\Golumded.wav",     { 0, NULL } },
/*LS_GSHRINE*/   { sfx_MISC,                  "Sfx\\Misc\\Gshrine.wav",      { 0, NULL } },
/*LS_GUARD*/     { sfx_MISC,                  "Sfx\\Misc\\Guard.wav",        { 0, NULL } },
/*LS_GUARDLAN*///{ sfx_MISC,                  "Sfx\\Misc\\Grdlanch.wav",     { 0, NULL } },
/*LS_HOLYBOLT*/  { sfx_MISC,                  "Sfx\\Misc\\Holybolt.wav",     { 0, NULL } },
/*LS_HYPER*///   { sfx_MISC,                  "Sfx\\Misc\\Hyper.wav",        { 0, NULL } },
/*LS_INFRAVIS*/  { sfx_MISC,                  "Sfx\\Misc\\Infravis.wav",     { 0, NULL } },
/*LS_INVISIBL*///{ sfx_MISC,                  "Sfx\\Misc\\Invisibl.wav",     { 0, NULL } },
/*LS_INVPOT*///  { sfx_MISC,                  "Sfx\\Misc\\Invpot.wav",       { 0, NULL } },
/*LS_LNING1*/    { sfx_MISC,                  "Sfx\\Misc\\Lning1.wav",       { 0, NULL } },
/*LS_LTNING*///  { sfx_MISC,                  "Sfx\\Misc\\Ltning.wav",       { 0, NULL } },
/*LS_MSHIELD*/   { sfx_MISC,                  "Sfx\\Misc\\Mshield.wav",      { 0, NULL } },
/*LS_NESTXPLD*///{ sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\NestXpld.wav",     { 0, NULL } },
/*LS_NOVA*/      { sfx_MISC,                  "Sfx\\Misc\\Nova.wav",         { 0, NULL } },
/*LS_PORTAL*///  { sfx_MISC,                  "Sfx\\Misc\\Portal.wav",       { 0, NULL } },
/*LS_PUDDLE*/    { sfx_MISC,                  "Sfx\\Misc\\Puddle.wav",       { 0, NULL } },
/*LS_RESUR*///   { sfx_MISC,                  "Sfx\\Misc\\Resur.wav",        { 0, NULL } },
/*LS_SCURSE*///  { sfx_MISC,                  "Sfx\\Misc\\Scurse.wav",       { 0, NULL } },
/*LS_SCURIMP*/   { sfx_MISC,                  "Sfx\\Misc\\Scurimp.wav",      { 0, NULL } },
/*LS_SENTINEL*/  { sfx_MISC,                  "Sfx\\Misc\\Sentinel.wav",     { 0, NULL } },
/*LS_SHATTER*/// { sfx_MISC,                  "Sfx\\Misc\\Shatter.wav",      { 0, NULL } },
/*LS_SOULFIRE*///{ sfx_MISC,                  "Sfx\\Misc\\Soulfire.wav",     { 0, NULL } },
/*LS_SPOUTLOP*///{ sfx_MISC,                  "Sfx\\Misc\\Spoutlop.wav",     { 0, NULL } },
/*LS_SPOUTSTR*/  { sfx_MISC,                  "Sfx\\Misc\\Spoutstr.wav",     { 0, NULL } },
/*LS_STORM*///   { sfx_MISC,                  "Sfx\\Misc\\Storm.wav",        { 0, NULL } },
/*LS_TRAPDIS*/   { sfx_MISC,                  "Sfx\\Misc\\Trapdis.wav",      { 0, NULL } },
/*LS_TELEPORT*/  { sfx_MISC,                  "Sfx\\Misc\\Teleport.wav",     { 0, NULL } },
/*LS_VTHEFT*///  { sfx_MISC,                  "Sfx\\Misc\\Vtheft.wav",       { 0, NULL } },
/*LS_WALLLOOP*/  { sfx_MISC,                  "Sfx\\Misc\\Wallloop.wav",     { 0, NULL } },
/*LS_WALLSTRT*///{ sfx_MISC,                  "Sfx\\Misc\\Wallstrt.wav",     { 0, NULL } },
/*LS_LMAG*///    { sfx_MISC | sfx_HELLFIRE,   "Sfx\\Misc\\LMag.wav",         { 0, NULL } },
/*TSFX_BMAID1*/  { sfx_STREAM,                "Sfx\\Towners\\Bmaid01.wav",   { 0, NULL } },
/*TSFX_BMAID2*/  { sfx_STREAM,                "Sfx\\Towners\\Bmaid02.wav",   { 0, NULL } },
/*TSFX_BMAID3*/  { sfx_STREAM,                "Sfx\\Towners\\Bmaid03.wav",   { 0, NULL } },
/*TSFX_BMAID4*/  { sfx_STREAM,                "Sfx\\Towners\\Bmaid04.wav",   { 0, NULL } },
/*TSFX_BMAID5*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid05.wav",   { 0, NULL } },
/*TSFX_BMAID6*/  { sfx_STREAM,                "Sfx\\Towners\\Bmaid06.wav",   { 0, NULL } },
/*TSFX_BMAID7*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid07.wav",   { 0, NULL } },
/*TSFX_BMAID8*/  { sfx_STREAM,                "Sfx\\Towners\\Bmaid08.wav",   { 0, NULL } },
/*TSFX_BMAID9*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid09.wav",   { 0, NULL } },
/*TSFX_BMAID10*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid10.wav",   { 0, NULL } },
/*TSFX_BMAID11*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid11.wav",   { 0, NULL } },
/*TSFX_BMAID12*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid12.wav",   { 0, NULL } },
/*TSFX_BMAID13*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid13.wav",   { 0, NULL } },
/*TSFX_BMAID14*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid14.wav",   { 0, NULL } },
/*TSFX_BMAID15*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid15.wav",   { 0, NULL } },
/*TSFX_BMAID16*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid16.wav",   { 0, NULL } },
/*TSFX_BMAID17*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid17.wav",   { 0, NULL } },
/*TSFX_BMAID18*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid18.wav",   { 0, NULL } },
/*TSFX_BMAID19*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid19.wav",   { 0, NULL } },
/*TSFX_BMAID20*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid20.wav",   { 0, NULL } },
/*TSFX_BMAID21*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid21.wav",   { 0, NULL } },
/*TSFX_BMAID22*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid22.wav",   { 0, NULL } },
/*TSFX_BMAID23*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid23.wav",   { 0, NULL } },
/*TSFX_BMAID24*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid24.wav",   { 0, NULL } },
/*TSFX_BMAID25*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid25.wav",   { 0, NULL } },
/*TSFX_BMAID26*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid26.wav",   { 0, NULL } },
/*TSFX_BMAID27*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid27.wav",   { 0, NULL } },
/*TSFX_BMAID28*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid28.wav",   { 0, NULL } },
/*TSFX_BMAID29*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid29.wav",   { 0, NULL } },
/*TSFX_BMAID30*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid30.wav",   { 0, NULL } },
/*TSFX_BMAID31*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid31.wav",   { 0, NULL } },
/*TSFX_BMAID32*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid32.wav",   { 0, NULL } },
/*TSFX_BMAID33*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid33.wav",   { 0, NULL } },
/*TSFX_BMAID34*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid34.wav",   { 0, NULL } },
/*TSFX_BMAID35*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid35.wav",   { 0, NULL } },
/*TSFX_BMAID36*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid36.wav",   { 0, NULL } },
/*TSFX_BMAID37*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid37.wav",   { 0, NULL } },
/*TSFX_BMAID38*///{ sfx_STREAM,                "Sfx\\Towners\\Bmaid38.wav",   { 0, NULL } },
/*TSFX_BMAID39*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid39.wav",   { 0, NULL } },
/*TSFX_BMAID40*/ { sfx_STREAM,                "Sfx\\Towners\\Bmaid40.wav",   { 0, NULL } },
/*TSFX_SMITH1*/  { sfx_STREAM,                "Sfx\\Towners\\Bsmith01.wav",  { 0, NULL } },
/*TSFX_SMITH2*/  { sfx_STREAM,                "Sfx\\Towners\\Bsmith02.wav",  { 0, NULL } },
/*TSFX_SMITH3*/  { sfx_STREAM,                "Sfx\\Towners\\Bsmith03.wav",  { 0, NULL } },
/*TSFX_SMITH4*/  { sfx_STREAM,                "Sfx\\Towners\\Bsmith04.wav",  { 0, NULL } },
/*TSFX_SMITH5*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith05.wav",  { 0, NULL } },
/*TSFX_SMITH6*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith06.wav",  { 0, NULL } },
/*TSFX_SMITH7*/  { sfx_STREAM,                "Sfx\\Towners\\Bsmith07.wav",  { 0, NULL } },
/*TSFX_SMITH8*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith08.wav",  { 0, NULL } },
/*TSFX_SMITH9*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith09.wav",  { 0, NULL } },
/*TSFX_SMITH10*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith10.wav",  { 0, NULL } },
/*TSFX_SMITH11*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith11.wav",  { 0, NULL } },
/*TSFX_SMITH12*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith12.wav",  { 0, NULL } },
/*TSFX_SMITH13*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith13.wav",  { 0, NULL } },
/*TSFX_SMITH14*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith14.wav",  { 0, NULL } },
/*TSFX_SMITH15*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith15.wav",  { 0, NULL } },
/*TSFX_SMITH16*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith16.wav",  { 0, NULL } },
/*TSFX_SMITH17*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith17.wav",  { 0, NULL } },
/*TSFX_SMITH18*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith18.wav",  { 0, NULL } },
/*TSFX_SMITH19*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith19.wav",  { 0, NULL } },
/*TSFX_SMITH20*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith20.wav",  { 0, NULL } },
/*TSFX_SMITH21*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith21.wav",  { 0, NULL } },
/*TSFX_SMITH22*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith22.wav",  { 0, NULL } },
/*TSFX_SMITH23*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith23.wav",  { 0, NULL } },
/*TSFX_SMITH24*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith24.wav",  { 0, NULL } },
/*TSFX_SMITH25*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith25.wav",  { 0, NULL } },
/*TSFX_SMITH26*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith26.wav",  { 0, NULL } },
/*TSFX_SMITH27*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith27.wav",  { 0, NULL } },
/*TSFX_SMITH28*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith28.wav",  { 0, NULL } },
/*TSFX_SMITH29*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith29.wav",  { 0, NULL } },
/*TSFX_SMITH30*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith30.wav",  { 0, NULL } },
/*TSFX_SMITH31*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith31.wav",  { 0, NULL } },
/*TSFX_SMITH32*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith32.wav",  { 0, NULL } },
/*TSFX_SMITH33*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith33.wav",  { 0, NULL } },
/*TSFX_SMITH34*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith34.wav",  { 0, NULL } },
/*TSFX_SMITH35*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith35.wav",  { 0, NULL } },
/*TSFX_SMITH36*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith36.wav",  { 0, NULL } },
/*TSFX_SMITH37*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith37.wav",  { 0, NULL } },
/*TSFX_SMITH38*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith38.wav",  { 0, NULL } },
/*TSFX_SMITH39*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith39.wav",  { 0, NULL } },
/*TSFX_SMITH40*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith40.wav",  { 0, NULL } },
/*TSFX_SMITH41*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith41.wav",  { 0, NULL } },
/*TSFX_SMITH42*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith42.wav",  { 0, NULL } },
/*TSFX_SMITH43*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith43.wav",  { 0, NULL } },
/*TSFX_SMITH44*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith44.wav",  { 0, NULL } },
/*TSFX_SMITH45*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith45.wav",  { 0, NULL } },
/*TSFX_SMITH46*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith46.wav",  { 0, NULL } },
/*TSFX_SMITH47*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith47.wav",  { 0, NULL } },
/*TSFX_SMITH48*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith48.wav",  { 0, NULL } },
/*TSFX_SMITH49*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith49.wav",  { 0, NULL } },
/*TSFX_SMITH50*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith50.wav",  { 0, NULL } },
/*TSFX_SMITH51*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith51.wav",  { 0, NULL } },
/*TSFX_SMITH52*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith52.wav",  { 0, NULL } },
/*TSFX_SMITH53*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith53.wav",  { 0, NULL } },
/*TSFX_SMITH54*///{ sfx_STREAM,                "Sfx\\Towners\\Bsmith54.wav",  { 0, NULL } },
/*TSFX_SMITH55*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith55.wav",  { 0, NULL } },
/*TSFX_SMITH56*/ { sfx_STREAM,                "Sfx\\Towners\\Bsmith56.wav",  { 0, NULL } },
/*TSFX_COW1*/    { sfx_STREAM,                "Sfx\\Towners\\Cow1.wav",      { 0, NULL } },
/*TSFX_COW2*/    { sfx_STREAM,                "Sfx\\Towners\\Cow2.wav",      { 0, NULL } },
/*TSFX_COW3*///  { sfx_MISC,                  "Sfx\\Towners\\Cow3.wav",      { 0, NULL } },
/*TSFX_COW4*///  { sfx_MISC,                  "Sfx\\Towners\\Cow4.wav",      { 0, NULL } },
/*TSFX_COW5*///  { sfx_MISC,                  "Sfx\\Towners\\Cow5.wav",      { 0, NULL } },
/*TSFX_COW6*///  { sfx_MISC,                  "Sfx\\Towners\\Cow6.wav",      { 0, NULL } },
/*TSFX_COW7*///  { sfx_MISC,                  "Sfx\\Towners\\Cow7.wav",      { 0, NULL } },
/*TSFX_COW8*///  { sfx_MISC,                  "Sfx\\Towners\\Cow8.wav",      { 0, NULL } },
/*TSFX_DEADGUY*///{ sfx_STREAM,                "Sfx\\Towners\\Deadguy2.wav",  { 0, NULL } },
/*TSFX_DRUNK1*/  { sfx_STREAM,                "Sfx\\Towners\\Drunk01.wav",   { 0, NULL } },
/*TSFX_DRUNK2*/  { sfx_STREAM,                "Sfx\\Towners\\Drunk02.wav",   { 0, NULL } },
/*TSFX_DRUNK3*/  { sfx_STREAM,                "Sfx\\Towners\\Drunk03.wav",   { 0, NULL } },
/*TSFX_DRUNK4*/  { sfx_STREAM,                "Sfx\\Towners\\Drunk04.wav",   { 0, NULL } },
/*TSFX_DRUNK5*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk05.wav",   { 0, NULL } },
/*TSFX_DRUNK6*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk06.wav",   { 0, NULL } },
/*TSFX_DRUNK7*/  { sfx_STREAM,                "Sfx\\Towners\\Drunk07.wav",   { 0, NULL } },
/*TSFX_DRUNK8*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk08.wav",   { 0, NULL } },
/*TSFX_DRUNK9*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk09.wav",   { 0, NULL } },
/*TSFX_DRUNK10*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk10.wav",   { 0, NULL } },
/*TSFX_DRUNK11*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk11.wav",   { 0, NULL } },
/*TSFX_DRUNK12*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk12.wav",   { 0, NULL } },
/*TSFX_DRUNK13*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk13.wav",   { 0, NULL } },
/*TSFX_DRUNK14*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk14.wav",   { 0, NULL } },
/*TSFX_DRUNK15*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk15.wav",   { 0, NULL } },
/*TSFX_DRUNK16*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk16.wav",   { 0, NULL } },
/*TSFX_DRUNK17*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk17.wav",   { 0, NULL } },
/*TSFX_DRUNK18*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk18.wav",   { 0, NULL } },
/*TSFX_DRUNK19*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk19.wav",   { 0, NULL } },
/*TSFX_DRUNK20*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk20.wav",   { 0, NULL } },
/*TSFX_DRUNK21*///{ sfx_STREAM,                "Sfx\\Towners\\Drunk21.wav",   { 0, NULL } },
/*TSFX_DRUNK22*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk22.wav",   { 0, NULL } },
/*TSFX_DRUNK23*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk23.wav",   { 0, NULL } },
/*TSFX_DRUNK24*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk24.wav",   { 0, NULL } },
/*TSFX_DRUNK25*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk25.wav",   { 0, NULL } },
/*TSFX_DRUNK26*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk26.wav",   { 0, NULL } },
/*TSFX_DRUNK27*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk27.wav",   { 0, NULL } },
/*TSFX_DRUNK28*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk28.wav",   { 0, NULL } },
/*TSFX_DRUNK29*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk29.wav",   { 0, NULL } },
/*TSFX_DRUNK30*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk30.wav",   { 0, NULL } },
/*TSFX_DRUNK31*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk31.wav",   { 0, NULL } },
/*TSFX_DRUNK32*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk32.wav",   { 0, NULL } },
/*TSFX_DRUNK33*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk33.wav",   { 0, NULL } },
/*TSFX_DRUNK34*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk34.wav",   { 0, NULL } },
/*TSFX_DRUNK35*/ { sfx_STREAM,                "Sfx\\Towners\\Drunk35.wav",   { 0, NULL } },
/*TSFX_HEALER1*/ { sfx_STREAM,                "Sfx\\Towners\\Healer01.wav",  { 0, NULL } },
/*TSFX_HEALER2*/ { sfx_STREAM,                "Sfx\\Towners\\Healer02.wav",  { 0, NULL } },
/*TSFX_HEALER3*/ { sfx_STREAM,                "Sfx\\Towners\\Healer03.wav",  { 0, NULL } },
/*TSFX_HEALER4*///{ sfx_STREAM,                "Sfx\\Towners\\Healer04.wav",  { 0, NULL } },
/*TSFX_HEALER5*/ { sfx_STREAM,                "Sfx\\Towners\\Healer05.wav",  { 0, NULL } },
/*TSFX_HEALER6*///{ sfx_STREAM,                "Sfx\\Towners\\Healer06.wav",  { 0, NULL } },
/*TSFX_HEALER7*///{ sfx_STREAM,                "Sfx\\Towners\\Healer07.wav",  { 0, NULL } },
/*TSFX_HEALER8*/ { sfx_STREAM,                "Sfx\\Towners\\Healer08.wav",  { 0, NULL } },
/*TSFX_HEALER9*///{ sfx_STREAM,                "Sfx\\Towners\\Healer09.wav",  { 0, NULL } },
/*TSFX_HEALER10*/{ sfx_STREAM,                "Sfx\\Towners\\Healer10.wav",  { 0, NULL } },
/*TSFX_HEALER11*/{ sfx_STREAM,                "Sfx\\Towners\\Healer11.wav",  { 0, NULL } },
/*TSFX_HEALER12*/{ sfx_STREAM,                "Sfx\\Towners\\Healer12.wav",  { 0, NULL } },
/*TSFX_HEALER13*/{ sfx_STREAM,                "Sfx\\Towners\\Healer13.wav",  { 0, NULL } },
/*TSFX_HEALER14*///{ sfx_STREAM,                "Sfx\\Towners\\Healer14.wav",  { 0, NULL } },
/*TSFX_HEALER15*///{ sfx_STREAM,                "Sfx\\Towners\\Healer15.wav",  { 0, NULL } },
/*TSFX_HEALER16*/{ sfx_STREAM,                "Sfx\\Towners\\Healer16.wav",  { 0, NULL } },
/*TSFX_HEALER17*///{ sfx_STREAM,                "Sfx\\Towners\\Healer17.wav",  { 0, NULL } },
/*TSFX_HEALER18*/{ sfx_STREAM,                "Sfx\\Towners\\Healer18.wav",  { 0, NULL } },
/*TSFX_HEALER19*///{ sfx_STREAM,                "Sfx\\Towners\\Healer19.wav",  { 0, NULL } },
/*TSFX_HEALER20*/{ sfx_STREAM,                "Sfx\\Towners\\Healer20.wav",  { 0, NULL } },
/*TSFX_HEALER21*/{ sfx_STREAM,                "Sfx\\Towners\\Healer21.wav",  { 0, NULL } },
/*TSFX_HEALER22*/{ sfx_STREAM,                "Sfx\\Towners\\Healer22.wav",  { 0, NULL } },
/*TSFX_HEALER23*///{ sfx_STREAM,                "Sfx\\Towners\\Healer23.wav",  { 0, NULL } },
/*TSFX_HEALER24*///{ sfx_STREAM,                "Sfx\\Towners\\Healer24.wav",  { 0, NULL } },
/*TSFX_HEALER25*///{ sfx_STREAM,                "Sfx\\Towners\\Healer25.wav",  { 0, NULL } },
/*TSFX_HEALER26*/{ sfx_STREAM,                "Sfx\\Towners\\Healer26.wav",  { 0, NULL } },
/*TSFX_HEALER27*/{ sfx_STREAM,                "Sfx\\Towners\\Healer27.wav",  { 0, NULL } },
/*TSFX_HEALER28*///{ sfx_STREAM,                "Sfx\\Towners\\Healer28.wav",  { 0, NULL } },
/*TSFX_HEALER29*///{ sfx_STREAM,                "Sfx\\Towners\\Healer29.wav",  { 0, NULL } },
/*TSFX_HEALER30*///{ sfx_STREAM,                "Sfx\\Towners\\Healer30.wav",  { 0, NULL } },
/*TSFX_HEALER31*///{ sfx_STREAM,                "Sfx\\Towners\\Healer31.wav",  { 0, NULL } },
/*TSFX_HEALER32*///{ sfx_STREAM,                "Sfx\\Towners\\Healer32.wav",  { 0, NULL } },
/*TSFX_HEALER33*///{ sfx_STREAM,                "Sfx\\Towners\\Healer33.wav",  { 0, NULL } },
/*TSFX_HEALER34*///{ sfx_STREAM,                "Sfx\\Towners\\Healer34.wav",  { 0, NULL } },
/*TSFX_HEALER35*///{ sfx_STREAM,                "Sfx\\Towners\\Healer35.wav",  { 0, NULL } },
/*TSFX_HEALER36*///{ sfx_STREAM,                "Sfx\\Towners\\Healer36.wav",  { 0, NULL } },
/*TSFX_HEALER37*/{ sfx_STREAM,                "Sfx\\Towners\\Healer37.wav",  { 0, NULL } },
/*TSFX_HEALER38*/{ sfx_STREAM,                "Sfx\\Towners\\Healer38.wav",  { 0, NULL } },
/*TSFX_HEALER39*/{ sfx_STREAM,                "Sfx\\Towners\\Healer39.wav",  { 0, NULL } },
/*TSFX_HEALER40*/{ sfx_STREAM,                "Sfx\\Towners\\Healer40.wav",  { 0, NULL } },
/*TSFX_HEALER41*/{ sfx_STREAM,                "Sfx\\Towners\\Healer41.wav",  { 0, NULL } },
/*TSFX_HEALER42*/{ sfx_STREAM,                "Sfx\\Towners\\Healer42.wav",  { 0, NULL } },
/*TSFX_HEALER43*/{ sfx_STREAM,                "Sfx\\Towners\\Healer43.wav",  { 0, NULL } },
/*TSFX_HEALER44*///{ sfx_STREAM,                "Sfx\\Towners\\Healer44.wav",  { 0, NULL } },
/*TSFX_HEALER45*/{ sfx_STREAM,                "Sfx\\Towners\\Healer45.wav",  { 0, NULL } },
/*TSFX_HEALER46*/{ sfx_STREAM,                "Sfx\\Towners\\Healer46.wav",  { 0, NULL } },
/*TSFX_HEALER47*/{ sfx_STREAM,                "Sfx\\Towners\\Healer47.wav",  { 0, NULL } },
/*TSFX_PEGBOY1*/ { sfx_STREAM,                "Sfx\\Towners\\Pegboy01.wav",  { 0, NULL } },
/*TSFX_PEGBOY2*/ { sfx_STREAM,                "Sfx\\Towners\\Pegboy02.wav",  { 0, NULL } },
/*TSFX_PEGBOY3*/ { sfx_STREAM,                "Sfx\\Towners\\Pegboy03.wav",  { 0, NULL } },
/*TSFX_PEGBOY4*/ { sfx_STREAM,                "Sfx\\Towners\\Pegboy04.wav",  { 0, NULL } },
/*TSFX_PEGBOY5*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy05.wav",  { 0, NULL } },
/*TSFX_PEGBOY6*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy06.wav",  { 0, NULL } },
/*TSFX_PEGBOY7*/ { sfx_STREAM,                "Sfx\\Towners\\Pegboy07.wav",  { 0, NULL } },
/*TSFX_PEGBOY8*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy08.wav",  { 0, NULL } },
/*TSFX_PEGBOY9*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy09.wav",  { 0, NULL } },
/*TSFX_PEGBOY10*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy10.wav",  { 0, NULL } },
/*TSFX_PEGBOY11*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy11.wav",  { 0, NULL } },
/*TSFX_PEGBOY12*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy12.wav",  { 0, NULL } },
/*TSFX_PEGBOY13*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy13.wav",  { 0, NULL } },
/*TSFX_PEGBOY14*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy14.wav",  { 0, NULL } },
/*TSFX_PEGBOY15*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy15.wav",  { 0, NULL } },
/*TSFX_PEGBOY16*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy16.wav",  { 0, NULL } },
/*TSFX_PEGBOY17*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy17.wav",  { 0, NULL } },
/*TSFX_PEGBOY18*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy18.wav",  { 0, NULL } },
/*TSFX_PEGBOY19*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy19.wav",  { 0, NULL } },
/*TSFX_PEGBOY20*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy20.wav",  { 0, NULL } },
/*TSFX_PEGBOY21*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy21.wav",  { 0, NULL } },
/*TSFX_PEGBOY22*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy22.wav",  { 0, NULL } },
/*TSFX_PEGBOY23*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy23.wav",  { 0, NULL } },
/*TSFX_PEGBOY24*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy24.wav",  { 0, NULL } },
/*TSFX_PEGBOY25*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy25.wav",  { 0, NULL } },
/*TSFX_PEGBOY26*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy26.wav",  { 0, NULL } },
/*TSFX_PEGBOY27*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy27.wav",  { 0, NULL } },
/*TSFX_PEGBOY28*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy28.wav",  { 0, NULL } },
/*TSFX_PEGBOY29*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy29.wav",  { 0, NULL } },
/*TSFX_PEGBOY30*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy30.wav",  { 0, NULL } },
/*TSFX_PEGBOY31*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy31.wav",  { 0, NULL } },
/*TSFX_PEGBOY32*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy32.wav",  { 0, NULL } },
/*TSFX_PEGBOY33*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy33.wav",  { 0, NULL } },
/*TSFX_PEGBOY34*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy34.wav",  { 0, NULL } },
/*TSFX_PEGBOY35*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy35.wav",  { 0, NULL } },
/*TSFX_PEGBOY36*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy36.wav",  { 0, NULL } },
/*TSFX_PEGBOY37*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy37.wav",  { 0, NULL } },
/*TSFX_PEGBOY38*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy38.wav",  { 0, NULL } },
/*TSFX_PEGBOY39*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy39.wav",  { 0, NULL } },
/*TSFX_PEGBOY40*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy40.wav",  { 0, NULL } },
/*TSFX_PEGBOY41*///{ sfx_STREAM,                "Sfx\\Towners\\Pegboy41.wav",  { 0, NULL } },
/*TSFX_PEGBOY42*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy42.wav",  { 0, NULL } },
/*TSFX_PEGBOY43*/{ sfx_STREAM,                "Sfx\\Towners\\Pegboy43.wav",  { 0, NULL } },
/*TSFX_PRIEST0*/ { sfx_STREAM,                "Sfx\\Towners\\Priest00.wav",  { 0, NULL } },
/*TSFX_PRIEST1*///{ sfx_STREAM,                "Sfx\\Towners\\Priest01.wav",  { 0, NULL } },
/*TSFX_PRIEST2*///{ sfx_STREAM,                "Sfx\\Towners\\Priest02.wav",  { 0, NULL } },
/*TSFX_PRIEST3*///{ sfx_STREAM,                "Sfx\\Towners\\Priest03.wav",  { 0, NULL } },
/*TSFX_PRIEST4*///{ sfx_STREAM,                "Sfx\\Towners\\Priest04.wav",  { 0, NULL } },
/*TSFX_PRIEST5*///{ sfx_STREAM,                "Sfx\\Towners\\Priest05.wav",  { 0, NULL } },
/*TSFX_PRIEST6*///{ sfx_STREAM,                "Sfx\\Towners\\Priest06.wav",  { 0, NULL } },
/*TSFX_PRIEST7*///{ sfx_STREAM,                "Sfx\\Towners\\Priest07.wav",  { 0, NULL } },
/*TSFX_STORY0*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt00.wav",  { 0, NULL } },
/*TSFX_STORY1*/  { sfx_STREAM,                "Sfx\\Towners\\Storyt01.wav",  { 0, NULL } },
/*TSFX_STORY2*/  { sfx_STREAM,                "Sfx\\Towners\\Storyt02.wav",  { 0, NULL } },
/*TSFX_STORY3*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt03.wav",  { 0, NULL } },
/*TSFX_STORY4*/  { sfx_STREAM,                "Sfx\\Towners\\Storyt04.wav",  { 0, NULL } },
/*TSFX_STORY5*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt05.wav",  { 0, NULL } },
/*TSFX_STORY6*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt06.wav",  { 0, NULL } },
/*TSFX_STORY7*/  { sfx_STREAM,                "Sfx\\Towners\\Storyt07.wav",  { 0, NULL } },
/*TSFX_STORY8*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt08.wav",  { 0, NULL } },
/*TSFX_STORY9*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt09.wav",  { 0, NULL } },
/*TSFX_STORY10*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt10.wav",  { 0, NULL } },
/*TSFX_STORY11*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt11.wav",  { 0, NULL } },
/*TSFX_STORY12*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt12.wav",  { 0, NULL } },
/*TSFX_STORY13*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt13.wav",  { 0, NULL } },
/*TSFX_STORY14*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt14.wav",  { 0, NULL } },
/*TSFX_STORY15*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt15.wav",  { 0, NULL } },
/*TSFX_STORY16*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt16.wav",  { 0, NULL } },
/*TSFX_STORY17*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt17.wav",  { 0, NULL } },
/*TSFX_STORY18*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt18.wav",  { 0, NULL } },
/*TSFX_STORY19*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt19.wav",  { 0, NULL } },
/*TSFX_STORY20*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt20.wav",  { 0, NULL } },
/*TSFX_STORY21*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt21.wav",  { 0, NULL } },
/*TSFX_STORY22*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt22.wav",  { 0, NULL } },
/*TSFX_STORY23*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt23.wav",  { 0, NULL } },
/*TSFX_STORY24*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt24.wav",  { 0, NULL } },
/*TSFX_STORY25*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt25.wav",  { 0, NULL } },
/*TSFX_STORY26*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt26.wav",  { 0, NULL } },
/*TSFX_STORY27*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt27.wav",  { 0, NULL } },
/*TSFX_STORY28*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt28.wav",  { 0, NULL } },
/*TSFX_STORY29*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt29.wav",  { 0, NULL } },
/*TSFX_STORY30*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt30.wav",  { 0, NULL } },
/*TSFX_STORY31*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt31.wav",  { 0, NULL } },
/*TSFX_STORY32*///{ sfx_STREAM,                "Sfx\\Towners\\Storyt32.wav",  { 0, NULL } },
/*TSFX_STORY33*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt33.wav",  { 0, NULL } },
/*TSFX_STORY34*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt34.wav",  { 0, NULL } },
/*TSFX_STORY35*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt35.wav",  { 0, NULL } },
/*TSFX_STORY36*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt36.wav",  { 0, NULL } },
/*TSFX_STORY37*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt37.wav",  { 0, NULL } },
/*TSFX_STORY38*/ { sfx_STREAM,                "Sfx\\Towners\\Storyt38.wav",  { 0, NULL } },
/*TSFX_TAVERN0*/ { sfx_STREAM,                "Sfx\\Towners\\Tavown00.wav",  { 0, NULL } },
/*TSFX_TAVERN1*/ { sfx_STREAM,                "Sfx\\Towners\\Tavown01.wav",  { 0, NULL } },
/*TSFX_TAVERN2*/ { sfx_STREAM,                "Sfx\\Towners\\Tavown02.wav",  { 0, NULL } },
/*TSFX_TAVERN3*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown03.wav",  { 0, NULL } },
/*TSFX_TAVERN4*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown04.wav",  { 0, NULL } },
/*TSFX_TAVERN5*/ { sfx_STREAM,                "Sfx\\Towners\\Tavown05.wav",  { 0, NULL } },
/*TSFX_TAVERN6*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown06.wav",  { 0, NULL } },
/*TSFX_TAVERN7*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown07.wav",  { 0, NULL } },
/*TSFX_TAVERN8*/ { sfx_STREAM,                "Sfx\\Towners\\Tavown08.wav",  { 0, NULL } },
/*TSFX_TAVERN9*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown09.wav",  { 0, NULL } },
/*TSFX_TAVERN10*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown10.wav",  { 0, NULL } },
/*TSFX_TAVERN11*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown11.wav",  { 0, NULL } },
/*TSFX_TAVERN12*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown12.wav",  { 0, NULL } },
/*TSFX_TAVERN13*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown13.wav",  { 0, NULL } },
/*TSFX_TAVERN14*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown14.wav",  { 0, NULL } },
/*TSFX_TAVERN15*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown15.wav",  { 0, NULL } },
/*TSFX_TAVERN16*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown16.wav",  { 0, NULL } },
/*TSFX_TAVERN17*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown17.wav",  { 0, NULL } },
/*TSFX_TAVERN18*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown18.wav",  { 0, NULL } },
/*TSFX_TAVERN19*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown19.wav",  { 0, NULL } },
/*TSFX_TAVERN20*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown20.wav",  { 0, NULL } },
/*TSFX_TAVERN21*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown21.wav",  { 0, NULL } },
/*TSFX_TAVERN22*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown22.wav",  { 0, NULL } },
/*TSFX_TAVERN23*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown23.wav",  { 0, NULL } },
/*TSFX_TAVERN24*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown24.wav",  { 0, NULL } },
/*TSFX_TAVERN25*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown25.wav",  { 0, NULL } },
/*TSFX_TAVERN26*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown26.wav",  { 0, NULL } },
/*TSFX_TAVERN27*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown27.wav",  { 0, NULL } },
/*TSFX_TAVERN28*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown28.wav",  { 0, NULL } },
/*TSFX_TAVERN29*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown29.wav",  { 0, NULL } },
/*TSFX_TAVERN30*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown30.wav",  { 0, NULL } },
/*TSFX_TAVERN31*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown31.wav",  { 0, NULL } },
/*TSFX_TAVERN32*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown32.wav",  { 0, NULL } },
/*TSFX_TAVERN33*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown33.wav",  { 0, NULL } },
/*TSFX_TAVERN34*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown34.wav",  { 0, NULL } },
/*TSFX_TAVERN35*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown35.wav",  { 0, NULL } },
/*TSFX_TAVERN36*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown36.wav",  { 0, NULL } },
/*TSFX_TAVERN37*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown37.wav",  { 0, NULL } },
/*TSFX_TAVERN38*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown38.wav",  { 0, NULL } },
/*TSFX_TAVERN39*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown39.wav",  { 0, NULL } },
/*TSFX_TAVERN40*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown40.wav",  { 0, NULL } },
/*TSFX_TAVERN41*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown41.wav",  { 0, NULL } },
/*TSFX_TAVERN42*///{ sfx_STREAM,                "Sfx\\Towners\\Tavown42.wav",  { 0, NULL } },
/*TSFX_TAVERN43*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown43.wav",  { 0, NULL } },
/*TSFX_TAVERN44*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown44.wav",  { 0, NULL } },
/*TSFX_TAVERN45*/{ sfx_STREAM,                "Sfx\\Towners\\Tavown45.wav",  { 0, NULL } },
/*TSFX_WITCH1*/  { sfx_STREAM,                "Sfx\\Towners\\Witch01.wav",   { 0, NULL } },
/*TSFX_WITCH2*/  { sfx_STREAM,                "Sfx\\Towners\\Witch02.wav",   { 0, NULL } },
/*TSFX_WITCH3*/  { sfx_STREAM,                "Sfx\\Towners\\Witch03.wav",   { 0, NULL } },
/*TSFX_WITCH4*/  { sfx_STREAM,                "Sfx\\Towners\\Witch04.wav",   { 0, NULL } },
/*TSFX_WITCH5*///{ sfx_STREAM,                "Sfx\\Towners\\Witch05.wav",   { 0, NULL } },
/*TSFX_WITCH6*///{ sfx_STREAM,                "Sfx\\Towners\\Witch06.wav",   { 0, NULL } },
/*TSFX_WITCH7*/  { sfx_STREAM,                "Sfx\\Towners\\Witch07.wav",   { 0, NULL } },
/*TSFX_WITCH8*///{ sfx_STREAM,                "Sfx\\Towners\\Witch08.wav",   { 0, NULL } },
/*TSFX_WITCH9*///{ sfx_STREAM,                "Sfx\\Towners\\Witch09.wav",   { 0, NULL } },
/*TSFX_WITCH10*/ { sfx_STREAM,                "Sfx\\Towners\\Witch10.wav",   { 0, NULL } },
/*TSFX_WITCH11*///{ sfx_STREAM,                "Sfx\\Towners\\Witch11.wav",   { 0, NULL } },
/*TSFX_WITCH12*/ { sfx_STREAM,                "Sfx\\Towners\\Witch12.wav",   { 0, NULL } },
/*TSFX_WITCH13*/ { sfx_STREAM,                "Sfx\\Towners\\Witch13.wav",   { 0, NULL } },
/*TSFX_WITCH14*/ { sfx_STREAM,                "Sfx\\Towners\\Witch14.wav",   { 0, NULL } },
/*TSFX_WITCH15*/ { sfx_STREAM,                "Sfx\\Towners\\Witch15.wav",   { 0, NULL } },
/*TSFX_WITCH16*///{ sfx_STREAM,                "Sfx\\Towners\\Witch16.wav",   { 0, NULL } },
/*TSFX_WITCH17*///{ sfx_STREAM,                "Sfx\\Towners\\Witch17.wav",   { 0, NULL } },
/*TSFX_WITCH18*/ { sfx_STREAM,                "Sfx\\Towners\\Witch18.wav",   { 0, NULL } },
/*TSFX_WITCH19*///{ sfx_STREAM,                "Sfx\\Towners\\Witch19.wav",   { 0, NULL } },
/*TSFX_WITCH20*/ { sfx_STREAM,                "Sfx\\Towners\\Witch20.wav",   { 0, NULL } },
/*TSFX_WITCH21*///{ sfx_STREAM,                "Sfx\\Towners\\Witch21.wav",   { 0, NULL } },
/*TSFX_WITCH22*/ { sfx_STREAM,                "Sfx\\Towners\\Witch22.wav",   { 0, NULL } },
/*TSFX_WITCH23*/ { sfx_STREAM,                "Sfx\\Towners\\Witch23.wav",   { 0, NULL } },
/*TSFX_WITCH24*/ { sfx_STREAM,                "Sfx\\Towners\\Witch24.wav",   { 0, NULL } },
/*TSFX_WITCH25*/ { sfx_STREAM,                "Sfx\\Towners\\Witch25.wav",   { 0, NULL } },
/*TSFX_WITCH26*/ { sfx_STREAM,                "Sfx\\Towners\\Witch26.wav",   { 0, NULL } },
/*TSFX_WITCH27*///{ sfx_STREAM,                "Sfx\\Towners\\Witch27.wav",   { 0, NULL } },
/*TSFX_WITCH28*///{ sfx_STREAM,                "Sfx\\Towners\\Witch28.wav",   { 0, NULL } },
/*TSFX_WITCH29*///{ sfx_STREAM,                "Sfx\\Towners\\Witch29.wav",   { 0, NULL } },
/*TSFX_WITCH30*///{ sfx_STREAM,                "Sfx\\Towners\\Witch30.wav",   { 0, NULL } },
/*TSFX_WITCH31*///{ sfx_STREAM,                "Sfx\\Towners\\Witch31.wav",   { 0, NULL } },
/*TSFX_WITCH32*///{ sfx_STREAM,                "Sfx\\Towners\\Witch32.wav",   { 0, NULL } },
/*TSFX_WITCH33*///{ sfx_STREAM,                "Sfx\\Towners\\Witch33.wav",   { 0, NULL } },
/*TSFX_WITCH34*///{ sfx_STREAM,                "Sfx\\Towners\\Witch34.wav",   { 0, NULL } },
/*TSFX_WITCH35*///{ sfx_STREAM,                "Sfx\\Towners\\Witch35.wav",   { 0, NULL } },
/*TSFX_WITCH36*///{ sfx_STREAM,                "Sfx\\Towners\\Witch36.wav",   { 0, NULL } },
/*TSFX_WITCH37*///{ sfx_STREAM,                "Sfx\\Towners\\Witch37.wav",   { 0, NULL } },
/*TSFX_WITCH38*/ { sfx_STREAM,                "Sfx\\Towners\\Witch38.wav",   { 0, NULL } },
/*TSFX_WITCH39*/ { sfx_STREAM,                "Sfx\\Towners\\Witch39.wav",   { 0, NULL } },
/*TSFX_WITCH40*/ { sfx_STREAM,                "Sfx\\Towners\\Witch40.wav",   { 0, NULL } },
/*TSFX_WITCH41*/ { sfx_STREAM,                "Sfx\\Towners\\Witch41.wav",   { 0, NULL } },
/*TSFX_WITCH42*/ { sfx_STREAM,                "Sfx\\Towners\\Witch42.wav",   { 0, NULL } },
/*TSFX_WITCH43*/ { sfx_STREAM,                "Sfx\\Towners\\Witch43.wav",   { 0, NULL } },
/*TSFX_WITCH44*/ { sfx_STREAM,                "Sfx\\Towners\\Witch44.wav",   { 0, NULL } },
/*TSFX_WITCH45*/ { sfx_STREAM,                "Sfx\\Towners\\Witch45.wav",   { 0, NULL } },
/*TSFX_WITCH46*/ { sfx_STREAM,                "Sfx\\Towners\\Witch46.wav",   { 0, NULL } },
/*TSFX_WITCH47*/ { sfx_STREAM,                "Sfx\\Towners\\Witch47.wav",   { 0, NULL } },
/*TSFX_WITCH48*///{ sfx_STREAM,                "Sfx\\Towners\\Witch48.wav",   { 0, NULL } },
/*TSFX_WITCH49*/ { sfx_STREAM,                "Sfx\\Towners\\Witch49.wav",   { 0, NULL } },
/*TSFX_WITCH50*/ { sfx_STREAM,                "Sfx\\Towners\\Witch50.wav",   { 0, NULL } },
/*TSFX_WOUND*/   { sfx_STREAM,                "Sfx\\Towners\\Wound01.wav",   { 0, NULL } },
/*PS_MAGE1*/     { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage01.wav",   { 0, NULL } },
/*PS_MAGE2*///   { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage02.wav",   { 0, NULL } },
/*PS_MAGE3*///   { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage03.wav",   { 0, NULL } },
/*PS_MAGE4*///   { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage04.wav",   { 0, NULL } },
/*PS_MAGE5*///   { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage05.wav",   { 0, NULL } },
/*PS_MAGE6*///   { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage06.wav",   { 0, NULL } },
/*PS_MAGE7*///   { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage07.wav",   { 0, NULL } },
/*PS_MAGE8*/     { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage08.wav",   { 0, NULL } },
/*PS_MAGE9*/     { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage09.wav",   { 0, NULL } },
/*PS_MAGE10*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage10.wav",   { 0, NULL } },
/*PS_MAGE11*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage11.wav",   { 0, NULL } },
/*PS_MAGE12*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage12.wav",   { 0, NULL } },
/*PS_MAGE13*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage13.wav",   { 0, NULL } }, // I can not use this yet.
/*PS_MAGE14*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage14.wav",   { 0, NULL } }, // I can not carry any more
/*PS_MAGE15*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage15.wav",   { 0, NULL } }, // I have no room.
/*PS_MAGE16*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage16.wav",   { 0, NULL } }, // Where would I put this?
/*PS_MAGE17*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage17.wav",   { 0, NULL } }, // No way.
/*PS_MAGE18*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage18.wav",   { 0, NULL } }, // Not a chance.
/*PS_MAGE19*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage19.wav",   { 0, NULL } },
/*PS_MAGE20*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage20.wav",   { 0, NULL } },
/*PS_MAGE21*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage21.wav",   { 0, NULL } },
/*PS_MAGE22*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage22.wav",   { 0, NULL } },
/*PS_MAGE23*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage23.wav",   { 0, NULL } },
/*PS_MAGE24*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage24.wav",   { 0, NULL } }, // I can not open this. Yet.
/*PS_MAGE25*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage25.wav",   { 0, NULL } },
/*PS_MAGE26*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage26.wav",   { 0, NULL } },
/*PS_MAGE27*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage27.wav",   { 0, NULL } }, // I can not cast that here.
/*PS_MAGE28*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage28.wav",   { 0, NULL } },
/*PS_MAGE29*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage29.wav",   { 0, NULL } },
/*PS_MAGE30*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage30.wav",   { 0, NULL } },
/*PS_MAGE31*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage31.wav",   { 0, NULL } },
/*PS_MAGE32*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage32.wav",   { 0, NULL } },
/*PS_MAGE33*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage33.wav",   { 0, NULL } },
/*PS_MAGE34*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage34.wav",   { 0, NULL } }, // I do not have a spell ready.
/*PS_MAGE35*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage35.wav",   { 0, NULL } }, // Not enough mana.
/*PS_MAGE36*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage36.wav",   { 0, NULL } },
/*PS_MAGE37*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage37.wav",   { 0, NULL } },
/*PS_MAGE38*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage38.wav",   { 0, NULL } },
/*PS_MAGE39*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage39.wav",   { 0, NULL } },
/*PS_MAGE40*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage40.wav",   { 0, NULL } },
/*PS_MAGE41*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage41.wav",   { 0, NULL } },
/*PS_MAGE42*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage42.wav",   { 0, NULL } },
/*PS_MAGE43*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage43.wav",   { 0, NULL } },
/*PS_MAGE44*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage44.wav",   { 0, NULL } },
/*PS_MAGE45*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage45.wav",   { 0, NULL } },
/*PS_MAGE46*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage46.wav",   { 0, NULL } },
/*PS_MAGE47*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage47.wav",   { 0, NULL } },
/*PS_MAGE48*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage48.wav",   { 0, NULL } },
/*PS_MAGE49*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage49.wav",   { 0, NULL } },
/*PS_MAGE50*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage50.wav",   { 0, NULL } },
/*PS_MAGE51*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage51.wav",   { 0, NULL } },
/*PS_MAGE52*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage52.wav",   { 0, NULL } },
/*PS_MAGE53*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage53.wav",   { 0, NULL } },
/*PS_MAGE54*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage54.wav",   { 0, NULL } },
/*PS_MAGE55*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage55.wav",   { 0, NULL } },
/*PS_MAGE56*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage56.wav",   { 0, NULL } },
/*PS_MAGE57*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage57.wav",   { 0, NULL } },
/*PS_MAGE58*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage58.wav",   { 0, NULL } },
/*PS_MAGE59*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage59.wav",   { 0, NULL } },
/*PS_MAGE60*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage60.wav",   { 0, NULL } },
/*PS_MAGE61*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage61.wav",   { 0, NULL } },
/*PS_MAGE62*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage62.wav",   { 0, NULL } },
/*PS_MAGE63*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage63.wav",   { 0, NULL } },
/*PS_MAGE64*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage64.wav",   { 0, NULL } },
/*PS_MAGE65*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage65.wav",   { 0, NULL } },
/*PS_MAGE66*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage66.wav",   { 0, NULL } },
/*PS_MAGE67*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage67.wav",   { 0, NULL } },
/*PS_MAGE68*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage68.wav",   { 0, NULL } },
/*PS_MAGE69*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage69.wav",   { 0, NULL } }, // Ouhm..
/*PS_MAGE69B*/   { sfx_SORCERER,              "Sfx\\Sorceror\\Mage69b.wav",  { 0, NULL } }, // Umm..
/*PS_MAGE70*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage70.wav",   { 0, NULL } }, // Argh...
/*PS_MAGE71*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage71.wav",   { 0, NULL } }, // Ouah.
/*PS_MAGE72*/    { sfx_SORCERER,              "Sfx\\Sorceror\\Mage72.wav",   { 0, NULL } }, // Huh ah..
/*PS_MAGE73*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage73.wav",   { 0, NULL } },
/*PS_MAGE74*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage74.wav",   { 0, NULL } },
/*PS_MAGE75*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage75.wav",   { 0, NULL } },
/*PS_MAGE76*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage76.wav",   { 0, NULL } },
/*PS_MAGE77*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage77.wav",   { 0, NULL } },
/*PS_MAGE78*///  { sfx_SORCERER,              "Sfx\\Sorceror\\Mage78.wav",   { 0, NULL } },
/*PS_MAGE79*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage79.wav",   { 0, NULL } },
/*PS_MAGE80*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage80.wav",   { 0, NULL } },
/*PS_MAGE81*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage81.wav",   { 0, NULL } },
/*PS_MAGE82*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage82.wav",   { 0, NULL } },
/*PS_MAGE83*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage83.wav",   { 0, NULL } },
/*PS_MAGE84*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage84.wav",   { 0, NULL } },
/*PS_MAGE85*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage85.wav",   { 0, NULL } },
/*PS_MAGE86*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage86.wav",   { 0, NULL } },
/*PS_MAGE87*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage87.wav",   { 0, NULL } },
/*PS_MAGE88*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage88.wav",   { 0, NULL } },
/*PS_MAGE89*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage89.wav",   { 0, NULL } },
/*PS_MAGE90*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage90.wav",   { 0, NULL } },
/*PS_MAGE91*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage91.wav",   { 0, NULL } },
/*PS_MAGE92*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage92.wav",   { 0, NULL } },
/*PS_MAGE93*///  { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage93.wav",   { 0, NULL } },
/*PS_MAGE94*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage94.wav",   { 0, NULL } },
/*PS_MAGE95*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage95.wav",   { 0, NULL } },
/*PS_MAGE96*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage96.wav",   { 0, NULL } },
/*PS_MAGE97*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage97.wav",   { 0, NULL } },
/*PS_MAGE98*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage98.wav",   { 0, NULL } },
/*PS_MAGE99*/    { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage99.wav",   { 0, NULL } },
/*PS_MAGE100*/// { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage100.wav",  { 0, NULL } },
/*PS_MAGE101*/// { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage101.wav",  { 0, NULL } },
/*PS_MAGE102*/// { sfx_STREAM | sfx_SORCERER, "Sfx\\Sorceror\\Mage102.wav",  { 0, NULL } },
/*PS_ROGUE1*/    { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue01.wav",     { 0, NULL } },
/*PS_ROGUE2*///  { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue02.wav",     { 0, NULL } },
/*PS_ROGUE3*///  { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue03.wav",     { 0, NULL } },
/*PS_ROGUE4*///  { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue04.wav",     { 0, NULL } },
/*PS_ROGUE5*///  { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue05.wav",     { 0, NULL } },
/*PS_ROGUE6*///  { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue06.wav",     { 0, NULL } },
/*PS_ROGUE7*///  { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue07.wav",     { 0, NULL } },
/*PS_ROGUE8*/    { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue08.wav",     { 0, NULL } },
/*PS_ROGUE9*/    { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue09.wav",     { 0, NULL } },
/*PS_ROGUE10*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue10.wav",     { 0, NULL } },
/*PS_ROGUE11*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue11.wav",     { 0, NULL } },
/*PS_ROGUE12*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue12.wav",     { 0, NULL } },
/*PS_ROGUE13*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue13.wav",     { 0, NULL } }, // I can't use this yet.
/*PS_ROGUE14*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue14.wav",     { 0, NULL } }, // I can't carry any more.
/*PS_ROGUE15*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue15.wav",     { 0, NULL } }, // I have no room.
/*PS_ROGUE16*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue16.wav",     { 0, NULL } }, // Now where would I put this?
/*PS_ROGUE17*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue17.wav",     { 0, NULL } }, // No way...
/*PS_ROGUE18*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue18.wav",     { 0, NULL } }, // Not a chance
/*PS_ROGUE19*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue19.wav",     { 0, NULL } },
/*PS_ROGUE20*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue20.wav",     { 0, NULL } },
/*PS_ROGUE21*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue21.wav",     { 0, NULL } },
/*PS_ROGUE22*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue22.wav",     { 0, NULL } },
/*PS_ROGUE23*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue23.wav",     { 0, NULL } },
/*PS_ROGUE24*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue24.wav",     { 0, NULL } }, // I can't open this. .. Yet.
/*PS_ROGUE25*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue25.wav",     { 0, NULL } },
/*PS_ROGUE26*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue26.wav",     { 0, NULL } },
/*PS_ROGUE27*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue27.wav",     { 0, NULL } }, // I can't cast that here.
/*PS_ROGUE28*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue28.wav",     { 0, NULL } },
/*PS_ROGUE29*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue29.wav",     { 0, NULL } }, // That did not do anything.
/*PS_ROGUE30*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue30.wav",     { 0, NULL } },
/*PS_ROGUE31*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue31.wav",     { 0, NULL } },
/*PS_ROGUE32*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue32.wav",     { 0, NULL } },
/*PS_ROGUE33*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue33.wav",     { 0, NULL } },
/*PS_ROGUE34*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue34.wav",     { 0, NULL } }, // I don't have a spell ready.
/*PS_ROGUE35*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue35.wav",     { 0, NULL } }, // Not enough mana.
/*PS_ROGUE36*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue36.wav",     { 0, NULL } },
/*PS_ROGUE37*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue37.wav",     { 0, NULL } },
/*PS_ROGUE38*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue38.wav",     { 0, NULL } },
/*PS_ROGUE39*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue39.wav",     { 0, NULL } },
/*PS_ROGUE40*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue40.wav",     { 0, NULL } },
/*PS_ROGUE41*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue41.wav",     { 0, NULL } },
/*PS_ROGUE42*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue42.wav",     { 0, NULL } },
/*PS_ROGUE43*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue43.wav",     { 0, NULL } },
/*PS_ROGUE44*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue44.wav",     { 0, NULL } },
/*PS_ROGUE45*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue45.wav",     { 0, NULL } },
/*PS_ROGUE46*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue46.wav",     { 0, NULL } }, // Just what I was looking for.
/*PS_ROGUE47*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue47.wav",     { 0, NULL } },
/*PS_ROGUE48*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue48.wav",     { 0, NULL } },
/*PS_ROGUE49*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue49.wav",     { 0, NULL } }, // I'm not thirsty.
/*PS_ROGUE50*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue50.wav",     { 0, NULL } }, // Hey, I'm no milkmaid.
/*PS_ROGUE51*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue51.wav",     { 0, NULL } },
/*PS_ROGUE52*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue52.wav",     { 0, NULL } }, // Yep, that's a cow, alright.
/*PS_ROGUE53*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue53.wav",     { 0, NULL } },
/*PS_ROGUE54*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue54.wav",     { 0, NULL } },
/*PS_ROGUE55*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue55.wav",     { 0, NULL } },
/*PS_ROGUE56*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue56.wav",     { 0, NULL } },
/*PS_ROGUE57*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue57.wav",     { 0, NULL } },
/*PS_ROGUE58*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue58.wav",     { 0, NULL } },
/*PS_ROGUE59*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue59.wav",     { 0, NULL } },
/*PS_ROGUE60*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue60.wav",     { 0, NULL } },
/*PS_ROGUE61*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue61.wav",     { 0, NULL } },
/*PS_ROGUE62*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue62.wav",     { 0, NULL } },
/*PS_ROGUE63*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue63.wav",     { 0, NULL } },
/*PS_ROGUE64*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue64.wav",     { 0, NULL } },
/*PS_ROGUE65*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue65.wav",     { 0, NULL } },
/*PS_ROGUE66*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue66.wav",     { 0, NULL } },
/*PS_ROGUE67*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue67.wav",     { 0, NULL } },
/*PS_ROGUE68*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue68.wav",     { 0, NULL } },
/*PS_ROGUE69*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue69.wav",     { 0, NULL } }, // Aeh...
/*PS_ROGUE69B*/  { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue69b.wav",    { 0, NULL } }, // Oah...
/*PS_ROGUE70*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue70.wav",     { 0, NULL } }, // Ouhuh..
/*PS_ROGUE71*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue71.wav",     { 0, NULL } }, // Aaaaauh.
/*PS_ROGUE72*/   { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue72.wav",     { 0, NULL } }, // Huuhuhh.
/*PS_ROGUE73*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue73.wav",     { 0, NULL } },
/*PS_ROGUE74*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue74.wav",     { 0, NULL } },
/*PS_ROGUE75*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue75.wav",     { 0, NULL } },
/*PS_ROGUE76*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue76.wav",     { 0, NULL } },
/*PS_ROGUE77*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue77.wav",     { 0, NULL } },
/*PS_ROGUE78*/// { sfx_ROGUE,                 "Sfx\\Rogue\\Rogue78.wav",     { 0, NULL } },
/*PS_ROGUE79*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue79.wav",     { 0, NULL } },
/*PS_ROGUE80*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue80.wav",     { 0, NULL } },
/*PS_ROGUE81*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue81.wav",     { 0, NULL } },
/*PS_ROGUE82*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue82.wav",     { 0, NULL } },
/*PS_ROGUE83*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue83.wav",     { 0, NULL } },
/*PS_ROGUE84*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue84.wav",     { 0, NULL } },
/*PS_ROGUE85*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue85.wav",     { 0, NULL } },
/*PS_ROGUE86*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue86.wav",     { 0, NULL } },
/*PS_ROGUE87*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue87.wav",     { 0, NULL } },
/*PS_ROGUE88*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue88.wav",     { 0, NULL } },
/*PS_ROGUE89*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue89.wav",     { 0, NULL } },
/*PS_ROGUE90*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue90.wav",     { 0, NULL } },
/*PS_ROGUE91*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue91.wav",     { 0, NULL } },
/*PS_ROGUE92*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue92.wav",     { 0, NULL } },
/*PS_ROGUE93*/// { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue93.wav",     { 0, NULL } },
/*PS_ROGUE94*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue94.wav",     { 0, NULL } },
/*PS_ROGUE95*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue95.wav",     { 0, NULL } },
/*PS_ROGUE96*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue96.wav",     { 0, NULL } },
/*PS_ROGUE97*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue97.wav",     { 0, NULL } },
/*PS_ROGUE98*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue98.wav",     { 0, NULL } },
/*PS_ROGUE99*/   { sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue99.wav",     { 0, NULL } },
/*PS_ROGUE100*///{ sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue100.wav",    { 0, NULL } },
/*PS_ROGUE101*///{ sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue101.wav",    { 0, NULL } },
/*PS_ROGUE102*///{ sfx_STREAM | sfx_ROGUE,    "Sfx\\Rogue\\Rogue102.wav",    { 0, NULL } },
/*PS_WARR1*/     { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior01.wav",  { 0, NULL } },
/*PS_WARR2*///   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior02.wav",  { 0, NULL } },
/*PS_WARR3*///   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior03.wav",  { 0, NULL } },
/*PS_WARR4*///   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior04.wav",  { 0, NULL } },
/*PS_WARR5*///   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior05.wav",  { 0, NULL } },
/*PS_WARR6*///   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior06.wav",  { 0, NULL } },
/*PS_WARR7*///   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior07.wav",  { 0, NULL } },
/*PS_WARR8*/     { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior08.wav",  { 0, NULL } },
/*PS_WARR9*/     { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior09.wav",  { 0, NULL } },
/*PS_WARR10*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior10.wav",  { 0, NULL } },
/*PS_WARR11*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior11.wav",  { 0, NULL } },
/*PS_WARR12*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior12.wav",  { 0, NULL } },
/*PS_WARR13*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior13.wav",  { 0, NULL } }, // I can't use this. .. Yet.
/*PS_WARR14*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior14.wav",  { 0, NULL } }, // I can't carry any more.
/*PS_WARR14B*/   { sfx_WARRIOR,               "Sfx\\Warrior\\Wario14b.wav",  { 0, NULL } }, // I've got to pawn some of this stuff.
/*PS_WARR14C*/   { sfx_WARRIOR,               "Sfx\\Warrior\\Wario14c.wav",  { 0, NULL } }, // Too much baggage.
/*PS_WARR15*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior15.wav",  { 0, NULL } },
/*PS_WARR15B*/// { sfx_WARRIOR,               "Sfx\\Warrior\\Wario15b.wav",  { 0, NULL } },
/*PS_WARR15C*/// { sfx_WARRIOR,               "Sfx\\Warrior\\Wario15c.wav",  { 0, NULL } },
/*PS_WARR16*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior16.wav",  { 0, NULL } }, // Where would I put this?
/*PS_WARR16B*/// { sfx_WARRIOR,               "Sfx\\Warrior\\Wario16b.wav",  { 0, NULL } }, // Where you want me to put this?
/*PS_WARR16C*/// { sfx_WARRIOR,               "Sfx\\Warrior\\Wario16c.wav",  { 0, NULL } }, // What am I a pack rat?
/*PS_WARR17*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior17.wav",  { 0, NULL } },
/*PS_WARR18*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior18.wav",  { 0, NULL } },
/*PS_WARR19*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior19.wav",  { 0, NULL } },
/*PS_WARR20*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior20.wav",  { 0, NULL } },
/*PS_WARR21*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior21.wav",  { 0, NULL } },
/*PS_WARR22*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior22.wav",  { 0, NULL } },
/*PS_WARR23*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior23.wav",  { 0, NULL } },
/*PS_WARR24*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior24.wav",  { 0, NULL } }, // I can not open this. Yet.
/*PS_WARR25*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior25.wav",  { 0, NULL } },
/*PS_WARR26*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior26.wav",  { 0, NULL } },
/*PS_WARR27*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior27.wav",  { 0, NULL } }, // I can't cast that here.
/*PS_WARR28*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior28.wav",  { 0, NULL } },
/*PS_WARR29*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior29.wav",  { 0, NULL } },
/*PS_WARR30*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior30.wav",  { 0, NULL } },
/*PS_WARR31*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior31.wav",  { 0, NULL } },
/*PS_WARR32*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior32.wav",  { 0, NULL } },
/*PS_WARR33*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior33.wav",  { 0, NULL } },
/*PS_WARR34*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior34.wav",  { 0, NULL } }, // I don't have a spell ready.
/*PS_WARR35*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior35.wav",  { 0, NULL } }, // Not enough mana.
/*PS_WARR36*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior36.wav",  { 0, NULL } },
/*PS_WARR37*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior37.wav",  { 0, NULL } },
/*PS_WARR38*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior38.wav",  { 0, NULL } },
/*PS_WARR39*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior39.wav",  { 0, NULL } },
/*PS_WARR40*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior40.wav",  { 0, NULL } },
/*PS_WARR41*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior41.wav",  { 0, NULL } },
/*PS_WARR42*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior42.wav",  { 0, NULL } },
/*PS_WARR43*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior43.wav",  { 0, NULL } },
/*PS_WARR44*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior44.wav",  { 0, NULL } },
/*PS_WARR45*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior45.wav",  { 0, NULL } },
/*PS_WARR46*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior46.wav",  { 0, NULL } },
/*PS_WARR47*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior47.wav",  { 0, NULL } },
/*PS_WARR48*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior48.wav",  { 0, NULL } },
/*PS_WARR49*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior49.wav",  { 0, NULL } },
/*PS_WARR50*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior50.wav",  { 0, NULL } },
/*PS_WARR51*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior51.wav",  { 0, NULL } },
/*PS_WARR52*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior52.wav",  { 0, NULL } },
/*PS_WARR53*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior53.wav",  { 0, NULL } },
/*PS_WARR54*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior54.wav",  { 0, NULL } },
/*PS_WARR55*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior55.wav",  { 0, NULL } },
/*PS_WARR56*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior56.wav",  { 0, NULL } },
/*PS_WARR57*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior57.wav",  { 0, NULL } },
/*PS_WARR58*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior58.wav",  { 0, NULL } },
/*PS_WARR59*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior59.wav",  { 0, NULL } },
/*PS_WARR60*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior60.wav",  { 0, NULL } },
/*PS_WARR61*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior61.wav",  { 0, NULL } },
/*PS_WARR62*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior62.wav",  { 0, NULL } },
/*PS_WARR63*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior63.wav",  { 0, NULL } },
/*PS_WARR64*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior64.wav",  { 0, NULL } },
/*PS_WARR65*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior65.wav",  { 0, NULL } },
/*PS_WARR66*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior66.wav",  { 0, NULL } },
/*PS_WARR67*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior67.wav",  { 0, NULL } },
/*PS_WARR68*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior68.wav",  { 0, NULL } },
/*PS_WARR69*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior69.wav",  { 0, NULL } }, // Ahh..
/*PS_WARR69B*/   { sfx_WARRIOR,               "Sfx\\Warrior\\Wario69b.wav",  { 0, NULL } }, // Ouh...
/*PS_WARR70*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior70.wav",  { 0, NULL } }, // Ouah..
/*PS_WARR71*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior71.wav",  { 0, NULL } }, // Auuahh..
/*PS_WARR72*/    { sfx_WARRIOR,               "Sfx\\Warrior\\Warior72.wav",  { 0, NULL } }, // Huhhuhh.
/*PS_WARR73*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior73.wav",  { 0, NULL } },
/*PS_WARR74*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior74.wav",  { 0, NULL } },
/*PS_WARR75*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior75.wav",  { 0, NULL } },
/*PS_WARR76*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior76.wav",  { 0, NULL } },
/*PS_WARR77*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior77.wav",  { 0, NULL } },
/*PS_WARR78*///  { sfx_WARRIOR,               "Sfx\\Warrior\\Warior78.wav",  { 0, NULL } },
/*PS_WARR79*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior79.wav",  { 0, NULL } },
/*PS_WARR80*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior80.wav",  { 0, NULL } },
/*PS_WARR81*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior81.wav",  { 0, NULL } },
/*PS_WARR82*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior82.wav",  { 0, NULL } },
/*PS_WARR83*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior83.wav",  { 0, NULL } },
/*PS_WARR84*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior84.wav",  { 0, NULL } },
/*PS_WARR85*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior85.wav",  { 0, NULL } },
/*PS_WARR86*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior86.wav",  { 0, NULL } },
/*PS_WARR87*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior87.wav",  { 0, NULL } },
/*PS_WARR88*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior88.wav",  { 0, NULL } },
/*PS_WARR89*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior89.wav",  { 0, NULL } },
/*PS_WARR90*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior90.wav",  { 0, NULL } },
/*PS_WARR91*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior91.wav",  { 0, NULL } },
/*PS_WARR92*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior92.wav",  { 0, NULL } },
/*PS_WARR93*///  { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior93.wav",  { 0, NULL } },
/*PS_WARR94*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior94.wav",  { 0, NULL } },
/*PS_WARR95*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior95.wav",  { 0, NULL } },
/*PS_WARR95B*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario95b.wav",  { 0, NULL } },
/*PS_WARR95C*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario95c.wav",  { 0, NULL } },
/*PS_WARR95D*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario95d.wav",  { 0, NULL } },
/*PS_WARR95E*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario95e.wav",  { 0, NULL } },
/*PS_WARR95F*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario95f.wav",  { 0, NULL } },
/*PS_WARR96B*/   { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario96b.wav",  { 0, NULL } },
/*PS_WARR97*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario97.wav",   { 0, NULL } },
/*PS_WARR98*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario98.wav",   { 0, NULL } },
/*PS_WARR99*/    { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Warior99.wav",  { 0, NULL } },
/*PS_WARR100*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario100.wav",  { 0, NULL } },
/*PS_WARR101*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario101.wav",  { 0, NULL } },
/*PS_WARR102*/// { sfx_STREAM | sfx_WARRIOR,  "Sfx\\Warrior\\Wario102.wav",  { 0, NULL } },
/*PS_MONK1*/     { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk01.wav",       { 0, NULL } },
/*PS_MONK2*///   { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK3*///   { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK4*///   { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK5*///   { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK6*///   { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK7*///   { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK8*/     { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk08.wav",       { 0, NULL } },
/*PS_MONK9*/     { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk09.wav",       { 0, NULL } },
/*PS_MONK10*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk10.wav",       { 0, NULL } },
/*PS_MONK11*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk11.wav",       { 0, NULL } },
/*PS_MONK12*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk12.wav",       { 0, NULL } },
/*PS_MONK13*/    { sfx_MONK,                  "Sfx\\Monk\\Monk13.wav",       { 0, NULL } }, // I can not use this. yet.
/*PS_MONK14*/    { sfx_MONK,                  "Sfx\\Monk\\Monk14.wav",       { 0, NULL } }, // I can not carry any more.
/*PS_MONK15*/    { sfx_MONK,                  "Sfx\\Monk\\Monk15.wav",       { 0, NULL } }, // I have no room.
/*PS_MONK16*/    { sfx_MONK,                  "Sfx\\Monk\\Monk16.wav",       { 0, NULL } }, // Where would I put this?
/*PS_MONK17*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK18*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK19*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK20*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK21*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK22*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK23*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK24*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk24.wav",       { 0, NULL } }, // I can not open this. .. Yet.
/*PS_MONK25*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK26*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK27*/    { sfx_MONK,                  "Sfx\\Monk\\Monk27.wav",       { 0, NULL } }, // I can not cast that here.
/*PS_MONK28*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK29*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk29.wav",       { 0, NULL } },
/*PS_MONK30*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK31*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK32*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK33*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK34*/    { sfx_MONK,                  "Sfx\\Monk\\Monk34.wav",       { 0, NULL } }, // I do not have a spell ready.
/*PS_MONK35*/    { sfx_MONK,                  "Sfx\\Monk\\Monk35.wav",       { 0, NULL } }, // Not enough mana.
/*PS_MONK36*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK37*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK38*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK39*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK40*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK41*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK42*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK43*///  { sfx_MONK,                  "Sfx\\Monk\\Monk43.wav",       { 0, NULL } },
/*PS_MONK44*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK45*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK46*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk46.wav",       { 0, NULL } },
/*PS_MONK47*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK48*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK49*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk49.wav",       { 0, NULL } },
/*PS_MONK50*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk50.wav",       { 0, NULL } },
/*PS_MONK51*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK52*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk52.wav",       { 0, NULL } },
/*PS_MONK53*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK54*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk54.wav",       { 0, NULL } },
/*PS_MONK55*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk55.wav",       { 0, NULL } },
/*PS_MONK56*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk56.wav",       { 0, NULL } },
/*PS_MONK57*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK58*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK59*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK60*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK61*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk61.wav",       { 0, NULL } },
/*PS_MONK62*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk62.wav",       { 0, NULL } },
/*PS_MONK63*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK64*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK65*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK66*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK67*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK68*/    { sfx_MONK,                  "Sfx\\Monk\\Monk68.wav",       { 0, NULL } },
/*PS_MONK69*/    { sfx_MONK,                  "Sfx\\Monk\\Monk69.wav",       { 0, NULL } }, // Umm..
/*PS_MONK69B*/   { sfx_MONK,                  "Sfx\\Monk\\Monk69b.wav",      { 0, NULL } }, // Ouch..
/*PS_MONK70*/    { sfx_MONK,                  "Sfx\\Monk\\Monk70.wav",       { 0, NULL } }, // Oahhahh.
/*PS_MONK71*/    { sfx_MONK,                  "Sfx\\Monk\\Monk71.wav",       { 0, NULL } }, // Oaah ah.
/*PS_MONK72*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK73*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK74*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK75*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK76*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK77*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK78*///  { sfx_MONK,                  "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK79*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk79.wav",       { 0, NULL } },
/*PS_MONK80*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk80.wav",       { 0, NULL } },
/*PS_MONK81*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK82*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk82.wav",       { 0, NULL } },
/*PS_MONK83*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk83.wav",       { 0, NULL } },
/*PS_MONK84*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK85*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK86*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK87*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk87.wav",       { 0, NULL } },
/*PS_MONK88*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk88.wav",       { 0, NULL } },
/*PS_MONK89*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk89.wav",       { 0, NULL } },
/*PS_MONK90*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK91*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk91.wav",       { 0, NULL } },
/*PS_MONK92*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk92.wav",       { 0, NULL } },
/*PS_MONK93*///  { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK94*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk94.wav",       { 0, NULL } },
/*PS_MONK95*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk95.wav",       { 0, NULL } },
/*PS_MONK96*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk96.wav",       { 0, NULL } },
/*PS_MONK97*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk97.wav",       { 0, NULL } },
/*PS_MONK98*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk98.wav",       { 0, NULL } },
/*PS_MONK99*/    { sfx_STREAM | sfx_MONK,     "Sfx\\Monk\\Monk99.wav",       { 0, NULL } },
/*PS_MONK100*/// { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK101*/// { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_MONK102*/// { sfx_STREAM | sfx_MONK,     "Sfx\\Misc\\blank.wav",        { 0, NULL } },
/*PS_NAR1*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar01.wav",    { 0, NULL } },
/*PS_NAR2*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar02.wav",    { 0, NULL } },
/*PS_NAR3*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar03.wav",    { 0, NULL } },
/*PS_NAR4*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar04.wav",    { 0, NULL } },
/*PS_NAR5*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar05.wav",    { 0, NULL } },
/*PS_NAR6*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar06.wav",    { 0, NULL } },
/*PS_NAR7*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar07.wav",    { 0, NULL } },
/*PS_NAR8*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar08.wav",    { 0, NULL } },
/*PS_NAR9*/      { sfx_STREAM,                "Sfx\\Narrator\\Nar09.wav",    { 0, NULL } },
/*PS_DIABLVLINT*/{ sfx_STREAM,                "Sfx\\Misc\\Lvl16int.wav",     { 0, NULL } },
/*USFX_CLEAVER*/ { sfx_STREAM,                "Sfx\\Monsters\\Butcher.wav",  { 0, NULL } },
/*USFX_GARBUD1*/ { sfx_STREAM,                "Sfx\\Monsters\\Garbud01.wav", { 0, NULL } },
/*USFX_GARBUD2*/ { sfx_STREAM,                "Sfx\\Monsters\\Garbud02.wav", { 0, NULL } },
/*USFX_GARBUD3*/ { sfx_STREAM,                "Sfx\\Monsters\\Garbud03.wav", { 0, NULL } },
/*USFX_GARBUD4*/ { sfx_STREAM,                "Sfx\\Monsters\\Garbud04.wav", { 0, NULL } },
/*USFX_IZUAL1*///{ sfx_STREAM,                "Sfx\\Monsters\\Izual01.wav",  { 0, NULL } },
/*USFX_LACH1*/   { sfx_STREAM,                "Sfx\\Monsters\\Lach01.wav",   { 0, NULL } },
/*USFX_LACH2*/   { sfx_STREAM,                "Sfx\\Monsters\\Lach02.wav",   { 0, NULL } },
/*USFX_LACH3*/   { sfx_STREAM,                "Sfx\\Monsters\\Lach03.wav",   { 0, NULL } },
/*USFX_LAZ1*/    { sfx_STREAM,                "Sfx\\Monsters\\Laz01.wav",    { 0, NULL } },
/*USFX_LAZ2*/    { sfx_STREAM,                "Sfx\\Monsters\\Laz02.wav",    { 0, NULL } },
/*USFX_SKING1*/  { sfx_STREAM,                "Sfx\\Monsters\\Sking01.wav",  { 0, NULL } },
/*USFX_SNOT1*/   { sfx_STREAM,                "Sfx\\Monsters\\Snot01.wav",   { 0, NULL } },
/*USFX_SNOT2*/   { sfx_STREAM,                "Sfx\\Monsters\\Snot02.wav",   { 0, NULL } },
/*USFX_SNOT3*/   { sfx_STREAM,                "Sfx\\Monsters\\Snot03.wav",   { 0, NULL } },
/*USFX_WARLRD1*/ { sfx_STREAM,                "Sfx\\Monsters\\Warlrd01.wav", { 0, NULL } },
/*USFX_WLOCK1*/  { sfx_STREAM,                "Sfx\\Monsters\\Wlock01.wav",  { 0, NULL } },
/*USFX_ZHAR1*/   { sfx_STREAM,                "Sfx\\Monsters\\Zhar01.wav",   { 0, NULL } },
/*USFX_ZHAR2*/   { sfx_STREAM,                "Sfx\\Monsters\\Zhar02.wav",   { 0, NULL } },
/*USFX_DIABLOD*/ { sfx_STREAM,                "Sfx\\Monsters\\DiabloD.wav",  { 0, NULL } },
/*TSFX_FARMER1*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer1.wav",  { 0, NULL } },
/*TSFX_FARMER2*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer2.wav",  { 0, NULL } },
/*TSFX_FARMER2A*/{ sfx_STREAM,                "Sfx\\Hellfire\\Farmer2A.wav", { 0, NULL } },
/*TSFX_FARMER3*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer3.wav",  { 0, NULL } },
/*TSFX_FARMER4*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer4.wav",  { 0, NULL } },
/*TSFX_FARMER5*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer5.wav",  { 0, NULL } },
/*TSFX_FARMER6*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer6.wav",  { 0, NULL } },
/*TSFX_FARMER7*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer7.wav",  { 0, NULL } },
/*TSFX_FARMER8*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer8.wav",  { 0, NULL } },
/*TSFX_FARMER9*/ { sfx_STREAM,                "Sfx\\Hellfire\\Farmer9.wav",  { 0, NULL } },
/*TSFX_TEDDYBR1*/{ sfx_STREAM,                "Sfx\\Hellfire\\TEDDYBR1.wav", { 0, NULL } },
/*TSFX_TEDDYBR2*/{ sfx_STREAM,                "Sfx\\Hellfire\\TEDDYBR2.wav", { 0, NULL } },
/*TSFX_TEDDYBR3*/{ sfx_STREAM,                "Sfx\\Hellfire\\TEDDYBR3.wav", { 0, NULL } },
/*TSFX_TEDDYBR4*/{ sfx_STREAM,                "Sfx\\Hellfire\\TEDDYBR4.wav", { 0, NULL } },
/*USFX_DEFILER1*///{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER1.wav", { 0, NULL } },
/*USFX_DEFILER2*///{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER2.wav", { 0, NULL } },
/*USFX_DEFILER3*///{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER3.wav", { 0, NULL } },
/*USFX_DEFILER4*///{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER4.wav", { 0, NULL } },
/*USFX_DEFILER8*/{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER8.wav", { 0, NULL } },
/*USFX_DEFILER6*/{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER6.wav", { 0, NULL } },
/*USFX_DEFILER7*/{ sfx_STREAM,                "Sfx\\Hellfire\\DEFILER7.wav", { 0, NULL } },
/*USFX_NAKRUL1*///{ sfx_STREAM,                "Sfx\\Hellfire\\NAKRUL1.wav",  { 0, NULL } },
/*USFX_NAKRUL2*///{ sfx_STREAM,                "Sfx\\Hellfire\\NAKRUL2.wav",  { 0, NULL } },
/*USFX_NAKRUL3*///{ sfx_STREAM,                "Sfx\\Hellfire\\NAKRUL3.wav",  { 0, NULL } },
/*USFX_NAKRUL4*/ { sfx_STREAM,                "Sfx\\Hellfire\\NAKRUL4.wav",  { 0, NULL } },
/*USFX_NAKRUL5*/ { sfx_STREAM,                "Sfx\\Hellfire\\NAKRUL5.wav",  { 0, NULL } },
/*USFX_NAKRUL6*/ { sfx_STREAM,                "Sfx\\Hellfire\\NAKRUL6.wav",  { 0, NULL } },
/*PS_NARATR3*/// { sfx_STREAM,                "Sfx\\Hellfire\\NARATR3.wav",  { 0, NULL } },
/*TSFX_COWSUT1*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT1.wav",  { 0, NULL } },
/*TSFX_COWSUT2*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT2.wav",  { 0, NULL } },
/*TSFX_COWSUT3*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT3.wav",  { 0, NULL } },
/*TSFX_COWSUT4*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT4.wav",  { 0, NULL } },
/*TSFX_COWSUT4A*/{ sfx_STREAM,                "Sfx\\Hellfire\\COWSUT4A.wav", { 0, NULL } },
/*TSFX_COWSUT5*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT5.wav",  { 0, NULL } },
/*TSFX_COWSUT6*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT6.wav",  { 0, NULL } },
/*TSFX_COWSUT7*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT7.wav",  { 0, NULL } },
/*TSFX_COWSUT8*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT8.wav",  { 0, NULL } },
/*TSFX_COWSUT9*/ { sfx_STREAM,                "Sfx\\Hellfire\\COWSUT9.wav",  { 0, NULL } },
/*TSFX_COWSUT10*/{ sfx_STREAM,                "Sfx\\Hellfire\\COWSUT10.wav", { 0, NULL } },
/*TSFX_COWSUT11*/{ sfx_STREAM,                "Sfx\\Hellfire\\COWSUT11.wav", { 0, NULL } },
/*TSFX_COWSUT12*/{ sfx_STREAM,                "Sfx\\Hellfire\\COWSUT12.wav", { 0, NULL } },
/*USFX_SKLJRN1*///{ sfx_STREAM,                "Sfx\\Hellfire\\Skljrn1.wav",  { 0, NULL } },
/*PS_NARATR6*/   { sfx_STREAM,                "Sfx\\Hellfire\\Naratr6.wav",  { 0, NULL } },
/*PS_NARATR7*/   { sfx_STREAM,                "Sfx\\Hellfire\\Naratr7.wav",  { 0, NULL } },
/*PS_NARATR8*/   { sfx_STREAM,                "Sfx\\Hellfire\\Naratr8.wav",  { 0, NULL } },
/*PS_NARATR5*/   { sfx_STREAM,                "Sfx\\Hellfire\\Naratr5.wav",  { 0, NULL } },
/*PS_NARATR9*/   { sfx_STREAM,                "Sfx\\Hellfire\\Naratr9.wav",  { 0, NULL } },
/*PS_NARATR4*/   { sfx_STREAM,                "Sfx\\Hellfire\\Naratr4.wav",  { 0, NULL } },
/*TSFX_TRADER1*///{ sfx_STREAM,                "Sfx\\Hellfire\\TRADER1.wav",  { 0, NULL } },
	// clang-format on
};

const int sgSFXSets[NUM_SFXSets][NUM_CLASSES] {
	// clang-format off
#ifdef HELLFIRE
	{ sfx_WARRIOR, sfx_ROGUE, sfx_SORCERER, sfx_MONK, sfx_ROGUE, sfx_WARRIOR },
	{ PS_WARR1,  PS_ROGUE1,  PS_MAGE1,  PS_MONK1,  PS_ROGUE1,  PS_WARR1  },
	{ PS_WARR8,  PS_ROGUE8,  PS_MAGE8,  PS_MONK8,  PS_ROGUE8,  PS_WARR8  },
	{ PS_WARR9,  PS_ROGUE9,  PS_MAGE9,  PS_MONK9,  PS_ROGUE9,  PS_WARR9  },
	{ PS_WARR10, PS_ROGUE10, PS_MAGE10, PS_MONK10, PS_ROGUE10, PS_WARR10 },
	{ PS_WARR11, PS_ROGUE11, PS_MAGE11, PS_MONK11, PS_ROGUE11, PS_WARR11 },
	{ PS_WARR12, PS_ROGUE12, PS_MAGE12, PS_MONK12, PS_ROGUE12, PS_WARR12 },
	{ PS_WARR13, PS_ROGUE13, PS_MAGE13, PS_MONK13, PS_ROGUE13, PS_WARR13 },
	{ PS_WARR14, PS_ROGUE14, PS_MAGE14, PS_MONK14, PS_ROGUE14, PS_WARR14 },
	//{ PS_WARR16, PS_ROGUE16, PS_MAGE16, PS_MONK16, PS_ROGUE16, PS_WARR16 },
	{ PS_WARR24, PS_ROGUE24, PS_MAGE24, PS_MONK24, PS_ROGUE24, PS_WARR24 },
	{ PS_WARR27, PS_ROGUE27, PS_MAGE27, PS_MONK27, PS_ROGUE27, PS_WARR27 },
	{ PS_WARR29, PS_ROGUE29, PS_MAGE29, PS_MONK29, PS_ROGUE29, PS_WARR29 },
	{ PS_WARR34, PS_ROGUE34, PS_MAGE34, PS_MONK34, PS_ROGUE34, PS_WARR34 },
	{ PS_WARR35, PS_ROGUE35, PS_MAGE35, PS_MONK35, PS_ROGUE35, PS_WARR35 },
	{ PS_WARR46, PS_ROGUE46, PS_MAGE46, PS_MONK46, PS_ROGUE46, PS_WARR46 },
	{ PS_WARR54, PS_ROGUE54, PS_MAGE54, PS_MONK54, PS_ROGUE54, PS_WARR54 },
	{ PS_WARR55, PS_ROGUE55, PS_MAGE55, PS_MONK55, PS_ROGUE55, PS_WARR55 },
	{ PS_WARR56, PS_ROGUE56, PS_MAGE56, PS_MONK56, PS_ROGUE56, PS_WARR56 },
	{ PS_WARR61, PS_ROGUE61, PS_MAGE61, PS_MONK61, PS_ROGUE61, PS_WARR61 },
	{ PS_WARR62, PS_ROGUE62, PS_MAGE62, PS_MONK62, PS_ROGUE62, PS_WARR62 },
	{ PS_WARR68, PS_ROGUE68, PS_MAGE68, PS_MONK68, PS_ROGUE68, PS_WARR68 },
	{ PS_WARR69, PS_ROGUE69, PS_MAGE69, PS_MONK69, PS_ROGUE69, PS_WARR69 },
	{ PS_WARR70, PS_ROGUE70, PS_MAGE70, PS_MONK70, PS_ROGUE70, PS_WARR70 },
	{ PS_DEAD,   PS_ROGUE71, PS_MAGE71, PS_MONK71, PS_ROGUE71, PS_WARR71 }, // BUGFIX: should use `PS_WARR71` like other classes
	{ PS_WARR72, PS_ROGUE72, PS_MAGE72, PS_MAGE72, PS_ROGUE72, PS_WARR72 }, // BUGFIX: should be PS_MONK72, but it is blank...
	{ PS_WARR79, PS_ROGUE79, PS_MAGE79, PS_MONK79, PS_ROGUE79, PS_WARR79 },
	{ PS_WARR80, PS_ROGUE80, PS_MAGE80, PS_MONK80, PS_ROGUE80, PS_WARR80 },
	{ PS_WARR82, PS_ROGUE82, PS_MAGE82, PS_MONK82, PS_ROGUE82, PS_WARR82 },
	{ PS_WARR83, PS_ROGUE83, PS_MAGE83, PS_MONK83, PS_ROGUE83, PS_WARR83 },
	{ PS_WARR87, PS_ROGUE87, PS_MAGE87, PS_MONK87, PS_ROGUE87, PS_WARR87 },
	{ PS_WARR88, PS_ROGUE88, PS_MAGE88, PS_MONK88, PS_ROGUE88, PS_WARR88 },
	{ PS_WARR89, PS_ROGUE89, PS_MAGE89, PS_MONK89, PS_ROGUE89, PS_WARR89 },
	{ PS_WARR91, PS_ROGUE91, PS_MAGE91, PS_MONK91, PS_ROGUE91, PS_WARR91 },
	{ PS_WARR92, PS_ROGUE92, PS_MAGE92, PS_MONK92, PS_ROGUE92, PS_WARR92 },
	{ PS_WARR94, PS_ROGUE94, PS_MAGE94, PS_MONK94, PS_ROGUE94, PS_WARR94 },
	{ PS_WARR95, PS_ROGUE95, PS_MAGE95, PS_MONK95, PS_ROGUE95, PS_WARR95 },
	{ PS_WARR96B,PS_ROGUE96, PS_MAGE96, PS_MONK96, PS_ROGUE96, PS_WARR96B},
	{ PS_WARR97, PS_ROGUE97, PS_MAGE97, PS_MONK97, PS_ROGUE97, PS_WARR97 },
	{ PS_WARR98, PS_ROGUE98, PS_MAGE98, PS_MONK98, PS_ROGUE98, PS_WARR98 },
	{ PS_WARR99, PS_ROGUE99, PS_MAGE99, PS_MONK99, PS_ROGUE99, PS_WARR99 },
#else
	{ sfx_WARRIOR, sfx_ROGUE, sfx_SORCERER },
	{ PS_WARR1,  PS_ROGUE1,  PS_MAGE1  },
	{ PS_WARR8,  PS_ROGUE8,  PS_MAGE8  },
	{ PS_WARR9,  PS_ROGUE9,  PS_MAGE9  },
	{ PS_WARR10, PS_ROGUE10, PS_MAGE10 },
	{ PS_WARR11, PS_ROGUE11, PS_MAGE11 },
	{ PS_WARR12, PS_ROGUE12, PS_MAGE12 },
	{ PS_WARR13, PS_ROGUE13, PS_MAGE13 },
	{ PS_WARR14, PS_ROGUE14, PS_MAGE14 },
	//{ PS_WARR16, PS_ROGUE16, PS_MAGE16 },
	{ PS_WARR24, PS_ROGUE24, PS_MAGE24 },
	{ PS_WARR27, PS_ROGUE27, PS_MAGE27 },
	{ PS_WARR29, PS_ROGUE29, PS_MAGE29 },
	{ PS_WARR34, PS_ROGUE34, PS_MAGE34 },
	{ PS_WARR35, PS_ROGUE35, PS_MAGE35 },
	{ PS_WARR46, PS_ROGUE46, PS_MAGE46 },
	{ PS_WARR54, PS_ROGUE54, PS_MAGE54 },
	{ PS_WARR55, PS_ROGUE55, PS_MAGE55 },
	{ PS_WARR56, PS_ROGUE56, PS_MAGE56 },
	{ PS_WARR61, PS_ROGUE61, PS_MAGE61 },
	{ PS_WARR62, PS_ROGUE62, PS_MAGE62 },
	{ PS_WARR68, PS_ROGUE68, PS_MAGE68 },
	{ PS_WARR69, PS_ROGUE69, PS_MAGE69 },
	{ PS_WARR70, PS_ROGUE70, PS_MAGE70 },
	{ PS_DEAD,   PS_ROGUE71, PS_MAGE71 }, // BUGFIX: should use `PS_WARR71` like other classes
	{ PS_WARR72, PS_ROGUE72, PS_MAGE72 },
	{ PS_WARR79, PS_ROGUE79, PS_MAGE79 },
	{ PS_WARR80, PS_ROGUE80, PS_MAGE80 },
	{ PS_WARR82, PS_ROGUE82, PS_MAGE82 },
	{ PS_WARR83, PS_ROGUE83, PS_MAGE83 },
	{ PS_WARR87, PS_ROGUE87, PS_MAGE87 },
	{ PS_WARR88, PS_ROGUE88, PS_MAGE88 },
	{ PS_WARR89, PS_ROGUE89, PS_MAGE89 },
	{ PS_WARR91, PS_ROGUE91, PS_MAGE91 },
	{ PS_WARR92, PS_ROGUE92, PS_MAGE92 },
	{ PS_WARR94, PS_ROGUE94, PS_MAGE94 },
	{ PS_WARR95, PS_ROGUE95, PS_MAGE95 },
	{ PS_WARR96B,PS_ROGUE96, PS_MAGE96 },
	{ PS_WARR97, PS_ROGUE97, PS_MAGE97 },
	{ PS_WARR98, PS_ROGUE98, PS_MAGE98 },
	{ PS_WARR99, PS_ROGUE99, PS_MAGE99 },
#endif
	// clang-format on
};

bool effect_is_playing(int nSFX)
{
	SFXStruct* sfx = &sgSFX[nSFX];

	if (sfx->bFlags & sfx_STREAM)
		return sfx == sgpStreamSFX;
	return sfx->pSnd.IsPlaying();
}

void StopStreamSFX()
{
	if (sgpStreamSFX != NULL) {
		Mix_HaltChannel(SFX_STREAM_CHANNEL);
		sgpStreamSFX->pSnd.Release();
		sgpStreamSFX = NULL;
	}
}

static void StartStreamSFX(SFXStruct* pSFX, int lVolume, int lPan)
{
	// assert(pSFX != NULL);
	// assert(pSFX->bFlags & sfx_STREAM);
	// assert(pSFX->pSnd != NULL);
	if (pSFX == sgpStreamSFX)
		return;
	StopStreamSFX();
	sgpStreamSFX = pSFX;

	sound_stream(pSFX->pszName, &pSFX->pSnd, lVolume, lPan);
}

static void stream_update()
{
	if (sgpStreamSFX != NULL && !sgpStreamSFX->pSnd.IsPlaying()) {
		StopStreamSFX();
	}
}

void InitMonsterSFX(int midx)
{
	char name[DATA_ARCHIVE_MAX_PATH];
	int i, n, j;
	MapMonData* cmon;
	const MonsterData* mdata;
	const MonFileData* mfdata;

	assert(gbSndInited);

	cmon = &mapMonTypes[midx];
	mdata = &monsterdata[cmon->cmType];
	mfdata = &monfiledata[mdata->moFileNum];
	static_assert((int)MS_SPECIAL + 1 == NUM_MON_SFX, "InitMonsterSND requires MS_SPECIAL at the end of the enum.");
	n = mfdata->moSndSpecial ? NUM_MON_SFX : MS_SPECIAL;
	for (i = 0; i < n; i++) {
		for (j = 0; j < lengthof(cmon->cmSnds[i]); j++) {
			snprintf(name, sizeof(name), mfdata->moSndFile, MonstSndChar[i], j + 1);
			assert(!cmon->cmSnds[i][j].IsLoaded());
			sound_file_load(name, &cmon->cmSnds[i][j]);
			assert(cmon->cmSnds[i][j].IsLoaded());
		}
	}
}

void FreeMonsterSFX()
{
	MapMonData* cmon;
	int i, j, k;

	cmon = mapMonTypes;
	for (i = 0; i < nummtypes; i++, cmon++) {
		for (j = 0; j < NUM_MON_SFX; ++j) {
			for (k = 0; k < lengthof(cmon->cmSnds[j]); ++k) {
				//if (cmon->cmSnds[j][k].IsLoaded())
					cmon->cmSnds[j][k].Release();
			}
		}
	}
}

static bool calc_snd_position(int x, int y, int* plVolume, int* plPan)
{
	int pan, volume;

	x -= myplr._px;
	y -= myplr._py;

	pan = (x - y);
	*plPan = pan;

	if (abs(pan) > SFX_DIST_MAX)
		return false;

	volume = std::max(abs(x), abs(y));
	if (volume >= SFX_DIST_MAX)
		return false;

	static_assert(((VOLUME_MAX - VOLUME_MIN) % SFX_DIST_MAX) == 0, "Volume calculation in calc_snd_position requires matching VOLUME_MIN/MAX and SFX_DIST_MAX values.");
	static_assert(((((VOLUME_MAX - VOLUME_MIN) / SFX_DIST_MAX)) & ((VOLUME_MAX - VOLUME_MIN) / SFX_DIST_MAX - 1)) == 0, "Volume calculation in calc_snd_position is no longer optimal for performance.");
	volume *= (VOLUME_MAX - VOLUME_MIN) / SFX_DIST_MAX;
	*plVolume = VOLUME_MAX - volume;

	return true;
}

static void PlaySFX_priv(int psfx, bool loc, int x, int y)
{
	int lPan, lVolume;
	SFXStruct* pSFX;

	if (!gbSoundOn || gbLvlLoad != 0)
		return;

	lPan = 0;
	lVolume = VOLUME_MAX;
	if (loc && !calc_snd_position(x, y, &lVolume, &lPan)) {
		return;
	}

	pSFX = &sgSFX[psfx];
	/* not necessary, since non-streamed sfx should be loaded at this time
	   streams are loaded in StartStreamSFX
	if (!pSFX->pSnd.IsLoaded()) {
		sound_file_load(pSFX->pszName, &pSFX->pSnd);
		// assert(pSFX->pSnd.IsLoaded());
	}*/
	if (pSFX->bFlags & sfx_STREAM) {
		StartStreamSFX(pSFX, lVolume, lPan);
		return;
	}
	assert(pSFX->pSnd.IsLoaded());
	if (!(pSFX->bFlags & sfx_MISC) && pSFX->pSnd.IsPlaying()) {
		return;
	}

	sound_play(&pSFX->pSnd, lVolume, lPan);
}

void PlayEffect(int mnum, int mode)
{
	MonsterStruct* mon;
	int sndIdx, lVolume, lPan;
	SoundSample* snd;

	sndIdx = random_(164, lengthof(mapMonTypes[0].cmSnds[0]));
	if (!gbSoundOn || gbLvlLoad != 0)
		return;

	mon = &monsters[mnum];
	snd = &mapMonTypes[mon->_mMTidx].cmSnds[mode][sndIdx];
	if (!snd->IsLoaded()) {
		assert(mode == MS_SPECIAL);
		assert(!monfiledata[monsterdata[mon->_mType].moFileNum].moSndSpecial);
		return;
	}
	if (snd->IsPlaying()) {
		return;
	}

	if (!calc_snd_position(mon->_mx, mon->_my, &lVolume, &lPan))
		return;

	sound_play(snd, lVolume, lPan);
}

void PlaySFX(int psfx, int rndCnt)
{
	if (rndCnt > 1)
		psfx += random_low(165, rndCnt);
	PlaySFX_priv(psfx, false, 0, 0);
}

void PlaySfxLoc(int psfx, int x, int y, int rndCnt)
{
	if (rndCnt > 1)
		psfx += random_low(165, rndCnt);

	//if (psfx <= PS_WALK4 && psfx >= PS_WALK1) {
	if (psfx == PS_WALK1) {
		sgSFX[psfx].pSnd.nextTc = 0;
	}

	PlaySFX_priv(psfx, true, x, y);
}

void sound_stop()
{
	Mix_HaltChannel(-1);
}

void sound_pause(bool pause)
{
	if (pause)
		Mix_Pause(-1);
	else
		Mix_Resume(-1);
}

void sound_update()
{
	assert(gbSndInited);

	stream_update();
}

static void priv_sound_free(BYTE bLoadMask)
{
	int i;

	for (i = 0; i < lengthof(sgSFX); i++) {
		if (/*sgSFX[i].pSnd.IsLoaded() &&*/ (sgSFX[i].bFlags & bLoadMask)) {
			sgSFX[i].pSnd.Release();
		}
	}
}

static void priv_sound_init(BYTE bLoadMask)
{
	int i;

	assert(gbSndInited);

	for (i = 0; i < lengthof(sgSFX); i++) {
		if ((sgSFX[i].bFlags & bLoadMask) != sgSFX[i].bFlags) {
			continue;
		}

		assert(!sgSFX[i].pSnd.IsLoaded());

		sound_file_load(sgSFX[i].pszName, &sgSFX[i].pSnd);
	}
}

void InitGameSFX()
{
#ifdef HELLFIRE
	BYTE mask = sfx_MISC | sfx_HELLFIRE;
#else
	BYTE mask = sfx_MISC;
#endif
	if (IsLocalGame) {
		mask |= sgSFXSets[SFXS_MASK][myplr._pClass];
	} else {
		mask |= sfx_WARRIOR | sfx_ROGUE | sfx_SORCERER;
#ifdef HELLFIRE
		mask |= sfx_MONK;
#endif
	}

	priv_sound_init(mask);
}

void InitUiSFX()
{
	priv_sound_init(sfx_UI);
}

void FreeGameSFX()
{
	priv_sound_free(~(sfx_UI | sfx_STREAM));
}

void FreeUiSFX()
{
	priv_sound_free(sfx_UI);
}

#endif // NOSOUND

DEVILUTION_END_NAMESPACE
