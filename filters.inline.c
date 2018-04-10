#include "filters.h"
#include "utilities.h"

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

static inline void apply_brightness_contrast_filter(
                       uint8_t *pixels,
                       size_t position,
                       float brightness,
                       float contrast
                   )
{
#if !defined BRIGHTNESS_CONTRAST_C_IMPLEMENTATION       && \
    !defined BRIGHTNESS_CONTRAST_X87_ASM_IMPLEMENTATION && \
    !defined BRIGHTNESS_CONTRAST_SIMD_ASM_IMPLEMENTATION
#define SEPIA_C_IMPLEMENTATION 1
#endif

#if defined BRIGHTNESS_CONTRAST_C_IMPLEMENTATION

    uint32_t red,
             green,
             blue;

    uint32_t current_red,
             current_green,
             current_blue;

    current_blue =
        pixels[position];
    current_green =
        pixels[position + 1];
    current_red =
        pixels[position + 2];

    blue =
        (uint32_t) (current_blue  * contrast + brightness);
    green =
        (uint32_t) (current_green * contrast + brightness);
    red =
        (uint32_t) (current_red   * contrast + brightness);

    blue =
        IPS_CLAMP(blue,  0, 255);
    green =
        IPS_CLAMP(green, 0, 255);
    red =
        IPS_CLAMP(red,   0, 255);

    pixels[position] =
        (uint8_t) blue;
    pixels[position + 1] =
        (uint8_t) green;
    pixels[position + 2] =
        (uint8_t) red;

#elif defined BRIGHTNESS_CONTRAST_X87_ASM_IMPLEMENTATION

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

#elif defined BRIGHTNESS_CONTRAST_SIMD_ASM_IMPLEMENTATION

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

static inline void apply_sepia_filter(uint8_t *pixels, size_t position)
{
    static const double Sepia_Coefficients[] = {
        0.272, 0.534, 0.131,
        0.349, 0.686, 0.168,
        0.393, 0.769, 0.189
    };

#if !defined SEPIA_C_IMPLEMENTATION &&     \
    !defined SEPIA_SIMD_ASM_IMPLEMENTATION
#define SEPIA_C_IMPLEMENTATION 1
#endif

#if defined SEPIA_C_IMPLEMENTATION

    uint32_t red,
             green,
             blue;

    uint32_t current_green,
             current_blue;

    current_blue  = pixels[position];
    current_green = pixels[position + 1];

    blue =
        (uint32_t) (Sepia_Coefficients[0] * current_blue  +
                    Sepia_Coefficients[1] * current_green +
                    Sepia_Coefficients[2] * current_blue);

    green =
        (uint32_t) (Sepia_Coefficients[3] * current_blue  +
                    Sepia_Coefficients[4] * current_green +
                    Sepia_Coefficients[5] * current_blue);

    red =
        (uint32_t) (Sepia_Coefficients[6] * current_blue  +
                    Sepia_Coefficients[7] * current_green +
                    Sepia_Coefficients[8] * current_blue);

    if (blue > 255) {
        blue = 255;
    }

    if (green > 255) {
        green = 255;
    }

    if (red > 255) {
        red = 255;
    }

    pixels[position] =
        (uint8_t) blue;
    pixels[position + 1] =
        (uint8_t) green;
    pixels[position + 2] =
        (uint8_t) red;

#elif defined SEPIA_SIMD_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    __asm__ __volatile__ (
        "addq $0x28, %%rsp\n\t" :::
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

static inline void apply_median_filter(
                       uint8_t *pixels,
                       size_t position
                   )
{
#if !defined MEDIAN_C_IMPLEMENTATION &&     \
    !defined MEDIAN_SIMD_ASM_IMPLEMENTATION
#define MEDIAN_C_IMPLEMENTATION 1
#endif

#if defined MEDIAN_C_IMPLEMENTATION

    // TBD

#elif defined MEDIAN_SIMD_ASM_IMPLEMENTATION

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

