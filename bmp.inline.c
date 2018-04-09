#include "bmp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void bmp_init_image_structure(bmp_image *image)
{
    if (image) {
        memset(image, 0, sizeof(*image));
    }
}

static inline void bmp_free_image_structure(bmp_image *image)
{
    if (image && image->payload) {
        free(image->payload);
        image->payload = NULL;
    }
}

static inline void bmp_open_image_headers(FILE *file_descriptor,
                                          bmp_image *image,
                                          const char **error_message)
{
    *error_message = NULL;

    if (!image) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_Image_Structure;
        }

        goto end;
    }

    if (!file_descriptor) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_File_Descriptor;
        }

        goto end;
    }

    size_t bmp_header_size =
        sizeof(image->file_header);
    size_t dib_header_size =
        sizeof(image->dib_header);
    size_t total_header_size =
        bmp_header_size + dib_header_size;

    if (!fread(&image->file_header, bmp_header_size, 1, file_descriptor)) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Read_File_Header;
        }

        goto end;
    }

    if (image->file_header.signature[0] != BMP_First_Magic_Byte ||
        image->file_header.signature[1] != BMP_Second_Magic_Byte) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_File_Signature;
        }

        goto end;
    }

    if (!fread(&image->dib_header, dib_header_size, 1, file_descriptor)) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Read_DIB_Header;
        }

        goto end;
    }

    if (image->dib_header.bits_per_pixel != 24) {
        if (error_message) {
            *error_message = BMP_Error_Unsupported_Color_Depth;
        }

        goto end;
    }

    if (image->file_header.file_size <= total_header_size) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_Size_Information;
        }

        goto end;
    }

end:
    return;
}

static inline void bmp_read_image_data(FILE *file_descriptor,
                                       bmp_image *image,
                                       const char **error_message)
{
    *error_message = NULL;

    if (!image) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_Image_Structure;
        }

        goto end;
    }

    if (!file_descriptor) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_File_Descriptor;
        }

        goto end;
    }

    size_t bmp_header_size =
        sizeof(image->file_header);
    size_t dib_header_size =
        sizeof(image->dib_header);
    size_t total_header_size =
        bmp_header_size + dib_header_size;

    size_t payload_size =
        ((size_t) image->file_header.file_size) - total_header_size;

    image->payload = (uint8_t *) malloc(payload_size);
    if (!image->payload) {
        if (error_message) {
            *error_message = BMP_Error_Not_Enough_Memory_to_Read;
        }

        goto end;
    }

    if (!fread(image->payload, payload_size, 1, file_descriptor)) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Read_Image_Data;
        }

        goto cleanup;
    }

    size_t first_pixel_index =
        ((size_t) image->file_header.pixel_array_offset) -
            (bmp_header_size + (size_t) image->dib_header.dib_header_size);

    if (first_pixel_index >= payload_size) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_Pixel_Offset_or_DIB_Header_Size;
        }

        goto cleanup;
    }

    image->pixels = &image->payload[first_pixel_index];

    size_t width = image->dib_header.image_width < 0 ?
                        (size_t) -image->dib_header.image_width :
                        (size_t)  image->dib_header.image_width;

    size_t height = image->dib_header.image_height < 0 ?
                        (size_t) -image->dib_header.image_height :
                        (size_t)  image->dib_header.image_height;

    size_t row_size = width * 3;

    size_t padding = (size_t) image->dib_header.bits_per_pixel;
    padding = (padding * width + 31) / 32 * 4 - row_size;

    image->absolute_image_width  = width;
    image->absolute_image_height = height;
    image->pixel_row_padding     = padding;

    if (height * (row_size + padding) > payload_size) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Calculate_Padding;
        }

        goto cleanup;
    }

end:
    return;

cleanup:
    if (image->payload)
    {
        free(image->payload);
        image->payload = NULL;
    }
}

static inline void bmp_write_image_headers(FILE *file_descriptor,
                                           bmp_image *image,
                                           const char **error_message)
{
    *error_message = NULL;

    if (!image) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_Image_Structure;
        }

        goto end;
    }

    if (!file_descriptor) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_File_Descriptor;
        }

        goto end;
    }

    size_t bmp_header_size =
        sizeof(image->file_header);

    if (!fwrite(&image->file_header, bmp_header_size, 1, file_descriptor)) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Write_File_Header;
        }

        goto end;
    }

    size_t dib_header_size =
        sizeof(image->dib_header);

    if (!fwrite(&image->dib_header, dib_header_size, 1, file_descriptor)) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Write_DIB_Header;
        }

        goto end;
    }

end:
    return;
}

static inline void bmp_write_image_data(FILE *file_descriptor,
                                        bmp_image *image,
                                        const char **error_message)
{
    *error_message = NULL;

    if (!image) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_Image_Structure;
        }

        goto end;
    }

    if (!file_descriptor) {
        if (error_message) {
            *error_message = BMP_Error_Invalid_File_Descriptor;
        }

        goto end;
    }

    size_t bmp_header_size =
        sizeof(image->file_header);
    size_t dib_header_size =
        sizeof(image->dib_header);
    size_t total_header_size =
        bmp_header_size + dib_header_size;

    size_t payload_size =
        ((size_t) image->file_header.file_size) - total_header_size;

    if (!fwrite(image->payload, payload_size, 1, file_descriptor)) {
        if (error_message) {
            *error_message = BMP_Error_Failed_to_Write_Image_Data;
        }

        goto end;
    }

end:
    return;
}

