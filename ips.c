#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "profiler.h"

#include "bmp.h"
#include "filters.h"

static const char *IPS_Usage =
                    "Usage: ips <source bitmap image file> <destination bitmap image file>",
                  *IPS_Error_Illegal_Parameters =
                    "Illegal parameters",
                  *IPS_Error_Failed_to_Open_Image =
                    "Failed to open the source image",
                  *IPS_Error_Failed_to_Create_Image =
                    "Failed to create the image",
                  *IPS_Error_Failed_to_Process_Image =
                    "Error processing the image";

int main(int argc, char *argv[])
{
    int result = -1;

    if (argc != 3) {
        fprintf(stderr,
                 "%s\n"
                 "\t%s\n", IPS_Error_Illegal_Parameters, IPS_Usage);
    } else {
        bmp_image image;
        bmp_init_image_structure(&image);

        char *source_file_name      = argv[1],
             *destination_file_name = argv[2];

        FILE *source_descriptor      = NULL,
             *destination_descriptor = NULL;

        source_descriptor = fopen(source_file_name, "r");
        if (!source_descriptor) {
            fprintf(stderr,
                     "%s '%s'\n", IPS_Error_Failed_to_Open_Image,
                                  source_file_name);

            goto cleanup;
        }

        const char *error_message;

        bmp_open_image_headers(source_descriptor, &image, &error_message);
        if (error_message) {
            fprintf(stderr,
                     "%s '%s':\n"
                     "\t%s\n", IPS_Error_Failed_to_Process_Image,
                               source_file_name,
                               error_message);

            goto cleanup;
        }

        bmp_read_image_data(source_descriptor, &image, &error_message);
        if (error_message) {
            fprintf(stderr,
                     "%s '%s':\n"
                     "\t%s\n", IPS_Error_Failed_to_Process_Image,
                               source_file_name,
                               error_message);

            goto cleanup;
        }

        destination_descriptor = fopen(argv[2], "w");
        if (!destination_descriptor) {
            fprintf(stderr,
                     "%s '%s'\n", IPS_Error_Failed_to_Create_Image,
                                  destination_file_name);

            goto cleanup;
        }

        bmp_write_image_headers(destination_descriptor, &image, &error_message);
        if (error_message) {
            fprintf(stderr,
                     "%s '%s':\n"
                     "\t%s\n", IPS_Error_Failed_to_Process_Image,
                               destination_file_name,
                               error_message);

            goto cleanup;
        }

        /* Main Image Processing Loop */
        {
            uint8_t *pixels = image.pixels;

            size_t width   = image.absolute_image_width,
                   height  = image.absolute_image_height,
                   padding = image.pixel_row_padding;

PROFILER_START(10)
            size_t position = 0;
            for (size_t y = 0; y < height; ++y, position += padding) {
                for (size_t x = 0; x < width; ++x, position += 3) {
                    filters_apply_sepia(pixels, position);
                    // filters_apply_brightness_contrast(pixels, position...)
                    // filters_apply_median(pixels, position...)
                }
            }
PROFILER_STOP();
        }

        bmp_write_image_data(destination_descriptor, &image, &error_message);
        if (error_message) {
            fprintf(stderr,
                     "%s '%s':\n"
                     "\t%s\n", IPS_Error_Failed_to_Process_Image,
                               destination_file_name,
                               error_message);

            goto cleanup;
        }

        result = 0;

    cleanup:
        bmp_free_image_structure(&image);

        if (source_descriptor) {
            fclose(source_descriptor);
            source_descriptor = NULL;
        }

        if (destination_descriptor) {
            fclose(destination_descriptor);
            destination_descriptor = NULL;
        }
    }

    return result;
}

