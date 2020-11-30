/**
 * @file dx.h
 *
 * Interface of functions setting up the graphics pipeline.
 */
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

DEVILUTION_BEGIN_NAMESPACE

extern SDL_Palette *palette;
extern SDL_Surface *pal_surface;
extern unsigned int pal_surface_palette_version;

template <
    typename T,
    typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
void OutputToLogical(T *x, T *y);

DEVILUTION_END_NAMESPACE

#endif /* __DISPLAY_H__ */
