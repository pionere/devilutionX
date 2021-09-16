/**
 * @file capture.h
 *
 * Interface of the screenshot function.
 */
#ifndef __CAPTURE_H__
#define __CAPTURE_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Save the current screen to a screen??.PCX (00-99) in file if available, then make the screen red for 200ms.
 *
void CaptureScreen();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CAPTURE_H__ */
