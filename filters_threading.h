#ifndef FILTERS_THREADING_H
#define FILTERS_THREADING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct _filters_brightness_contrast_data
{
    size_t linear_position;
    size_t channels_to_process;
    uint8_t *pixels;
    float brightness, contrast;
    volatile ssize_t *channels_left;
    volatile bool *barrier_sense;
} filters_brightness_contrast_data_t;

typedef struct _filters_sepia_data
{
    size_t linear_position;
    size_t channels_to_process;
    uint8_t *pixels;
    volatile ssize_t *channels_left;
    volatile bool *barrier_sense;
} filters_sepia_data_t;

typedef struct _filters_median_data
{
    size_t linear_position;
    size_t channels_to_process;
    size_t image_width, image_height;
    uint8_t *source_pixels;
    uint8_t *destination_pixels;
    volatile ssize_t *channels_left;
    volatile bool *barrier_sense;
} filters_median_data_t;

static inline filters_brightness_contrast_data_t *filters_brightness_contrast_data_create(
                                                       size_t linear_position,
                                                       size_t channels_to_process,
                                                       uint8_t *pixels,
                                                       float brightness,
                                                       float contrast,
                                                       volatile ssize_t *channels_left,
                                                       volatile bool *barrier_sense
                                                  );

static inline void filters_brightness_contrast_data_destroy(
                       filters_brightness_contrast_data_t *data
                   );

static inline filters_sepia_data_t *filters_sepia_data_create(
                                        size_t linear_position,
                                        size_t channels_to_process,
                                        uint8_t *pixels,
                                        volatile ssize_t *channels_left,
                                        volatile bool *barrier_sense
                                    );

static inline void filters_sepia_data_destroy(
                       filters_sepia_data_t *data
                   );

static inline filters_median_data_t *filters_median_data_create(
                                         size_t linear_position,
                                         size_t channels_to_process,
                                         size_t image_width,
                                         size_t image_height,
                                         uint8_t *source_pixels,
                                         uint8_t *destination_pixels,
                                         volatile ssize_t *channels_left,
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

#include "filters_threading.impl.h.c"

#endif /* FILTERS_THREADING_H */

