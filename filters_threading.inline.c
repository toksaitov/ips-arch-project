#include "filters_threading.h"
#include "filters.h"

#include <stdlib.h>

static inline filters_brightness_contrast_data_t *filters_brightness_contrast_data_create(
                                                      size_t linear_position,
                                                      size_t row_padding,
                                                      size_t width,
                                                      size_t height,
                                                      uint8_t *pixels,
                                                      float brightness,
                                                      float contrast,
                                                      volatile size_t *rows_left_shared,
                                                      volatile bool *spinlock_sense
                                                  ) {
    filters_brightness_contrast_data_t *data =
        (filters_brightness_contrast_data_t *) malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->row_padding =
        row_padding;
    data->width =
        width;
    data->height =
        height;
    data->pixels =
        pixels;
    data->brightness =
        brightness;
    data->contrast =
        contrast;
    data->rows_left_shared =
        rows_left_shared;
    data->spinlock_sense =
        spinlock_sense;

    return data;
}

static inline void filters_brightness_contrast_data_destroy(filters_brightness_contrast_data_t *data)
{
    if (NULL != data) {
        free(data);
    }
}

static inline filters_sepia_data_t *filters_sepia_data_create(
                                        size_t linear_position,
                                        size_t row_padding,
                                        size_t width,
                                        size_t height,
                                        uint8_t *pixels,
                                        volatile size_t *rows_left_shared,
                                        volatile bool *spinlock_sense
                                   ) {
    filters_sepia_data_t *data =
        (filters_sepia_data_t *) malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->row_padding =
        row_padding;
    data->width =
        width;
    data->height =
        height;
    data->pixels =
        pixels;
    data->rows_left_shared =
        rows_left_shared;
    data->spinlock_sense =
        spinlock_sense;

    return data;
}

static inline void filters_sepia_data_destroy(filters_sepia_data_t *data)
{
    if (NULL != data) {
        free(data);
    }
}

static inline filters_median_data_t *filters_median_data_create(
                                         size_t linear_position,
                                         size_t row_padding,
                                         size_t width,
                                         size_t height,
                                         uint8_t *pixels,
                                         volatile size_t *rows_left_shared,
                                         volatile bool *spinlock_sense
                                     ) {
    filters_median_data_t *data =
        (filters_median_data_t *) malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->row_padding =
        row_padding;
    data->width =
        width;
    data->height =
        height;
    data->pixels =
        pixels;
    data->rows_left_shared =
        rows_left_shared;
    data->spinlock_sense =
        spinlock_sense;

    return data;
}

static inline void filters_median_data_destroy(filters_median_data_t *data)
{
    if (NULL != data) {
        free(data);
    }
}

static void filters_brightness_contrast_filter_task(
                void *task_data,
                void (*result_callback)(void *result)
            )
{
    filters_brightness_contrast_data_t *data =
        (filters_brightness_contrast_data_t *) task_data;

    size_t linear_position =
        data->linear_position;
    size_t row_padding =
        data->row_padding;
    size_t width =
        data->width;
    size_t height =
        data->height;
    uint8_t *pixels =
        data->pixels;
    float brightness =
        data->brightness;
    float contrast =
        data->contrast;

    for (size_t y = 0; y < height; ++y, linear_position += row_padding) {
        for (size_t x = 0; x < width; ++x, linear_position += 3) {
            filters_apply_brightness_contrast(
                pixels, linear_position,
                brightness, contrast
            );
        }
    }

    size_t rows_left_shared = __sync_sub_and_fetch(data->rows_left_shared, height);
    if (0 == rows_left_shared) {
        __sync_lock_test_and_set(data->spinlock_sense, true);
    }

    filters_brightness_contrast_data_destroy(data);
}

static void filters_sepia_filter_task(
                void *task_data,
                void (*result_callback)(void *result)
            )
{
    filters_sepia_data_t *data =
        (filters_sepia_data_t *) task_data;

    size_t linear_position =
        data->linear_position;
    size_t row_padding =
        data->row_padding;
    size_t width =
        data->width;
    size_t height =
        data->height;
    uint8_t *pixels =
        data->pixels;

    for (size_t y = 0; y < height; ++y, linear_position += row_padding) {
        for (size_t x = 0; x < width; ++x, linear_position += 3) {
            filters_apply_sepia(pixels, linear_position);
        }
    }

    size_t rows_left_shared = __sync_sub_and_fetch(data->rows_left_shared, height);
    if (0 == rows_left_shared) {
        __sync_lock_test_and_set(data->spinlock_sense, true);
    }

    filters_sepia_data_destroy(data);
}

static void filters_median_filter_task(
                void *task_data,
                void (*result_callback)(void *result)
            )
{
    filters_median_data_t *data =
        (filters_median_data_t *) task_data;

    size_t linear_position =
        data->linear_position;
    size_t row_padding =
        data->row_padding;
    size_t width =
        data->width;
    size_t height =
        data->height;
    uint8_t *pixels =
        data->pixels;

    for (size_t y = 0; y < height; ++y, linear_position += row_padding) {
        for (size_t x = 0; x < width; ++x, linear_position += 3) {
            filters_apply_median(pixels, linear_position);
        }
    }

    size_t rows_left_shared = __sync_sub_and_fetch(data->rows_left_shared, height);
    if (0 == rows_left_shared) {
        __sync_lock_test_and_set(data->spinlock_sense, true);
    }

    filters_median_data_destroy(data);
}

