#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include <stddef.h>

static inline void apply_brightness_contrast_filter(
                       uint8_t *pixels,
                       size_t position,
                       float brightness,
                       float contrast
                   );

static inline void apply_sepia_filter(uint8_t *pixels, size_t position);

#define MEDIAN_FILTER_WINDOW_SIZE 4
static inline void apply_median_filter(
                       uint8_t *pixels,
                       size_t position
                   );

#include "filters.inline.c"

#endif /* FILTERS_H */

