#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bmp.h"
#include "utils.h"
#include "threadpool.h"
#include "filters_threading.h"
#include "profiler.h"

static const char IPS_Usage[] =
                    "Usage: ips "                                                       \
                        "<filter name (brightness-contrast | sepia | median)> "         \
                        "[<brightness> <contrast> for brightness and contrast filter] " \
                        "<source bitmap image file> <destination bitmap image file>",
                  IPS_Brightness_Contrast_Filter_Name[] =
                    "brightness-contrast",
                  IPS_Sepia_Filter_Name[] =
                    "sepia",
                  IPS_Median_Filter_Name[] =
                    "median",
                  IPS_Error_Illegal_Parameters[] =
                    "Illegal parameters",
                  IPS_Error_Failed_to_Open_Image[] =
                    "Failed to open the source image",
                  IPS_Error_Failed_to_Create_Image[] =
                    "Failed to create the image",
                  IPS_Error_Failed_to_Process_Image[] =
                    "Error processing the image",
                  IPS_Error_Failed_to_Create_Threadpool[] =
                    "Error trying to create a threadpool",
                  IPS_Error_Failed_to_Duplicate_the_Image[] =
                    "Error duplicating the image";

int main(int argc, char *argv[])
{
    int result =
        EXIT_FAILURE;

    int filter_id =
        -1;
    void (*task)(void *task_data, void (*result_callback)(void *result)) =
        NULL;

    char *source_file_name =
        NULL;
    char *destination_file_name =
        NULL;

    float brightness =
        0.0f;
    float contrast =
        0.0f;

    if (3 > argc) {
        fprintf(
            stderr,
            "%s\n"
            "\t%s\n",
            IPS_Error_Illegal_Parameters, IPS_Usage
        );

        return result;
    }

    if (0 == strncmp(
            argv[1],
            IPS_Brightness_Contrast_Filter_Name,
            UTILS_COUNT_OF(IPS_Brightness_Contrast_Filter_Name)
        )) {
        if (6 > argc) {
            fprintf(
                stderr,
                "%s\n"
                "\t%s\n",
                IPS_Error_Illegal_Parameters, IPS_Usage
            );

            return result;
        }

        filter_id =
            FILTERS_BRIGHTNESS_CONTRAST_ID;
        task =
            filters_brightness_contrast_processing_task;
        brightness =
            strtof(argv[2], NULL);
        contrast =
            strtof(argv[3], NULL);
        source_file_name =
            argv[4];
        destination_file_name =
            argv[5];
    } else if (0 == strncmp(
                        argv[1],
                        IPS_Sepia_Filter_Name,
                        UTILS_COUNT_OF(IPS_Sepia_Filter_Name)
                    )) {
        filter_id =
            FILTERS_SEPIA_ID;
        task =
            filters_sepia_processing_task;
        source_file_name =
            argv[2];
        destination_file_name =
            argv[3];
    } else if (0 == strncmp(
                        argv[1],
                        IPS_Median_Filter_Name,
                        UTILS_COUNT_OF(IPS_Median_Filter_Name)
                    )) {
        filter_id =
            FILTERS_MEDIAN_ID;
        task =
            filters_median_processing_task;
        source_file_name =
            argv[2];
        destination_file_name =
            argv[3];
    } else {
        fprintf(
            stderr,
            "%s\n"
            "\t%s\n",
            IPS_Error_Illegal_Parameters, IPS_Usage
        );

        return result;
    }

    bmp_image image;
    bmp_init_image_structure(&image);

    FILE *source_descriptor =
        NULL;
    FILE *destination_descriptor =
        NULL;

    source_descriptor = fopen(source_file_name, "r");
    if (NULL == source_descriptor) {
        fprintf(
            stderr,
            "%s '%s'\n",
            IPS_Error_Failed_to_Open_Image,
            source_file_name
        );

        goto cleanup;
    }

    const char *error_message;

    bmp_open_image_headers(source_descriptor, &image, &error_message);
    if (NULL != error_message) {
        fprintf(
            stderr,
            "%s '%s':\n"
            "\t%s\n",
            IPS_Error_Failed_to_Process_Image,
            source_file_name,
            error_message
        );

        goto cleanup;
    }

    bmp_read_image_data(source_descriptor, &image, &error_message);
    if (NULL != error_message) {
        fprintf(
            stderr,
            "%s '%s':\n"
            "\t%s\n",
            IPS_Error_Failed_to_Process_Image,
            source_file_name,
            error_message
        );

        goto cleanup;
    }

    destination_descriptor = fopen(destination_file_name, "w");
    if (NULL == destination_descriptor) {
        fprintf(
            stderr,
            "%s '%s'\n",
            IPS_Error_Failed_to_Create_Image,
            destination_file_name
        );

        goto cleanup;
    }

    bmp_write_image_headers(destination_descriptor, &image, &error_message);
    if (NULL != error_message) {
        fprintf(
            stderr,
            "%s '%s':\n"
            "\t%s\n",
            IPS_Error_Failed_to_Process_Image,
            destination_file_name,
            error_message
        );

        goto cleanup;
    }

    size_t pool_size = utils_get_number_of_cpu_cores() * 2;
    threadpool_t *threadpool = threadpool_create(pool_size);
    if (NULL == threadpool) {
        fprintf(
            stderr,
            "%s.\n",
            IPS_Error_Failed_to_Create_Threadpool
        );

        goto cleanup;
    }

    /* Main Image Processing Loop */
    {
        static volatile size_t rows_left =
            0;
        static volatile bool barrier_sense =
            false;
        uint8_t *pixels =
            image.pixels;

        uint8_t *original_pixels = NULL;
        if (filter_id == FILTERS_MEDIAN_ID) {
            original_pixels = (uint8_t *) malloc(image.image_size);
            if (NULL == original_pixels) {
                fprintf(
                    stderr,
                    "%s.\n",
                    IPS_Error_Failed_to_Duplicate_the_Image
                );

                goto cleanup;
            }

            memcpy(original_pixels, pixels, image.image_size);
        }

        size_t width =
            image.absolute_image_width;
        size_t height =
            image.absolute_image_height;
        size_t row_padding =
            image.pixel_row_padding;
        size_t rows_per_thread =
            height / pool_size;
        size_t linear_position_step =
            rows_per_thread * (width * 3 + row_padding);

PROFILER_START(1)
        rows_left =
            height;
        barrier_sense =
            false;

        for (
            size_t y = 0, linear_position = 0;
            y < height;
            y += rows_per_thread, linear_position += linear_position_step
        ) {
            size_t rows_to_process =
                y + rows_per_thread > height ?
                    (height - y) :
                     rows_per_thread;

            void *task_data;
            switch (filter_id) {
                case FILTERS_BRIGHTNESS_CONTRAST_ID:
                    task_data =
                        filters_brightness_contrast_data_create(
                            linear_position, row_padding,
                            0, y,
                            width,
                            rows_to_process,
                            pixels,
                            brightness, contrast,
                            &rows_left,
                            &barrier_sense
                        );
                    break;
                case FILTERS_SEPIA_ID:
                    task_data =
                        filters_sepia_data_create(
                            linear_position, row_padding,
                            0, y,
                            width,
                            rows_to_process,
                            pixels,
                            &rows_left,
                            &barrier_sense
                        );
                    break;
                case FILTERS_MEDIAN_ID:
                    task_data =
                        filters_median_data_create(
                            linear_position, row_padding,
                            0, y,
                            width,
                            rows_to_process,
                            width, height,
                            original_pixels,
                            pixels,
                            &rows_left,
                            &barrier_sense
                        );
                    break;
                default:
                    task_data =
                        NULL;
            }

            if (NULL != task_data) {
                threadpool_enqueue_task(
                    threadpool,
                    task,
                    task_data,
                    NULL
                );
            }
        }

        while (!barrier_sense) { }
PROFILER_STOP();
    }

    bmp_write_image_data(destination_descriptor, &image, &error_message);
    if (NULL != error_message) {
        fprintf(
            stderr,
            "%s '%s':\n"
            "\t%s\n",
            IPS_Error_Failed_to_Process_Image,
            destination_file_name,
            error_message
        );

        goto cleanup;
    }

    result =
        EXIT_SUCCESS;

cleanup:
    bmp_free_image_structure(&image);

    if (NULL != source_descriptor) {
        fclose(source_descriptor);
        source_descriptor = NULL;
    }

    if (NULL != destination_descriptor) {
        fclose(destination_descriptor);
        destination_descriptor = NULL;
    }

    return result;
}

