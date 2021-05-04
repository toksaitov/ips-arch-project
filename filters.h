#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include <stddef.h>

#define FILTERS_BRIGHTNESS_CONTRAST_ID 0
#define FILTERS_SEPIA_ID               1
#define FILTERS_MEDIAN_ID              2

#define FILTERS_MEDIAN_WINDOW_SIZE 3

static inline void filters_apply_brightness_contrast(
                       uint8_t *pixels,
                       size_t position,
                       float brightness,
                       float contrast
                   );

static inline void filters_apply_sepia(
                       uint8_t *pixels,
                       size_t position
                   );

static inline void filters_apply_median(
                       uint8_t *source_pixels,
                       uint8_t *destination_pixels,
                       size_t position,
                       size_t x,
                       size_t y,
                       size_t width,
                       size_t height
                   );

#include "filters.impl.h.c"

#endif /* FILTERS_H */

