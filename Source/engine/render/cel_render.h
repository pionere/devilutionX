/**
 * @file cel_render.h
 *
 * CEL rendering.
 */
#ifndef __CEL_RENDER_H__
#define __CEL_RENDER_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Blit CEL sprite to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 */
void CelDraw(int sx, int sy, const CelImageBuf* pCelBuff, int nCel);

/**
 * @brief Blit CEL sprite, and apply trn, to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 * @param tbl Palette translation table
 */
void CelDrawTrnTbl(int sx, int sy, const CelImageBuf* pCelBuff, int nCel, const BYTE* tbl);

/**
 * @brief Blit CEL sprite, and optionally use stippled-transparency or light trn, to the given buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDrawLightTrans(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Blit CEL sprite, and apply a given lighting/trn, to the given buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 * @param light index of the light shade/trn to use
 */
void CelClippedDrawLightTbl(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth, BYTE light);

/**
 * @brief Blit CEL sprite with an outline one pixel larger then the given sprite shape to the target buffer at the given coordinates
 * @param col color of the sprite and the outline (Color index from current palette)
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff pointer to CEL-frame offsets and data with width information
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDrawOutline(BYTE col, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CEL_RENDER_H__ */
