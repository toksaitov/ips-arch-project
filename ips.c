#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bmp.h"
#include "threadpool.h"
#include "filters.h"
#include "filters_threading.h"
#include "profiler.h"
#include "utils.h"

static const char IPS_Usage[] =
                    "Usage: ips "                                                                     \
                        "<filter name> [brightness and contrast for brightness and contrast filter] " \
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
                    "Error trying to create a threadpool";

int main(int argc, char *argv[])
{
    int result =
        EXIT_FAILURE;

    char *source_file_name =
        NULL;
    char *destination_file_name =
        NULL;

    float brightness =
        0.0f;
    float contrast =
        0.0f;
    void (*task)(void *task_data, void (*result_callback)(void *result)) =
        NULL;

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

        task =
            filters_brightness_contrast_filter_task;
        brightness =
            strtof(argv[2], NULL);
        contrast =
            strtof(argv[3], NULL);
        source_file_name =
            argv[4],
        destination_file_name =
            argv[5];
    } else if (0 == strncmp(
                        argv[1],
                        IPS_Sepia_Filter_Name,
                        UTILS_COUNT_OF(IPS_Sepia_Filter_Name)
                    )) {
        task =
            filters_sepia_filter_task;
        source_file_name =
            argv[2],
        destination_file_name =
            argv[3];
    } else if (0 == strncmp(
                        argv[1],
                        IPS_Median_Filter_Name,
                        UTILS_COUNT_OF(IPS_Median_Filter_Name)
                    )) {
        task =
            filters_brightness_contrast_filter_task;
        source_file_name =
            argv[2],
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

    FILE *source_descriptor      = NULL,
         *destination_descriptor = NULL;

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
        static volatile size_t rows_left_shared =
            0;
        static volatile bool barrier_sense =
            false;
        uint8_t *pixels =
            image.pixels;

        size_t width =
            image.absolute_image_width;
        size_t height =
            image.absolute_image_height;
        size_t row_padding =
            image.pixel_row_padding;

PROFILER_START(1)
        rows_left_shared =
            height;
        barrier_sense =
            false;

        size_t linear_position =
            0;

        for (size_t y = 0; y < height; ++y, linear_position += (width * 3 + row_padding)) {
            void *task_data;
            if (task == filters_brightness_contrast_filter_task) {
                task_data =
                    (filters_brightness_contrast_data_t *) filters_brightness_contrast_data_create(
                                                               linear_position, row_padding,
                                                               width, 1,
                                                               pixels,
                                                               brightness, contrast,
                                                               &rows_left_shared,
                                                               &barrier_sense
                                                           );
            } else if (task == filters_sepia_filter_task) {
                task_data =
                    (filters_sepia_data_t *) filters_sepia_data_create(
                                                 linear_position, row_padding,
                                                 width, 1,
                                                 pixels,
                                                 &rows_left_shared,
                                                 &barrier_sense
                                             );
            } else {
                task_data =
                    (filters_median_data_t *) filters_median_data_create(
                                                  linear_position, row_padding,
                                                  width, 1,
                                                  pixels,
                                                  &rows_left_shared,
                                                  &barrier_sense
                                              );
            }

            threadpool_enqueue_task(threadpool, task, task_data, NULL);
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

