#include "filters_threading.h"
#include "filters.h"

#include <stdlib.h>

static inline filters_brightness_contrast_data_t *filters_brightness_contrast_data_create(
                                                      size_t linear_position,
                                                      size_t row_padding,
                                                      size_t start_x,
                                                      size_t start_y,
                                                      size_t width,
                                                      size_t height,
                                                      uint8_t *pixels,
                                                      float brightness,
                                                      float contrast,
                                                      volatile size_t *rows_left,
                                                      volatile bool *barrier_sense
                                                  ) {
    filters_brightness_contrast_data_t *data =
        malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->row_padding =
        row_padding;
    data->start_x =
        start_x;
    data->start_y =
        start_y;
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
    data->rows_left =
        rows_left;
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
                                        size_t row_padding,
                                        size_t start_x,
                                        size_t start_y,
                                        size_t width,
                                        size_t height,
                                        uint8_t *pixels,
                                        volatile size_t *rows_left,
                                        volatile bool *barrier_sense
                                   ) {
    filters_sepia_data_t *data =
        malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->row_padding =
        row_padding;
    data->start_x =
        start_x;
    data->start_y =
        start_y;
    data->width =
        width;
    data->height =
        height;
    data->pixels =
        pixels;
    data->rows_left =
        rows_left;
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
                                         size_t row_padding,
                                         size_t start_x,
                                         size_t start_y,
                                         size_t width,
                                         size_t height,
                                         size_t image_width,
                                         size_t image_height,
                                         uint8_t *source_pixels,
                                         uint8_t *destination_pixels,
                                         volatile size_t *rows_left,
                                         volatile bool *barrier_sense
                                     ) {
    filters_median_data_t *data =
        malloc(sizeof(*data));

    if (NULL == data) {
        return data;
    }

    data->linear_position =
        linear_position;
    data->row_padding =
        row_padding;
    data->start_x =
        start_x;
    data->start_y =
        start_y;
    data->width =
        width;
    data->height =
        height;
    data->image_width =
        image_width;
    data->image_height =
        image_height;
    data->source_pixels =
        source_pixels;
    data->destination_pixels =
        destination_pixels;
    data->rows_left =
        rows_left;
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
    size_t row_padding =
        data->row_padding;
    size_t start_x =
        data->start_x;
    size_t start_y =
        data->start_y;
    size_t width =
        data->width;
    size_t height =
        data->height;
    size_t end_x =
        start_x + width;
    size_t end_y =
        start_y + height;
    uint8_t *pixels =
        data->pixels;
    float brightness =
        data->brightness;
    float contrast =
        data->contrast;

    for (size_t y = start_y; y < end_y; ++y, linear_position += row_padding) {
        for (size_t x = start_x; x < end_x; ++x, linear_position += 3) {
            filters_apply_brightness_contrast(
                pixels, linear_position,
                brightness, contrast
            );
        }
    }

    size_t rows_left = __sync_sub_and_fetch(data->rows_left, height);
    if (0 == rows_left) {
        __sync_lock_test_and_set(data->barrier_sense, true);
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
    size_t row_padding =
        data->row_padding;
    size_t start_x =
        data->start_x;
    size_t start_y =
        data->start_y;
    size_t width =
        data->width;
    size_t height =
        data->height;
    size_t end_x =
        start_x + width;
    size_t end_y =
        start_y + height;
    uint8_t *pixels =
        data->pixels;

    for (size_t y = start_y; y < end_y; ++y, linear_position += row_padding) {
        for (size_t x = start_x; x < end_x; ++x, linear_position += 3) {
            filters_apply_sepia(pixels, linear_position);
        }
    }

    size_t rows_left = __sync_sub_and_fetch(data->rows_left, height);
    if (0 == rows_left) {
        __sync_lock_test_and_set(data->barrier_sense, true);
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
    size_t row_padding =
        data->row_padding;
    size_t start_x =
        data->start_x;
    size_t start_y =
        data->start_y;
    size_t height =
        data->height;
    size_t width =
        data->width;
    size_t image_height =
        data->image_height;
    size_t image_width =
        data->image_width;
    size_t end_x =
        start_x + width;
    size_t end_y =
        start_y + height;
    uint8_t *source_pixels =
        data->source_pixels;
    uint8_t *destination_pixels =
        data->destination_pixels;

    for (size_t y = start_y; y < end_y; ++y, linear_position += row_padding) {
        for (size_t x = start_x; x < end_x; ++x, linear_position += 3) {
            filters_apply_median(
                source_pixels,
                destination_pixels,
                linear_position,
                x, y,
                image_width, image_height,
                row_padding
            );
        }
    }

    size_t rows_left = __sync_sub_and_fetch(data->rows_left, height);
    if (0 == rows_left) {
        __sync_lock_test_and_set(data->barrier_sense, true);
    }

    filters_median_data_destroy(data);
}

