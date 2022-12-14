/**
 * @file cl2_render.hpp
 *
 * CL2 rendering.
 */
#ifndef __CL2_RENDER_H__
#define __CL2_RENDER_H__

#include "../types.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Apply the color swaps to a CL2 sprite
 * @param p CL2 buffer
 * @param ttbl Palette translation table
 * @param nCel number of frames in the CL2 file
 */
void Cl2ApplyTrans(BYTE* p, const BYTE* ttbl, int nCel);

/**
 * @brief Blit CL2 sprite, to the back buffer at the given coordinates
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth CL2 width of the frame
 */
void Cl2Draw(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Blit an outline one pixel larger than the given sprite shape to the back buffer at the given coordinates
 * @param col Color index from current palette
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth CL2 width of the frame
 */
void Cl2DrawOutline(BYTE col, int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

/**
 * @brief Blit CL2 sprite, and apply a given lighting, to the given buffer at the given coordinates
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pCelBuff CL2 buffer
 * @param nCel CL2 frame number
 * @param nWidth CL2 width of the frame
 * @param light index of the light shade to use
 */
void Cl2DrawLightTbl(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth, BYTE light);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CL2_RENDER_H__ */
