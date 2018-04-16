#ifndef BMP_H
#define BMP_H

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

static const char *BMP_Error_Invalid_File_Descriptor =
                    "Invalid file descriptor",

                  *BMP_Error_Failed_to_Read_File_Header =
                    "Failed to read the bitmap file header",
                  *BMP_Error_Invalid_File_Signature =
                    "Invalid bitmap file signature",
                  *BMP_Error_Failed_to_Read_DIB_Header =
                    "Failed to read the DIB header",
                  *BMP_Error_Unsupported_Color_Depth =
                    "Invalid color depth (not 24 bits per pixel)",
                  *BMP_Error_Invalid_Size_Information =
                    "The bitmap image containes invalid size information",

                  *BMP_Error_Invalid_Image_Structure =
                    "Invalid bitmap image structure",
                  *BMP_Error_Not_Enough_Memory_to_Read =
                    "Not enough memory to read the image",
                  *BMP_Error_Failed_to_Read_Image_Data =
                    "Failed to read the image data",
                  *BMP_Error_Invalid_Pixel_Offset_or_DIB_Header_Size =
                    "Invalid pixel offset or DIB header size",
                  *BMP_Error_Failed_to_Calculate_Padding =
                    "Failed to calculate padding information",

                  *BMP_Error_Failed_to_Write_File_Header =
                    "Failed to write the bitmap file header",
                  *BMP_Error_Failed_to_Write_DIB_Header =
                    "Failed to write the DIB header",

                  *BMP_Error_Failed_to_Write_Image_Data =
                    "Failed to write the image data";

static const int BMP_First_Magic_Byte  = 0x42,
                 BMP_Second_Magic_Byte = 0x4D;

struct _bmp_file_header
{
    uint8_t  signature[2];
    uint32_t file_size;
    uint16_t reserved[2];
    uint32_t pixel_array_offset;
} __attribute__((packed));

typedef struct _bmp_file_header bmp_file_header;

struct _bmp_dib_header
{
    uint32_t dib_header_size;
    int32_t  image_width;
    int32_t  image_height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_pixels_per_meter;
    int32_t  y_pixels_per_meter;
    uint32_t colors_in_color_table;
    uint32_t important_color_count;
} __attribute__((packed));

typedef struct _bmp_dib_header bmp_dib_header;

typedef struct _bmp_image
{
    bmp_file_header file_header;
    bmp_dib_header dib_header;
    size_t payload_size;
    uint8_t *payload;

    /* Convenience Variables */
    uint8_t *pixels;                /* start of pixel array in payload  */
    size_t absolute_image_width;    /* abs(dib_header.image_width)      */
    size_t absolute_image_height;   /* abs(dib_header.image_height)     */
    size_t pixel_row_padding;       /* the padding after each row of pixels      */
    size_t image_size;              /* the total size of the image part in bytes */
} bmp_image;

static inline void bmp_init_image_structure(bmp_image *image);
static inline void bmp_free_image_structure(bmp_image *image);

static void bmp_open_image_headers(
                FILE *file_descriptor,
                bmp_image *image,
                const char **error_message
            );

static void bmp_read_image_data(
                FILE *file_descriptor,
                bmp_image *image,
                const char **error_message
            );

static void bmp_write_image_headers(
                FILE *file_descriptor,
                bmp_image *image,
                const char **error_message
            );

static void bmp_write_image_data(
                FILE *file_descriptor,
                bmp_image *image,
                const char **error_message
            );

static inline uint8_t *bmp_sample_pixel(
                           uint8_t *pixels,
                           ssize_t x,
                           ssize_t y,
                           size_t absolute_image_width,
                           size_t absolute_image_height,
                           size_t row_padding
                       );

#include "bmp.impl.h.c"

#endif /* BMP_H */

