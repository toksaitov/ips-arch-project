#ifndef FILTERS_THREADING_H
#define FILTERS_THREADING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct _filters_brightness_contrast_data
{
    size_t linear_position, row_padding;
    size_t width, height;
    uint8_t *pixels;
    float brightness, contrast;
    volatile size_t *rows_left_shared;
    volatile bool *barrier_sense;
} filters_brightness_contrast_data_t;

typedef struct _filters_sepia_data
{
    size_t linear_position, row_padding;
    size_t width, height;
    uint8_t *pixels;
    volatile size_t *rows_left_shared;
    volatile bool *barrier_sense;
} filters_sepia_data_t;

typedef struct _filters_median_data
{
    size_t linear_position, row_padding;
    size_t width, height;
    uint8_t *pixels;
    volatile size_t *rows_left_shared;
    volatile bool *barrier_sense;
} filters_median_data_t;

static inline filters_brightness_contrast_data_t *filters_brightness_contrast_data_create(
                                                       size_t linear_position,
                                                       size_t row_padding,
                                                       size_t width,
                                                       size_t height,
                                                       uint8_t *pixels,
                                                       float brightness,
                                                       float contrast,
                                                       volatile size_t *rows_left_shared,
                                                       volatile bool *barrier_sense
                                                  );

static inline void filters_brightness_contrast_data_destroy(
                       filters_brightness_contrast_data_t *data
                   );

static inline filters_sepia_data_t *filters_sepia_data_create(
                                        size_t linear_position,
                                        size_t row_padding,
                                        size_t width,
                                        size_t height,
                                        uint8_t *pixels,
                                        volatile size_t *rows_left_shared,
                                        volatile bool *barrier_sense
                                    );

static inline void filters_sepia_data_destroy(
                       filters_sepia_data_t *data
                   );

static inline filters_median_data_t *filters_median_data_create(
                                         size_t linear_position,
                                         size_t row_padding,
                                         size_t width,
                                         size_t height,
                                         uint8_t *pixels,
                                         volatile size_t *rows_left_shared,
                                         volatile bool *barrier_sense
                                     );

static inline void filters_median_data_destroy(
                       filters_median_data_t *data
                   );

/* Threading Tasks */

static void filters_brightness_contrast_processing_task(
                void *task_data,
                void (*result_callback)(void *result)
            );

static void filters_sepia_processing_task(
                void *task_data,
                void (*result_callback)(void *result)
            );

static void filters_median_processing_task(
                void *task_data,
                void (*result_callback)(void *result)
            );

#include "filters_threading.inline.c"

#endif /* FILTERS_THREADING_H */

