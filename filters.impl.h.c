#include "filters.h"
#include "utils.h"

/*
    // AT&T/UNIX GCC Inline Assembly Sample

    static const char Argument[] =                   // C constants
        "some data";
    static const unsigned long Another_Argument =
        sizeof(Argument);
    long result;                                     // a C variable

    // x86, x86-64
    __asm__ __volatile__ (
        "op-code<length suffix> %%src_register, %%dest_register\n\t"
        "op-code<length suffix> $immediate, %%dest_register\n\t"
        // ...
        "op-code<length suffix> %<argument number>, %%dest_register\n\t"
        "op-code"
        : "=a" (result)                              // output argument/s
        : "D" ((unsigned long) file_descriptor),     // input arguments
            "S" (buffer),
            "d" (buffer_size),
            "r" (Argument), "r" (Another_Argument)
        : "%used register", "%another used register" // clobbered registers
    );

    // The ARM assembly syntax uses the `#` symbol for constants and NOT
    // the `$` symbol. Registers `r` or `x` (for the 32-bit or 64-bit
    // architecture) do not need a `%%` prefix.
    //
    // `__asm__` and `__volatile__` could also be written as `asm` and `volatile`.
    //
    // The `volatile` modifier tells the compiler not to remove or reorder
    // the inlined assembly block during the compiler optimization step.
    //
    // <length suffixes>
    //     'b'    'w'     's'     'l'     'q'
    //      8 bit  16 bit  16 bit  32 bit  64 bit  integers
    //                     32 bit  64 bit          floating point numbers
    //
    // Length suffixes are not required for the ARM assembly syntax.
    // Argument numbers go from top to bottom, from left to right
    // starting from zero.
    //
    //     result           : %<argument number> = %0
    //     file_descriptor  : ...                = %1
    //     buffer           :                    = %2
    //     buffer_size      :                    = %3
    //     Argument         :                    = %4
    //     Another_Argument :                    = %5
    //
    // The first quoted letter before the argument in brackets is a
    // register constraint. It tells the compiler to provide the
    // argument through that register.
    //
    // On X86/-64 the following register constraints are possible
    // +---+--------------------------+
    // | r :   any register           |
    // +---+--------------------------+
    // | a :   %rax, %eax, %ax, %al   |
    // | b :   %rbx, %ebx, %bx, %bl   |
    // | c :   %rcx, %ecx, %cx, %cl   |
    // | d :   %rdx, %edx, %dx, %dl   |
    // | S :   %rsi, %esi, %si        |
    // | D :   %rdi, %edi, %di        |
    // +---+--------------------------+
    //
    // On ARM, the `r` constraint will work for all general purpose
    // registers. The input variable's register can be specified after the
    // variable's declaration wrapped in quotes and parentheses.
    //
    //     register long result ("r7"); // 32-bit ARM
    //     register long result ("x0"); // 64-bit ARM
    //
    // All registers used as input or output arguments should not be
    // listed as clobbered.
    //
    // https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
*/

static inline void filters_apply_brightness_contrast(
                       uint8_t *pixels,
                       size_t position,
                       float brightness,
                       float contrast
                   )
{
#if !defined FILTERS_C_IMPLEMENTATION       && \
    !defined FILTERS_X87_ASM_IMPLEMENTATION && \
    !defined FILTERS_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

#if defined FILTERS_C_IMPLEMENTATION

    pixels[position] =
        (uint8_t) UTILS_CLAMP(pixels[position]     * contrast + brightness, 0.0f, 255.0f);
    pixels[position + 1] =
        (uint8_t) UTILS_CLAMP(pixels[position + 1] * contrast + brightness, 0.0f, 255.0f);
    pixels[position + 2] =
        (uint8_t) UTILS_CLAMP(pixels[position + 2] * contrast + brightness, 0.0f, 255.0f);

#elif defined FILTERS_X87_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#elif defined x86_64_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#else
#error "Unsupported processor architecture"
#endif

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#elif defined x86_64_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#else
#error "Unsupported processor architecture"
#endif

#endif
}

static inline void filters_apply_sepia(
                       uint8_t *pixels,
                       size_t position
                   )
{
    static const float Sepia_Coefficients[] = {
        0.272f, 0.534f, 0.131f,
        0.349f, 0.686f, 0.168f,
        0.393f, 0.769f, 0.189f
    };

#if !defined FILTERS_C_IMPLEMENTATION &&     \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

#if defined FILTERS_C_IMPLEMENTATION

    uint32_t blue =
        pixels[position];
    uint32_t green =
        pixels[position + 1];

    pixels[position] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[0] * blue  +
                      Sepia_Coefficients[1] * green +
                      Sepia_Coefficients[2] * blue,
                      255.0f
                  );
    pixels[position + 1] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[3] * blue  +
                      Sepia_Coefficients[4] * green +
                      Sepia_Coefficients[5] * blue,
                      255.0f
                  );
    pixels[position + 2] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[6] * blue  +
                      Sepia_Coefficients[7] * green +
                      Sepia_Coefficients[8] * blue,
                      255.0f
                  );

#elif defined FILTERS_X87_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subl $0x14, %%esp\n\t"

        "xorl %%eax, %%eax\n\t"
        "movl $0xff, %%edx\n\t"

        "movb (%1,%2), %%al\n\t"
        "movl %%eax, (%%esp)\n\t"

        "movb 0x1(%1,%2), %%al\n\t"
        "movl %%eax, 0x4(%%esp)\n\t"

        "filds (%%esp)\n\t"
        "filds 0x4(%%esp)\n\t"

        "fldl (%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x8(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x10(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0x8(%%esp)\n\t"

        "fldl 0x18(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x20(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x28(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0xc(%%esp)\n\t"

        "fldl 0x30(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x38(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x40(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0x10(%%esp)\n\t"

        "fstp %%st\n\t"
        "fstp %%st\n\t"

        "movl 0x8(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, (%1,%2)\n\t"

        "movl 0xc(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, 0x1(%1,%2)\n\t"

        "movl 0x10(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, 0x2(%1,%2)\n\t"

        "addl $0x14, %%esp\n\t"
    ::
        "S"(Sepia_Coefficients),
        "b"(pixels), "c"(position)
    :
        "%eax", "%edx"
    );

#elif defined x86_64_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subq $0x28, %%rsp\n\t"

        "xorq %%rax, %%rax\n\t"
        "movq $0xff, %%rdx\n\t"

        "movb (%1,%2), %%al\n\t"
        "movq %%rax, (%%rsp)\n\t"

        "movb 0x1(%1,%2), %%al\n\t"
        "movq %%rax, 0x8(%%rsp)\n\t"

        "fildl (%%rsp)\n\t"
        "fildl 0x8(%%rsp)\n\t"

        "fldl (%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x8(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x10(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x10(%%rsp)\n\t"

        "fldl 0x18(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x20(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x28(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x18(%%rsp)\n\t"

        "fldl 0x30(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x38(%0)\n\t"
        "fmul %%st(2), %%st\n\t"
        "fldl 0x40(%0)\n\t"
        "fmul %%st(4), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x20(%%rsp)\n\t"

        "fstp %%st\n\t"
        "fstp %%st\n\t"

        "movq 0x10(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, (%1,%2)\n\t"

        "movq 0x18(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, 0x1(%1,%2)\n\t"

        "movq 0x20(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, 0x2(%1,%2)\n\t"

        "addq $0x28, %%rsp\n\t"
    ::
        "S"(Sepia_Coefficients),
        "b"(pixels), "c"(position)
    :
        "%rax", "%rdx"
    );

#else
#error "Unsupported processor architecture"
#endif

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#elif defined x86_64_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#else
#error "Unsupported processor architecture"
#endif

#endif
}

static int _filters_compare_color_channels(const void *a, const void *b)
{
    return *((const uint8_t *) a) - *((const uint8_t *) b);
}

static inline void filters_apply_median(
                       uint8_t *source_pixels,
                       uint8_t *destination_pixels,
                       size_t position,
                       size_t x,
                       size_t y,
                       size_t width,
                       size_t height,
                       size_t padding
                   )
{
#if !defined FILTERS_C_IMPLEMENTATION &&     \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

    const size_t window_width =
        FILTERS_MEDIAN_WINDOW_SIZE % 2 == 0 ?
            FILTERS_MEDIAN_WINDOW_SIZE + 1 :
            FILTERS_MEDIAN_WINDOW_SIZE;
    const size_t window_height =
        window_width;
    const size_t window_center_shift_x =
        window_width / 2;
    const size_t window_center_shift_y =
        window_height / 2;
    const size_t window_size =
        window_width * window_height;
    const size_t window_center =
        window_size / 2;

    uint8_t window[window_size];
    for (size_t channel = 0; channel < 3; ++channel) {
        for (size_t wy = 0; wy < window_height; ++wy) {
            for (size_t wx = 0; wx < window_width; ++wx) {
                ssize_t adjusted_x =
                    (ssize_t) x - (ssize_t) window_center_shift_x + (ssize_t) wx;
                ssize_t adjusted_y =
                    (ssize_t) y - (ssize_t) window_center_shift_y + (ssize_t) wy;

                window[wy * window_width + wx] =
                    bmp_sample_pixel(
                        source_pixels,
                        adjusted_x,
                        adjusted_y,
                        width,
                        height,
                        padding
                    )[channel];
            }
        }

#if defined FILTERS_C_IMPLEMENTATION

        qsort(window, window_size, sizeof(*window), _filters_compare_color_channels);

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#elif defined x86_64_CPU

    __asm__ __volatile__ (
        "\n\t" :::
    );

#else
#error "Unsupported processor architecture"
#endif

#endif

        uint8_t median =
            window_size % 2 == 0 ?
                (uint8_t) ((window[window_center - 1] + window[window_center]) * 0.5f) :
                window[window_center];
        destination_pixels[position + channel] =
            median;
    }
}

