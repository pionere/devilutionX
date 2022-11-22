/**
 * @file cel_render.hpp
 *
 * CEL rendering.
 */
#ifndef __CEL_RENDER_H__
#define __CEL_RENDER_H__

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Blit CEL sprite to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 */
void CelDraw(int sx, int sy, const CelImageBuf* pCelBuff, int nCel);

/**
 * @brief Blit CEL sprite with frame header to the back buffer at the given coordinates
 * @param out Target buffer
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDraw(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Blit CEL sprite, and apply lighting, to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 */
void CelDrawLight(int sx, int sy, const CelImageBuf* pCelBuff, int nCel, const BYTE* tbl);

/**
 * @brief Blit CEL sprite with frame header, and apply lighting, to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDrawLight(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Same as CelClippedDrawLight optionally drawing in stippled-transparent mode
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDrawLightTrans(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Apply red hue to the CEL sprite and blit to the back buffer at the given coordinates
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDrawLightRed(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Blit an outline one pixel larger then the given sprite shape to the target buffer at the given coordinates
 * @param col Color index from current palette
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param pCelBuff CEL buffer
 * @param nCel CEL frame number
 * @param nWidth CEL width of the frame
 */
void CelClippedDrawOutline(BYTE col, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CEL_RENDER_H__ */
