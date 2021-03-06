/**
 * @file itemdat.h
 *
 * Interface of all item data.
 */
#ifndef __ITEMDAT_H__
#define __ITEMDAT_H__

DEVILUTION_BEGIN_NAMESPACE

#define ITEM_ANIM_WIDTH		96
#define ITEM_ANIM_XOFFSET	16

#ifdef __cplusplus
extern "C" {
#endif

extern const ItemData AllItemsList[NUM_IDI];
extern const AffixData PL_Prefix[];
extern const AffixData PL_Suffix[];
extern const UniqItemData UniqueItemList[NUM_UITEM];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __ITEMDAT_H__ */
