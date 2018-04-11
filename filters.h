#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>
#include <stddef.h>

static inline void filters_apply_brightness_contrast(
                       uint8_t *pixels,
                       size_t position,
                       float brightness,
                       float contrast
                   );

static inline void filters_apply_sepia(uint8_t *pixels, size_t position);

#define MEDIAN_FILTER_WINDOW_SIZE 4
static inline void filters_apply_median(
                       uint8_t *pixels,
                       size_t position
                   );

#include "filters.inline.c"

#endif /* FILTERS_H */

