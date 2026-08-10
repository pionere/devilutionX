/**
 * @file cl2_render.h
 *
 * CL2 rendering.
 */
#ifndef __CL2_RENDER_H__
#define __CL2_RENDER_H__

#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Blit CL2 sprite, and apply a given lighting/trn, to the given buffer at the given coordinates
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pCelBuff pointer to CL2-frame offsets and data
 * @param nCel CL2 frame number
 * @param nWidth CL2 width of the frame
 */
void Cl2Draw(int sx, int sy, const BYTE* pCelBuff, int nCel, int nWidth);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CL2_RENDER_H__ */
