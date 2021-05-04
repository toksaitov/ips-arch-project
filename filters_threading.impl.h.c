#include "filters_threading.h"
#include "filters.h"

#include <stdlib.h>

static inline filters_brightness_contrast_data_t *filters_brightness_contrast_data_create(
                                                      size_t linear_position,
                                                      size_t channels_to_process,
                                                      uint8_t *pixels,
                                                      float brightness,
                                                      float contrast,
                                                      volatile ssize_t *channels_left,
                                                      volatile bool *barrier_sense
                                                  ) {
    filters_brightness_contrast_data_t *data =
        malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->channels_to_process =
        channels_to_process;
    data->pixels =
        pixels;
    data->brightness =
        brightness;
    data->contrast =
        contrast;
    data->channels_left =
        channels_left;
    data->barrier_sense =
        barrier_sense;

    return data;
}

static inline void filters_brightness_contrast_data_destroy(
                       filters_brightness_contrast_data_t *data
                   )
{
    if (NULL != data) {
        free(data);
    }
}

static inline filters_sepia_data_t *filters_sepia_data_create(
                                        size_t linear_position,
                                        size_t channels_to_process,
                                        uint8_t *pixels,
                                        volatile ssize_t *channels_left,
                                        volatile bool *barrier_sense
                                   ) {
    filters_sepia_data_t *data =
        malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->channels_to_process =
        channels_to_process;
    data->pixels =
        pixels;
    data->channels_left =
        channels_left;
    data->barrier_sense =
        barrier_sense;

    return data;
}

static inline void filters_sepia_data_destroy(
                       filters_sepia_data_t *data
                   )
{
    if (NULL != data) {
        free(data);
    }
}

static inline filters_median_data_t *filters_median_data_create(
                                         size_t linear_position,
                                         size_t channels_to_process,
                                         size_t image_width,
                                         size_t image_height,
                                         uint8_t *source_pixels,
                                         uint8_t *destination_pixels,
                                         volatile ssize_t *channels_left,
                                         volatile bool *barrier_sense
                                     ) {
    filters_median_data_t *data =
        malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->channels_to_process =
        channels_to_process;
    data->image_width =
        image_width;
    data->image_height =
        image_height;
    data->source_pixels =
        source_pixels;
    data->destination_pixels =
        destination_pixels;
    data->channels_left =
        channels_left;
    data->barrier_sense =
        barrier_sense;

    return data;
}

static inline void filters_median_data_destroy(
                       filters_median_data_t *data
                   )
{
    if (NULL != data) {
        free(data);
    }
}

static void filters_brightness_contrast_processing_task(
                void *task_data,
                void (*result_callback)(void *result) __attribute__((unused))
            )
{
    filters_brightness_contrast_data_t *data =
        task_data;

    size_t linear_position =
        data->linear_position;
    size_t channels_to_process =
        data->channels_to_process;
    size_t end =
        linear_position + channels_to_process;
    uint8_t *pixels =
        data->pixels;
    float brightness =
        data->brightness;
    float contrast =
        data->contrast;
#if defined FILTERS_SIMD_ASM_IMPLEMENTATION
    size_t step =
        16;
#else
    size_t step =
        4;
#endif

    for (; linear_position < end; linear_position += step) {
        filters_apply_brightness_contrast(
            pixels, linear_position,
            brightness, contrast
        );
    }

    ssize_t channels_left = __sync_sub_and_fetch(data->channels_left, (ssize_t) channels_to_process);
    if (0 >= channels_left) {
        (void) __sync_lock_test_and_set(data->barrier_sense, true);
    }

    filters_brightness_contrast_data_destroy(data);
}

static void filters_sepia_processing_task(
                void *task_data,
                void (*result_callback)(void *result) __attribute__((unused))
            )
{
    filters_sepia_data_t *data =
        task_data;

    size_t linear_position =
        data->linear_position;
    size_t channels_to_process =
        data->channels_to_process;
    size_t end =
        linear_position + channels_to_process;
    uint8_t *pixels =
        data->pixels;
#if defined FILTERS_SIMD_ASM_IMPLEMENTATION
    size_t step =
        16;
#else
    size_t step =
        4;
#endif

    for (; linear_position < end; linear_position += step) {
        filters_apply_sepia(pixels, linear_position);
    }

    ssize_t channels_left = __sync_sub_and_fetch(data->channels_left, (ssize_t) channels_to_process);
    if (0 >= channels_left) {
        (void) __sync_lock_test_and_set(data->barrier_sense, true);
    }

    filters_sepia_data_destroy(data);
}

static void filters_median_processing_task(
                void *task_data,
                void (*result_callback)(void *result) __attribute__((unused))
            )
{
    filters_median_data_t *data =
        task_data;

    size_t linear_position =
        data->linear_position;
    size_t channels_to_process =
        data->channels_to_process;
    size_t image_height =
        data->image_height;
    size_t image_width =
        data->image_width;
    size_t end =
        linear_position + channels_to_process;
    uint8_t *source_pixels =
        data->source_pixels;
    uint8_t *destination_pixels =
        data->destination_pixels;
    size_t step =
        4;

    for (; linear_position < end; linear_position += step) {
        size_t x =
            (linear_position / 4) % image_width;
        size_t y =
            (linear_position / 4) / image_width;

        filters_apply_median(
            source_pixels,
            destination_pixels,
            linear_position,
            x, y,
            image_width, image_height
        );
    }

    ssize_t channels_left = __sync_sub_and_fetch(data->channels_left, (ssize_t) channels_to_process);
    if (0 >= channels_left) {
        (void) __sync_lock_test_and_set(data->barrier_sense, true);
    }

    filters_median_data_destroy(data);
}

