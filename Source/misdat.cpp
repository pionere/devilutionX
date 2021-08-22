/**
 * @file misdat.cpp
 *
 * Implementation of data related to missiles.
 */
#include "all.h"
#include "misproc.h"

DEVILUTION_BEGIN_NAMESPACE

/** Data related to each missile ID. */
const MissileData missiledata[] = {
	// clang-format off
	// mName,             mAddProc,                   mProc,              mDraw, mType,                          mdFlags, mResist,        mFileNum,        mlSFX,       miSFX,       mlSFXCnt, miSFXCnt
	{  MIS_ARROW,         &AddArrow,                  &MI_Arrow,          TRUE,      0,                                0, MISR_NONE,      MFILE_ARROWS,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_PBARROW,       &AddArrow,                  &MI_Arrow,          TRUE,      0,                                0, MISR_NONE,      MFILE_ARROWS,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_ASARROW,       &AddArrow,                  &MI_AsArrow,        TRUE,      0,                                0, MISR_NONE,      MFILE_ARROWS,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_ARROWC,        &AddArrowC,                 &MI_Dummy,          FALSE,     0,                                0, MISR_NONE,      MFILE_NONE,      PS_BFIRE,    SFX_NONE,    1,        1        },
	{  MIS_PBARROWC,      &AddArrowC,                 &MI_Dummy,          FALSE,     0,                                0, MISR_NONE,      MFILE_NONE,      PS_BFIRE,    SFX_NONE,    1,        1        },
	{  MIS_ASARROWC,      &AddArrowC,                 &MI_Dummy,          FALSE,     0,                                0, MISR_NONE,      MFILE_NONE,      PS_BFIRE,    SFX_NONE,    1,        1        },
	{  MIS_FIREBOLT,      &AddFirebolt,               &MI_Firebolt,       TRUE,      1,                                0, MISR_FIRE,      MFILE_FIREBA,    LS_FBOLT1,   LS_FIRIMP2,  1,        1        },
	{  MIS_HBOLT,         &AddFirebolt,               &MI_Firebolt,       TRUE,      1,                                0, MISR_ACID,      MFILE_HOLY,      LS_HOLYBOLT, LS_ELECIMP1, 1,        1        },
	{  MIS_FLARE,         &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_FLARE,     SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_SNOWWICH,      &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_SCUBMISB,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_HLSPWN,        &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_SCUBMISD,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_SOLBRNR,       &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_SCUBMISC,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_MAGMABALL,     &AddMagmaball,              &MI_Firebolt,       TRUE,      1,                                0, MISR_FIRE,      MFILE_MAGBALL,   SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_ACID,          &AddAcid,                   &MI_Firebolt,       TRUE,      1,                                0, MISR_ACID,      MFILE_ACIDBF,    LS_ACID,     SFX_NONE,    2,        1        },
	{  MIS_ACIDPUD,       &AddAcidpud,                &MI_Acidpud,        TRUE,      2, MIF_NOBLOCK | MIF_AREA | MIF_DOT, MISR_ACID,      MFILE_ACIDPUD,   LS_PUDDLE,   SFX_NONE,    1,        1        },
	{  MIS_EXACIDP,       &AddMisexp,                 &MI_Acidsplat,      TRUE,      2,                                0, MISR_ACID,      MFILE_ACIDSPLA,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXFIRE,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_MAGBLOS,   SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXLGHT,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_MINILTNG,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXMAGIC,       &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_MAGICEXP,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXACID,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_GREENEXP,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXHOLY,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_HOLYEXPL,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXFLARE,       &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_FLAREEXP,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXSNOWWICH,    &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_SCBSEXPB,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXHLSPWN,      &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_SCBSEXPD,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXSOLBRNR,     &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_SCBSEXPC,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_GUARDIAN,      &AddGuardian,               &MI_Guardian,       TRUE,      1,                                0, MISR_NONE,      MFILE_GUARD,     LS_GUARD,    LS_GUARDLAN, 1,        1        },
	{  MIS_RNDTELEPORT,   &AddRndTeleport,            &MI_Teleport,       FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_TELEPORT, SFX_NONE,    1,        1        },
	{  MIS_LIGHTBALL,     &AddLightball,              &MI_Lightball,      TRUE,      1,                          MIF_DOT, MISR_LIGHTNING, MFILE_LGHNING,   LS_LNING1,   LS_ELECIMP1, 1,        1        },
	{  MIS_FIREBALL,      &AddFireball,               &MI_Fireball,       TRUE,      1,                                0, MISR_FIRE,      MFILE_FIREBA,    LS_FBOLT1,   LS_FIRIMP2,  1,        1        },
	{  MIS_LIGHTNINGC,    &AddLightningC,             &MI_LightningC,     FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_LIGHTNING,     &AddLightning,              &MI_Lightning,      TRUE,      1,            MIF_NOBLOCK | MIF_DOT, MISR_LIGHTNING, MFILE_LGHNING,   LS_LNING1,   LS_ELECIMP1, 1,        1        },
	{  MIS_LIGHTNINGC2,   &AddLightningC,             &MI_LightningC,     FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_LIGHTNING2,    &AddLightning,              &MI_Lightning,      TRUE,      1,            MIF_NOBLOCK | MIF_DOT, MISR_LIGHTNING, MFILE_THINLGHT,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_TOWN,          &AddTown,                   &MI_Portal,         TRUE,      1,                                0, MISR_NONE,      MFILE_PORTAL,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RPORTAL,       &AddPortal,                 &MI_Portal,         TRUE,      2,                                0, MISR_NONE,      MFILE_RPORTAL,   SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_FLASH,         &AddFlash,                  &MI_Flash,          TRUE,      1, MIF_NOBLOCK | MIF_AREA | MIF_DOT, MISR_MAGIC,     MFILE_BLUEXFR,   LS_NOVA,     LS_ELECIMP1, 1,        1        },
	{  MIS_FLASH2,        &AddFlash2,                 &MI_Flash2,         TRUE,      1,                                0, MISR_NONE,      MFILE_BLUEXBK,   SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_MANASHIELD,    &AddManashield,             &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_MSHIELD,  SFX_NONE,    1,        1        },
	{  MIS_CHAIN,         &AddChain,                  &MI_Chain,          TRUE,      1,                      MIF_NOBLOCK, MISR_LIGHTNING, MFILE_LGHNING,   LS_LNING1,   LS_ELECIMP1, 1,        1        },
	//{  MIS_BLODSTAR,      NULL,                       NULL,               TRUE,      2,                                0, MISR_NONE,      MFILE_BLOOD,     LS_BLODSTAR, LS_BLSIMPT,  1,        1        },
	//{  MIS_BONE,          NULL,                       NULL,               TRUE,      2,                                0, MISR_NONE,      MFILE_BONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_METLHIT,       NULL,                       NULL,               TRUE,      2,                                0, MISR_NONE,      MFILE_METLHIT,   SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RHINO,         &AddRhino,                  &MI_Rhino,          TRUE,      2,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_TELEPORT,      &AddTeleport,               &MI_Teleport,       FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_ELEMENTL, SFX_NONE,    1,        1        },
	//{  MIS_FARROW,        NULL,                       NULL,               TRUE,      0,                                0, MISR_FIRE,      MFILE_FARROW,    SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_DOOMSERP,      NULL,                       NULL,               FALSE,     1,                                0, MISR_MAGIC,     MFILE_DOOM,      LS_DSERP,    SFX_NONE,    1,        1        },
	{  MIS_STONE,         &AddStone,                  &MI_Stone,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_SCURIMP,  SFX_NONE,    1,        1        },
	//{  MIS_INVISIBL,      NULL,                       NULL,               FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_INVISIBL, SFX_NONE,    1,        1        },
	{  MIS_GOLEM,         &AddGolem,                  &MI_Golem,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_GOLUM,    SFX_NONE,    1,        1        },
	//{  MIS_ETHEREALIZE,   NULL,                       NULL,               TRUE,      1,                                0, MISR_NONE,      MFILE_ETHRSHLD,  LS_ETHEREAL, SFX_NONE,    1,        1        },
	//{  MIS_BLODBUR,       NULL,                       NULL,               TRUE,      2,                                0, MISR_NONE,      MFILE_BLODBUR,   SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_EXAPOCA,       &AddApocaExp,               &MI_ApocaExp,       TRUE,      2,                                0, MISR_NONE,      MFILE_NEWEXP,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_FIREWALLC,     &AddWallC,                  &MI_WallC,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_FIREWALL,      &AddFirewall,               &MI_Firewall,       TRUE,      1, MIF_NOBLOCK | MIF_AREA | MIF_DOT, MISR_FIRE,      MFILE_FIREWAL,   LS_WALLLOOP, LS_FIRIMP2,  1,        1        },
	{  MIS_INFRA,         &AddInfra,                  &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_INFRAVIS, SFX_NONE,    1,        1        },
	{  MIS_FIREWAVEC,     &AddFireWaveC,              &MI_FireWaveC,      FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_FLAMWAVE, SFX_NONE,    1,        1        },
	{  MIS_FIREWAVE,      &AddFireWave,               &MI_FireWave,       TRUE,      1,               MIF_AREA | MIF_DOT, MISR_FIRE,      MFILE_FIREWAL,   SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_LIGHTNOVAC,    &AddLightNovaC,             &MI_NovaC,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_NOVA,     SFX_NONE,    1,        1        },
	//{  MIS_APOCAC,        &AddApocaC,                 &MI_ApocaC,         FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_APOC,     SFX_NONE,    1,        1        },
	{  MIS_HEAL,          &AddHeal,                   &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_HEALOTHER,     &AddHealOther,              &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RESURRECT,     &AddResurrect,              &MI_Resurrect,      TRUE,      1,                                0, MISR_NONE,      MFILE_RESSUR1,   SFX_NONE,    LS_RESUR,    1,        1        },
	{  MIS_TELEKINESIS,   &AddTelekinesis,            &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_ETHEREAL, SFX_NONE,    1,        1        },
	//{  MIS_LARROW,        NULL,                       NULL,               TRUE,      0,                                0, MISR_LIGHTNING, MFILE_LARROW,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_ABILITY,       &AddAbility,                &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_IDENTIFY,      &AddAbility,                &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_REPAIR,        &AddAbility,                &MI_Dummy,          FALSE,     2,                                0, MISR_NONE,      MFILE_NONE,      IS_REPAIR,   SFX_NONE,    1,        1        },
	{  MIS_DISARM,        &AddDisarm,                 &MI_Dummy,          FALSE,     2,                                0, MISR_NONE,      MFILE_NONE,      LS_TRAPDIS,  SFX_NONE,    1,        1        },
	{  MIS_INFERNOC,      &AddInfernoC,               &MI_InfernoC,       FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_INFERNO,       &AddInferno,                &MI_Inferno,        TRUE,      1, MIF_NOBLOCK | MIF_AREA | MIF_DOT, MISR_FIRE,      MFILE_INFERNO,   LS_SPOUTSTR, SFX_NONE,    1,        1        },
	{  MIS_FIRETRAP,      &AddFireTrap,               &MI_FireTrap,       FALSE,     1,               MIF_AREA | MIF_DOT, MISR_FIRE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_BARRELEX,      &AddBarrelExp,              &MI_Dummy,          FALSE,     1,                                0, MISR_FIRE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_FIREMAN,       &AddFireman,                &MI_Fireman,        TRUE,      2,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_KRULL,         &AddKrull,                  &MI_Krull,          TRUE,      0,                                0, MISR_FIRE,      MFILE_KRULL,     SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_CBOLTC,        &AddCboltC,                 &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_CBOLT,         &AddCbolt,                  &MI_Cbolt,          TRUE,      1,                                0, MISR_LIGHTNING, MFILE_MINILTNG,  LS_CBOLT,    SFX_NONE,    1,        1        },
	{  MIS_ELEMENTAL,     &AddElemental,              &MI_Elemental,      TRUE,      1,                                0, MISR_FIRE,      MFILE_FIRERUN,   LS_ELEMENTL, SFX_NONE,    1,        1        },
	{  MIS_EXELE,         &AddMisexp,                 &MI_EleExp,         TRUE,      1,                                0, MISR_NONE,      MFILE_BIGEXP,    SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_BONESPIRIT,    &AddBoneSpirit,             &MI_Bonespirit,     TRUE,      1,                                0, MISR_MAGIC,     MFILE_SKLBALL,   LS_BONESP,   LS_BSIMPCT,  1,        1        },
	{  MIS_APOCAC2,       &AddApocaC2,                &MI_Dummy,          FALSE,     2,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXAPOCA2,      &AddApocaExp,               &MI_ApocaExp,       TRUE,      2,                                0, MISR_NONE,      MFILE_FIREPLAR,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RAGE,          &AddRage,                   &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
#ifdef HELLFIRE
	//{  MIS_LIGHTWALLC,    &AddWallC,                  &MI_WallC,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_LIGHTWALL,     &AddLightwall,              &MI_Lightwall,      TRUE,      1, MIF_NOBLOCK | MIF_AREA | MIF_DOT, MISR_LIGHTNING, MFILE_LGHNING,   LS_LMAG,     LS_ELECIMP1, 1,        1        },
	//{  MIS_FIRENOVAC,     &AddFireNovaC,              &MI_NovaC,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      LS_FBOLT1,   SFX_NONE,    1,        1        },
	//{  MIS_FIREBALL2,     &AddFireball2,              &MI_Fireball,       TRUE,      1,                                0, MISR_FIRE,      MFILE_FIREBA,    IS_FBALLBOW, LS_FIRIMP2,  1,        1        },
	//{  MIS_REFLECT,       &AddReflection,             &MI_Reflect,        TRUE,      1,                                0, MISR_NONE,      MFILE_REFLECT,   LS_MSHIELD,  SFX_NONE,    1,        1        },
	{  MIS_FIRERING,      &AddRingC,                  &MI_RingC,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	//{  MIS_MANATRAP,      &AddManaTrap,               &MI_Dummy,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      IS_CAST7,    SFX_NONE,    1,        1        },
	//{  MIS_LIGHTRING,     &AddRingC,                  &MI_RingC,          FALSE,     1,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RUNEFIRE,      &AddFireRune,               &MI_Rune,           TRUE,      1,                                0, MISR_NONE,      MFILE_RUNE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RUNELIGHT,     &AddLightRune,              &MI_Rune,           TRUE,      1,                                0, MISR_NONE,      MFILE_RUNE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RUNENOVA,      &AddNovaRune,               &MI_Rune,           TRUE,      1,                                0, MISR_NONE,      MFILE_RUNE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RUNEWAVE,      &AddWaveRune,               &MI_Rune,           TRUE,      1,                                0, MISR_NONE,      MFILE_RUNE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_RUNESTONE,     &AddStoneRune,              &MI_Rune,           TRUE,      1,                                0, MISR_NONE,      MFILE_RUNE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_HIVEEXP,       &AddHiveexp,                &MI_Hiveexp,        TRUE,      1,                                0, MISR_FIRE,      MFILE_BIGEXP,    LS_NESTXPLD, LS_NESTXPLD, 1,        1        },
	{  MIS_HORKDMN,       &AddHorkSpawn,              &MI_HorkSpawn,      TRUE,      2,                                0, MISR_NONE,      MFILE_SPAWNS,    SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_HIVEEXPC,      &AddHiveexpC,               &MI_Dummy,          FALSE,     2,                                0, MISR_NONE,      MFILE_NONE,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_LICH,          &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_LICH,      SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_PSYCHORB,      &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_BONEDEMON, SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_NECROMORB,     &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_NECROMORB, SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_ARCHLICH,      &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_ARCHLICH,  SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_BONEDEMON,     &AddFlare,                  &MI_Firebolt,       TRUE,      1,                                0, MISR_MAGIC,     MFILE_BONEDEMON, SFX_NONE,    SFX_NONE,    1,        1        },
	{  MIS_EXYEL2,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_EXYEL2,    LS_FIRIMP2,  SFX_NONE,    1,        1        },
	{  MIS_EXRED3,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_EXRED3,    LS_FIRIMP2,  SFX_NONE,    1,        1        },
	{  MIS_EXBL2,         &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_EXBL2,     LS_FIRIMP2,  SFX_NONE,    1,        1        },
	{  MIS_EXBL3,         &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_EXBL3,     LS_FIRIMP2,  SFX_NONE,    1,        1        },
	{  MIS_EXORA1,        &AddMisexp,                 &MI_Misexp,         TRUE,      2,                                0, MISR_NONE,      MFILE_EXORA1,    LS_FIRIMP2,  SFX_NONE,    1,        1        },
#endif
	// clang-format on
};

/** Data related to each missile graphic ID. */
const MisFileData misfiledata[NUM_MFILE + 1] = {
	// clang-format off
// anim_index      mfAnimFAmt, mfName,      mfAnimTrans,                     mfFlags,               mfAnimFrameLen[16],                                 mfAnimLen[16],                                                      mfAnimWidth, mfAnimXOffset
/*MFILE_ARROWS*/    {       1, "Arrows",    NULL,                            MAFLAG_LOCK_ANIMATION, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_FIREBA*/    {      16, "Fireba",    NULL,                            0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },          96,           16 },
/*MFILE_GUARD*/     {       3, "Guard",     NULL,                            0,                     { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 15, 14,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_LGHNING*/   {       1, "Lghning",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_FIREWAL*/   {       2, "Firewal",   NULL,                            0,                     { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 13, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_MAGBLOS*/   {       1, "MagBlos",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_PORTAL*/    {       2, "Portal",    NULL,                            0,                     { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_BLUEXFR*/   {       1, "Bluexfr",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         160,           48 },
/*MFILE_BLUEXBK*/   {       1, "Bluexbk",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         160,           48 },
/*MFILE_MANASHLD*/  {       1, "Manashld",  NULL,                            MAFLAG_UNUSED | MAFLAG_LOCK_ANIMATION, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, 96,    16 },
/*MFILE_BONE*/      {       3, "Bone",      NULL,                            MAFLAG_UNUSED,         { 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  8,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_METLHIT*/   {       3, "Metlhit",   NULL,                            MAFLAG_UNUSED,         { 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10, 10, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_FARROW*/    {      16, "Farrow",    NULL,                            0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },          96,           16 },
/*MFILE_LARROW*/    {      16, "Larrow",    NULL,                            0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },          96,           16 },
/*MFILE_MARROW*/    {      16, "Larrow",    "Monsters\\Monsters\\BR.TRN",    0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },          96,           16 },
/*MFILE_PARROW*/    {      16, "Farrow",    "Monsters\\FalSpear\\Dark.TRN",  0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, {  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },          96,           16 },
/*MFILE_MAGICEXP*/  {       1, "Magicexp",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          64,            0 },
/*MFILE_GREENEXP*/  {       1, "Greenexp",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          64,            0 },
/*MFILE_DOOM*/      {       9, "Doom",      NULL,                            MAFLAG_UNUSED,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_BLODBUR*/   {       2, "Blodbur",   NULL,                            MAFLAG_UNUSED,         { 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_NEWEXP*/    {       1, "Newexp",    NULL,                            MAFLAG_UNUSED,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_SHATTER1*/  {       1, "Shatter1",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_BIGEXP*/    {       1, "Bigexp",    NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         160,           48 },
/*MFILE_INFERNO*/   {       1, "Inferno",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_THINLGHT*/  {       1, "Thinlght",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_FLARE*/     {       1, "Flare",     NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_FLAREEXP*/  {       1, "Flareexp",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_MAGBALL*/   {       8, "Magball",   NULL,                            MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16, 16, 16, 16, 16, 16, 16, 16,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_KRULL*///     {       1, "Krull",     NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_MINILTNG*/  {       1, "Miniltng",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          64,            0 },
/*MFILE_HOLY*/      {      16, "Holy",      NULL,                            0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },          96,           16 },
/*MFILE_HOLYEXPL*/  {       1, "Holyexpl",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         160,           48 },
/*MFILE_FIRARWEX*/  {       1, "Firarwex",  NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          64,            0 },
/*MFILE_ACIDBF*/    {      16, "Acidbf",    NULL,                            MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, {  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 },          96,           16 },
/*MFILE_ACIDSPLA*/  {       1, "Acidspla",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_ACIDPUD*/   {       2, "Acidpud",   NULL,                            MAFLAG_HIDDEN,         { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  9,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_ETHRSHLD*/  {       1, "Ethrshld",  NULL,                            MAFLAG_UNUSED,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_FIRERUN*/   {       8, "Firerun",   NULL,                            0,                     { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, { 12, 12, 12, 12, 12, 12, 12, 12,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_RESSUR1*/   {       1, "Ressur1",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_SKLBALL*/   {       9, "Sklball",   NULL,                            MAFLAG_UNUSED,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, { 16, 16, 16, 16, 16, 16, 16, 16,  8,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_RPORTAL*/   {       2, "Rportal",   NULL,                            0,                     { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_FIREPLAR*/  {       1, "Fireplar",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         160,           48 },
/*MFILE_SCUBMISB*/  {       1, "Scubmisb",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_SCBSEXPB*/  {       1, "Scbsexpb",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_SCUBMISC*/  {       1, "Scubmisc",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_SCBSEXPC*/  {       1, "Scbsexpc",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
/*MFILE_SCUBMISD*/  {       1, "Scubmisd",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_SCBSEXPD*/  {       1, "Scbsexpd",  NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         128,           32 },
#ifdef HELLFIRE
/*MFILE_SPAWNS*/    {       8, "spawns",    NULL,                            MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, {  9,  9,  9,  9,  9,  9,  9,  9,  0,  0,  0,  0,  0,  0,  0,  0 },          96,           16 },
/*MFILE_REFLECT*/   {       1, "reflect",   NULL,                            MAFLAG_UNUSED | MAFLAG_LOCK_ANIMATION, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, 160,  160 },
/*MFILE_LICH*/      {      16, "ms_ora",    NULL,                            MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },          96,            8 },
/*MFILE_MSBLA*/     {      16, "ms_bla",    NULL,                            MAFLAG_UNUSED,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },          96,            8 },
/*MFILE_NECROMORB*/ {      16, "ms_reb",    "Monsters\\Monsters\\BLKJD.TRN", MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },          96,            8 },
/*MFILE_ARCHLICH*/  {      16, "ms_yeb",    "Monsters\\Fat\\FatB.TRN",       MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },          96,            8 },
/*MFILE_RUNE*/      {       1, "rglows1",   NULL,                            0,                     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,            8 },
/*MFILE_EXYEL2*/    {       1, "ex_yel2",   "Monsters\\Monsters\\DE.TRN",    MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         220,           78 },
/*MFILE_EXBL2*/     {       1, "ex_yel2",   NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         220,           78 },
/*MFILE_EXRED3*/    {       1, "ex_yel2",   "Monsters\\Monsters\\SKFR.TRN",  MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         220,           78 },
/*MFILE_BONEDEMON*/ {      16, "ms_ora",    "Monsters\\Monsters\\DSFM.TRN",  MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },          96,            8 },
/*MFILE_EXORA1*/    {       1, "ex_ora1",   NULL,                            MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,          -12 },
/*MFILE_EXBL3*/     {       1, "ex_ora1",   "Monsters\\Monsters\\DSFM.TRN",  MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },          96,          -12 },
#endif
/*MFILE_NONE*/      {       0, "",          NULL,                            0,                     { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },           0,            0 },
	// clang-format on
};

// container for live data of missile-animations
BYTE* misanimdata[NUM_MFILE + 1][16] = { { 0 } };

/* Unused missile graphic:
// anim_index      mfAnimFAmt, mfName,      mfFlags,               mfAnimFrameLen[16],                                 mfAnimLen[16],                                                      mfAnimWidth[16],                                                            mfAnimXOffset[16]
..MFILE_BLOOD..		{       4, "Blood",     MAFLAG_UNUSED,         { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 15,  8,  8,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  96, 128, 128, 128,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0 }, {  16, 32, 32, 32,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
..MFILE_EXBL2..     {       1, "ex_blu2",   NULL,                           MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         212,           86 },
..MFILE_EXRED3..    {       1, "ex_red3",   NULL,                           MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         292,          114 },
..MFILE_BONEDEMON.. {      16, "ms_blb",    NULL,                           MAFLAG_HIDDEN,         { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 },          96,            8 },
..MFILE_EXBL3..     {       1, "ex_blu3",   NULL,                           MAFLAG_HIDDEN,         { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, {  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },         292,          114 },
*/
DEVILUTION_END_NAMESPACE
