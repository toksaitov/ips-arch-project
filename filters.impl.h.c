#include "filters.h"
#include "utils.h"

#include <immintrin.h>

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
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
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

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subl $0x1c, %%esp\n\t"

        "xorl %%eax, %%eax\n\t"

        "movb (%2,%3), %%al\n\t"
        "movl %%eax, (%%esp)\n\t"

        "movb 0x1(%2,%3), %%al\n\t"
        "movl %%eax, 0x4(%%esp)\n\t"

        "movb 0x2(%2,%3), %%al\n\t"
        "movl %%eax, 0x8(%%esp)\n\t"

        "fildl (%%esp)\n\t"
        "fildl 0x4(%%esp)\n\t"
        "fildl 0x8(%%esp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpl 0x8(%%esp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpl 0x4(%%esp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpl (%%esp)\n\t"

        "movl $0xff, %%edx\n\t"
        "movl $0x0, %%edi\n\t"

        "movl (%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "cmpl %%edi, %%eax\n\t"
        "cmovbl %%edi, %%eax\n\t"
        "movb %%al, (%2,%3)\n\t"

        "movl 0x4(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "cmpl %%edi, %%eax\n\t"
        "cmovbl %%edi, %%eax\n\t"
        "movb %%al, 0x1(%2,%3)\n\t"

        "movl 0x8(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "cmpl %%edi, %%eax\n\t"
        "cmovbl %%edi, %%eax\n\t"
        "movb %%al, 0x2(%2,%3)\n\t"

        "addl $0x1c, %%esp\n\t"
    ::
        "S"(&brightness), "D"(&contrast),
        "b"(pixels), "c"(position)
    :
        "%eax", "%edx"
    );

#elif defined x86_64_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subq $0x38, %%rsp\n\t"

        "xorq %%rax, %%rax\n\t"

        "movb (%2,%3), %%al\n\t"
        "movq %%rax, (%%rsp)\n\t"

        "movb 0x1(%2,%3), %%al\n\t"
        "movq %%rax, 0x8(%%rsp)\n\t"

        "movb 0x2(%2,%3), %%al\n\t"
        "movq %%rax, 0x10(%%rsp)\n\t"

        "fildl (%%rsp)\n\t"
        "fildl 0x8(%%rsp)\n\t"
        "fildl 0x10(%%rsp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpq 0x10(%%rsp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpq 0x8(%%rsp)\n\t"

        "flds (%1)\n\t"
        "fmulp\n\t"
        "flds (%0)\n\t"
        "faddp\n\t"
        "fistpq (%%rsp)\n\t"

        "movq $0xff, %%rdx\n\t"
        "movq $0x0, %%r8\n\t"

        "movq (%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "cmpq %%r8, %%rax\n\t"
        "cmovbq %%r8, %%rax\n\t"
        "movb %%al, (%2,%3)\n\t"

        "movq 0x8(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "cmpq %%r8, %%rax\n\t"
        "cmovbq %%r8, %%rax\n\t"
        "movb %%al, 0x1(%2,%3)\n\t"

        "movq 0x10(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "cmpq %%r8, %%rax\n\t"
        "cmovbq %%r8, %%rax\n\t"
        "movb %%al, 0x2(%2,%3)\n\t"

        "addq $0x38, %%rsp\n\t"
    ::
        "S"(&brightness), "D"(&contrast),
        "b"(pixels), "c"(position)
    :
        "%rax", "%rdx", "%r8"
    );

#else
#error "Unsupported processor architecture"
#endif

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

#if defined INTRINSICS

    /*
       Task #1: Write the intrinsics for the SIMD assembly bellow.
    */

    // TODO

#elif defined x86_32_CPU

    // Process 16 color channels at the same time.
    __asm__ __volatile__ (
        "vbroadcastss (%0), %%zmm2\n\t"
        "vbroadcastss (%1), %%zmm1\n\t"
        "vpmovzxbd (%2,%3), %%zmm0\n\t"
        "vcvtdq2ps %%zmm0, %%zmm0\n\t"
        "vfmadd132ps %%zmm1, %%zmm2, %%zmm0\n\t"
        "vcvtps2dq %%zmm0, %%zmm0\n\t"
        "vpmovusdb %%zmm0, (%2,%3)\n\t"
    ::
        "S"(&brightness), "D"(&contrast), "b"(pixels), "c"(position)
    :
        "%zmm0", "%zmm1", "%zmm2"
    );

#elif defined x86_64_CPU

    // Process 16 color channels at the same time.
    __asm__ __volatile__ (
        "vbroadcastss (%0), %%zmm2\n\t"
        "vbroadcastss (%1), %%zmm1\n\t"
        "vpmovzxbd (%2,%3), %%zmm0\n\t"
        "vcvtdq2ps %%zmm0, %%zmm0\n\t"
        "vfmadd132ps %%zmm1, %%zmm2, %%zmm0\n\t"
        "vcvtps2dq %%zmm0, %%zmm0\n\t"
        "vpmovusdb %%zmm0, (%2,%3)\n\t"
    ::
        "S"(&brightness), "D"(&contrast), "b"(pixels), "c"(position)
    :
        "%zmm0", "%zmm1", "%zmm2"
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
#if defined FILTERS_SIMD_ASM_IMPLEMENTATION
    static const float Sepia_Coefficients[] __attribute__((aligned(0x40))) = {
        0.272f, 0.349f, 0.393f, 1.0f, 0.272f, 0.349f, 0.393f, 1.0f, 0.272f, 0.349f, 0.393f, 1.0f, 0.272f, 0.349f, 0.393f, 1.0f,
        0.534f, 0.686f, 0.769f, 1.0f, 0.534f, 0.686f, 0.769f, 1.0f, 0.534f, 0.686f, 0.769f, 1.0f, 0.534f, 0.686f, 0.769f, 1.0f,
        0.131f, 0.168f, 0.189f, 1.0f, 0.131f, 0.168f, 0.189f, 1.0f, 0.131f, 0.168f, 0.189f, 1.0f, 0.131f, 0.168f, 0.189f, 1.0f
    };
#else
    static const float Sepia_Coefficients[] = {
        0.272f, 0.534f, 0.131f,
        0.349f, 0.686f, 0.168f,
        0.393f, 0.769f, 0.189f
    };
#endif

#if !defined FILTERS_C_IMPLEMENTATION &&     \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

#if defined FILTERS_C_IMPLEMENTATION

    uint32_t blue =
        pixels[position];
    uint32_t green =
        pixels[position + 1];
    uint32_t red =
        pixels[position + 2];

    pixels[position] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[0] * blue  +
                      Sepia_Coefficients[1] * green +
                      Sepia_Coefficients[2] * red,
                      255.0f
                  );
    pixels[position + 1] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[3] * blue  +
                      Sepia_Coefficients[4] * green +
                      Sepia_Coefficients[5] * red,
                      255.0f
                  );
    pixels[position + 2] =
        (uint8_t) UTILS_MIN(
                      Sepia_Coefficients[6] * blue  +
                      Sepia_Coefficients[7] * green +
                      Sepia_Coefficients[8] * red,
                      255.0f
                  );

#elif defined FILTERS_X87_ASM_IMPLEMENTATION

#if defined x86_32_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subl $0xC, %%esp\n\t"

        "xorl %%eax, %%eax\n\t"

        "movb (%1,%2), %%al\n\t"
        "movl %%eax, (%%esp)\n\t"

        "movb 0x1(%1,%2), %%al\n\t"
        "movl %%eax, 0x4(%%esp)\n\t"

        "movb 0x2(%1,%2), %%al\n\t"
        "movl %%eax, 0x8(%%esp)\n\t"

        "filds (%%esp)\n\t"
        "filds 0x4(%%esp)\n\t"
        "filds 0x8(%%esp)\n\t"

        "flds (%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x4(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x8(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl (%%esp)\n\t"

        "flds 0xc(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x10(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x14(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0x4(%%esp)\n\t"

        "flds 0x18(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x1c(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x20(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpl 0x8(%%esp)\n\t"

        "fstp %%st\n\t"
        "fstp %%st\n\t"
        "fstp %%st\n\t"

        "movl $0xff, %%edx\n\t"

        "movl (%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, (%1,%2)\n\t"

        "movl 0x4(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, 0x1(%1,%2)\n\t"

        "movl 0x8(%%esp), %%eax\n\t"
        "cmpl %%edx, %%eax\n\t"
        "cmoval %%edx, %%eax\n\t"
        "movb %%al, 0x2(%1,%2)\n\t"

        "addl $0xC, %%esp\n\t"
    ::
        "S"(Sepia_Coefficients),
        "b"(pixels), "c"(position)
    :
        "%eax", "%edx"
    );

#elif defined x86_64_CPU

    // Similar, but not a one to one conversion of the C code above. Try to find in what way.
    __asm__ __volatile__ (
        "subq $0x18, %%rsp\n\t"

        "xorq %%rax, %%rax\n\t"

        "movb (%1,%2), %%al\n\t"
        "movq %%rax, (%%rsp)\n\t"

        "movb 0x1(%1,%2), %%al\n\t"
        "movq %%rax, 0x8(%%rsp)\n\t"

        "movb 0x2(%1,%2), %%al\n\t"
        "movq %%rax, 0x10(%%rsp)\n\t"

        "fildl (%%rsp)\n\t"
        "fildl 0x8(%%rsp)\n\t"
        "fildl 0x10(%%rsp)\n\t"

        "flds (%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x4(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x8(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq (%%rsp)\n\t"

        "flds 0xc(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x10(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x14(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x8(%%rsp)\n\t"

        "flds 0x18(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x1c(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "flds 0x20(%0)\n\t"
        "fmul %%st(3), %%st\n\t"
        "faddp\n\t"
        "faddp\n\t"
        "fistpq 0x10(%%rsp)\n\t"

        "fstp %%st\n\t"
        "fstp %%st\n\t"
        "fstp %%st\n\t"

        "movq $0xff, %%rdx\n\t"

        "movq (%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, (%1,%2)\n\t"

        "movq 0x8(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, 0x1(%1,%2)\n\t"

        "movq 0x10(%%rsp), %%rax\n\t"
        "cmpq %%rdx, %%rax\n\t"
        "cmovaq %%rdx, %%rax\n\t"
        "movb %%al, 0x2(%1,%2)\n\t"

        "addq $0x18, %%rsp\n\t"
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

#if defined INTRINSICS

    // Process 16 color channels at the same time.
    __m512 coeff1 = _mm512_load_ps(&Sepia_Coefficients[0]);
    __m512 coeff2 = _mm512_load_ps(&Sepia_Coefficients[16]);
    __m512 coeff3 = _mm512_load_ps(&Sepia_Coefficients[32]);
    __m512i ints = _mm512_cvtepu8_epi32(_mm_load_si128((__m128i *) &pixels[position]));
    __m512 floats = _mm512_cvtepi32_ps(ints);
    __m512 temp1 = floats;
    __m512 temp2 = floats;
    __m512 temp3 = floats;
    temp1 = _mm512_permute_ps(temp1, 0b11000000);
    temp2 = _mm512_permute_ps(temp2, 0b11010101);
    temp3 = _mm512_permute_ps(temp3, 0b11101010);
    floats = _mm512_mul_ps(coeff1, temp1);
    floats = _mm512_fmadd_ps(coeff2, temp2, floats);
    floats = _mm512_fmadd_ps(coeff3, temp3, floats);
    ints = _mm512_cvtps_epi32(floats);
    _mm512_mask_cvtusepi32_storeu_epi8(&pixels[position], 0xffff, ints);

#elif defined x86_32_CPU

    /*
       Task #2: Write the inline assembly representation of the intrinsics above.
    */

    // TODO

#elif defined x86_64_CPU

    /*
       Task #2: Write the inline assembly representation of the intrinsics above.
    */

    // TODO

#else
#error "Unsupported processor architecture"
#endif

#endif
}

#if defined FILTERS_SIMD_ASM_IMPLEMENTATION

static const size_t window_width =
    3;
static const size_t window_height =
    3;
static const size_t window_center_shift_x =
    1;
static const size_t window_center_shift_y =
    1;
static const size_t window_size =
    8;
static const size_t window_center =
    4;

static const uint64_t permutations_1[] __attribute__((aligned(0x40))) = {
    1, 0, 3, 2, 5, 4, 7, 6
};
static const uint64_t permutations_2[] __attribute__((aligned(0x40))) = {
    3, 2, 1, 0, 7, 6, 5, 4
};
static const uint64_t permutations_3[] __attribute__((aligned(0x40))) = {
    7, 6, 5, 4, 3, 2, 1, 0
};
static const uint64_t permutations_4[] __attribute__((aligned(0x40))) = {
    2, 3, 0, 1, 6, 7, 4, 5
};

#endif

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
                       size_t height
                   )
{
#if !defined FILTERS_C_IMPLEMENTATION &&     \
    !defined FILTERS_SIMD_ASM_IMPLEMENTATION
#define FILTERS_C_IMPLEMENTATION 1
#endif

#if defined FILTERS_C_IMPLEMENTATION

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
                        height
                    )[channel];
            }
        }

        qsort(window, window_size, sizeof(*window), _filters_compare_color_channels);

#elif defined FILTERS_SIMD_ASM_IMPLEMENTATION

    double window[9] __attribute__((aligned(0x40)));
    for (size_t channel = 0; channel < 3; ++channel) {
        for (size_t wy = 0; wy < window_height; ++wy) {
            for (size_t wx = 0; wx < window_width; ++wx) {
                ssize_t adjusted_x =
                    (ssize_t) x - (ssize_t) window_center_shift_x + (ssize_t) wx;
                ssize_t adjusted_y =
                    (ssize_t) y - (ssize_t) window_center_shift_y + (ssize_t) wy;

                window[wy * window_width + wx] =
                    (double) bmp_sample_pixel(
                                 source_pixels,
                                 adjusted_x,
                                 adjusted_y,
                                 width,
                                 height
                             )[channel];
            }
        }

#if defined INTRINSICS

        __m512d result = _mm512_load_pd(window);

        {
            __m512i idxNoNeigh   = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
            __m512d permNeigh    = _mm512_permutexvar_pd(idxNoNeigh, result);
            __m512d permNeighMin = _mm512_min_pd(permNeigh, result);
            __m512d permNeighMax = _mm512_max_pd(permNeigh, result);

            result = _mm512_mask_mov_pd(permNeighMin, 0xAA, permNeighMax);
        }

        {
            __m512i idxNoNeigh   = _mm512_set_epi64(4, 5, 6, 7, 0, 1, 2, 3);
            __m512d permNeigh    = _mm512_permutexvar_pd(idxNoNeigh, result);
            __m512d permNeighMin = _mm512_min_pd(permNeigh, result);
            __m512d permNeighMax = _mm512_max_pd(permNeigh, result);

            result = _mm512_mask_mov_pd(permNeighMin, 0xCC, permNeighMax);
        }

        {
            __m512i idxNoNeigh   = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
            __m512d permNeigh    = _mm512_permutexvar_pd(idxNoNeigh, result);
            __m512d permNeighMin = _mm512_min_pd(permNeigh, result);
            __m512d permNeighMax = _mm512_max_pd(permNeigh, result);

            result = _mm512_mask_mov_pd(permNeighMin, 0xAA, permNeighMax);
        }

        {
            __m512i idxNoNeigh   = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
            __m512d permNeigh    = _mm512_permutexvar_pd(idxNoNeigh, result);
            __m512d permNeighMin = _mm512_min_pd(permNeigh, result);
            __m512d permNeighMax = _mm512_max_pd(permNeigh, result);

            result = _mm512_mask_mov_pd(permNeighMin, 0xF0, permNeighMax);
        }

        {
            __m512i idxNoNeigh   = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
            __m512d permNeigh    = _mm512_permutexvar_pd(idxNoNeigh, result);
            __m512d permNeighMin = _mm512_min_pd(permNeigh, result);
            __m512d permNeighMax = _mm512_max_pd(permNeigh, result);

            result = _mm512_mask_mov_pd(permNeighMin, 0xCC, permNeighMax);
        }

        {
            __m512i idxNoNeigh   = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
            __m512d permNeigh    = _mm512_permutexvar_pd(idxNoNeigh, result);
            __m512d permNeighMin = _mm512_min_pd(permNeigh, result);
            __m512d permNeighMax = _mm512_max_pd(permNeigh, result);

            result = _mm512_mask_mov_pd(permNeighMin, 0xAA, permNeighMax);
        }

        _mm512_store_pd(window, result);

#elif defined x86_32_CPU

        __asm__ __volatile__ (
            "vmovapd (%0), %%zmm1\n\t"

            "vmovdqa64 (%1), %%zmm3\n\t"
            "vmovdqa64 (%2), %%zmm4\n\t"
            "vmovdqa64 (%3), %%zmm5\n\t"
            "kmovw %%eax, %%k1\n\t"
            "movl  $-52, %%eax\n\t"
            "kmovw %%eax, %%k2\n\t"
            "movl  $-16, %%eax\n\t"
            "kmovw %%eax, %%k3\n\t"

            "vpermpd %%zmm1, %%zmm3, %%zmm2\n\t"
            "vminpd  %%zmm1, %%zmm2, %%zmm0\n\t"
            "vmaxpd  %%zmm1, %%zmm2, %%zmm0%{%%k1%}\n\t"

            "vpermpd %%zmm0, %%zmm4, %%zmm2\n\t"
            "vminpd  %%zmm0, %%zmm2, %%zmm1\n\t"
            "vmaxpd  %%zmm0, %%zmm2, %%zmm1%{%%k2%}\n\t"

            "vpermpd %%zmm1, %%zmm3, %%zmm2\n\t"
            "vminpd  %%zmm1, %%zmm2, %%zmm0\n\t"
            "vmaxpd  %%zmm1, %%zmm2, %%zmm0%{%%k1%}\n\t"

            "vpermpd %%zmm0, %%zmm5, %%zmm2\n\t"
            "vminpd  %%zmm0, %%zmm2, %%zmm1\n\t"
            "vmaxpd  %%zmm0, %%zmm2, %%zmm1%{%%k3%}\n\t"

            "vpermpd %%zmm1, %%zmm6, %%zmm2\n\t"
            "vminpd  %%zmm1, %%zmm2, %%zmm0\n\t"
            "vmaxpd  %%zmm1, %%zmm2, %%zmm0%{%%k2%}\n\t"

            "vpermpd %%zmm0, %%zmm3, %%zmm2\n\t"
            "vminpd  %%zmm0, %%zmm2, %%zmm1\n\t"
            "vmaxpd  %%zmm0, %%zmm2, %%zmm1%{%%k1%}\n\t"

            "vmovapd %%zmm1, (%0)\n\t"
        ::
            "b"(window),
            "c"(permutations_1),
            "d"(permutations_2),
            "S"(permutations_3),
            "D"(permutations_4)
        :
            "%rax",
            "k1", "k2", "k3",
            "%zmm0", "%zmm1", "%zmm2", "%zmm3", "%zmm5", "%zmm6", "%zmm7"
        );

#elif defined x86_64_CPU

        __asm__ __volatile__ (
            "vmovapd %0, %%zmm1\n\t"

            "vmovdqa64 %1, %%zmm3\n\t"
            "vmovdqa64 %2, %%zmm4\n\t"
            "vmovdqa64 %3, %%zmm5\n\t"
            "vmovdqa64 %4, %%zmm6\n\t"

            "movl  $-86, %%eax\n\t"
            "kmovw %%eax, %%k1\n\t"
            "movl  $-52, %%eax\n\t"
            "kmovw %%eax, %%k2\n\t"
            "movl  $-16, %%eax\n\t"
            "kmovw %%eax, %%k3\n\t"

            "vpermpd %%zmm1, %%zmm3, %%zmm2\n\t"
            "vminpd  %%zmm1, %%zmm2, %%zmm0\n\t"
            "vmaxpd  %%zmm1, %%zmm2, %%zmm0%{%%k1%}\n\t"

            "vpermpd %%zmm0, %%zmm4, %%zmm2\n\t"
            "vminpd  %%zmm0, %%zmm2, %%zmm1\n\t"
            "vmaxpd  %%zmm0, %%zmm2, %%zmm1%{%%k2%}\n\t"

            "vpermpd %%zmm1, %%zmm3, %%zmm2\n\t"
            "vminpd  %%zmm1, %%zmm2, %%zmm0\n\t"
            "vmaxpd  %%zmm1, %%zmm2, %%zmm0%{%%k1%}\n\t"

            "vpermpd %%zmm0, %%zmm5, %%zmm2\n\t"
            "vminpd  %%zmm0, %%zmm2, %%zmm1\n\t"
            "vmaxpd  %%zmm0, %%zmm2, %%zmm1%{%%k3%}\n\t"

            "vpermpd %%zmm1, %%zmm6, %%zmm2\n\t"
            "vminpd  %%zmm1, %%zmm2, %%zmm0\n\t"
            "vmaxpd  %%zmm1, %%zmm2, %%zmm0%{%%k2%}\n\t"

            "vpermpd %%zmm0, %%zmm3, %%zmm2\n\t"
            "vminpd  %%zmm0, %%zmm2, %%zmm1\n\t"
            "vmaxpd  %%zmm0, %%zmm2, %%zmm1%{%%k1%}\n\t"

            "vmovapd %%zmm1, %0\n\t"
        :
            "+m"(*window)
        :
            "m"(*permutations_1), "m"(*permutations_2), "m"(*permutations_3), "m"(*permutations_4)
        :
            "rax",
            "k1", "k2", "k3",
            "zmm0", "zmm1", "zmm2", "zmm3", "zmm5", "zmm6",
            "cc"
        );

#else
#error "Unsupported processor architecture"
#endif

#endif

        uint8_t median =
            window_size % 2 == 0 ?
                (uint8_t) ((window[window_center - 1] + window[window_center]) * 0.5) :
                (uint8_t) window[window_center];

        destination_pixels[position + channel] =
            median;
    }
}

