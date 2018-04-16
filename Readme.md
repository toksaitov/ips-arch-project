Course Project (Draft)
======================

![Sample Source Image](https://i.imgur.com/40Bvuur.png)

## Part #1, Brightness and Contrast Adjustments with x87 Instructions

In this part of the project, you will have to implement a simple brightness and
contrast adjustment filter for a 2D image in assembly. You should start by
writing an implementation by using the x87 floating-point instructions. The
brightness adjustment requires using a simple addition operation. The contrast
adjustment requires using multiplication. The final calculated value should be
converted back to an integer color channel value.

![Brightness and Contrast Sample](https://i.imgur.com/ezN6oDV.png)

The overall formula is

```C
// ...
channel_value = (unsigned char) (channel_value * contrast + brightness);
```

The image is represented as an array of pixel color values. Every pixel is
represented as three-byte integers with channel values ranging from 0 to 255.
You test image is stored in the BMP/DIB format. Your code template provides a
simple library to read, decode, and write some variants of bitmap images. Access
to the image data array, unfortunately, you need to handle on your own.  You can
refer to the following image (courtesy to [Verpies](https://commons.wikimedia.org/wiki/File:BMPfileFormat.png))
to understand how you can work with bitmap data.

![BMP Image Structure](https://i.imgur.com/CKrcD9u.png)

### Tasks

1. Use the provided template, and specifically modify the
   `filters_apply_brightness_contrast(...)` function by inserting inline GCC
   assembly to your code. If you do not like GCC inline assembly, you can create
   a separate assembly `.s` file, modify the build rules in `Makefile` to
   assemble the file separately and then to link it with the other `.o` files
   from C. Do not forget to perform a call to your assembly function from the C
   file.

2. Benchmark your solution, by compiling and running `make profile`.

3. If everything works, and you got a speed-up, write a
   `Readme_Student_Part_1.md` file. Throughout your work, you should be keeping
   notes about your development process. What I would like to see is: how you
   implemented your functionality, what kind of speed-up you got, and challenges
   you experienced. Here, you can also clearly document your reference
   materials.

## Part #2, Brightness and Contrast Adjustments with SIMD AVX-512 Instructions

In this part, you need to accelerate your assembly solution by utilizing the
AVX-512 SIMD instructions and registers ZMM0-ZMM31. Vector registers allow you
to perform one instruction on multiple parts of one register at the same time in
parallel. Your task is to pack up to 16 floating-point value into one register
for up to 5 pixels and perform multiplication and addition on all of them at the
same time. Then store the result back to memory.

### Tasks

1. Use the provided template, and specifically modify the
   `filters_apply_brightness_contrast(...)` function by inserting inline GCC
   assembly to your code. If you do not like GCC inline assembly, you can create
   a separate assembly `.s` file, modify the build rules in `Makefile` to
   assemble the file separately and then to link it with the other `.o` files
   from C. Do not forget to perform a call to your assembly function from the C
   file.

2. Benchmark your solution, by compiling and running `make profile`.

3. If everything works, and you got a speed-up, write a
   `Readme_Student_Part_2.md` file. Throughout your work, you should be keeping
   notes about your development process. What I would like to see is: how you
   implemented your functionality, what kind of speed-up you got, and challenges
   you experienced. Here, you can also clearly document your reference
   materials.

## Part #3, The Sepia Filter

In this part, you need to accelerate a C function that applies the Sepia filter.
The Sepia filter converts a color image to a duotone image with a dark
Brown-Gray color as explained in the following [paper](https://software.intel.com/en-us/articles/image-processing-acceleration-techniques-using-intel-streaming-simd-extensions-and-intel-advanced-vector-extensions)
written by Petter Larsson and Eric Palmer. You can also find out the weights to
use for the three color channels to apply the filter. Use the same approach to
speed-up the solution as in the second part of the project.

![The Sepia Filter](https://i.imgur.com/bKsDknj.png)

### Tasks

1. Find the coefficients you need to use in your work in the paper.

2. Use the provided template, and specifically modify the
   `filters_apply_sepia(...)` function by inserting inline GCC assembly to your
   code. If you do not like GCC inline assembly, you can create a separate
   assembly `.s` file, modify the build rules in `Makefile` to assemble the file
   separately and then to link it with the other `.o` files from C. Do not
   forget to perform a call to your assembly function from the C file.

3. Benchmark your solution, by compiling and running `make profile`.

4. If everything works, and you got a speed-up, write a
   `Readme_Student_Part_3.md` file. Throughout your work, you should be keeping
   notes about your development process. What I would like to see is: how you
   implemented your functionality, what kind of speed-up you got, and challenges
   you experienced. Here, you can also clearly document your reference
   materials.

## Part 4, The Median Filter

In this part of the project, you will have to accelerate execution of an image
processing filter, called a median filter.  The median filter is used to remove
noise from a signal (e.g., a 1D audio signal or a 2D image data signal). It does
so by replacing each pixel value (or channel value) with a median of neighboring
entries. The list of neighboring entries is called a window. The window slides,
entry by entry, over the signal. For 1D signal, the window is just a list of
values to the left and to the right of the data point being filtered. For 2D
signals, the window can be represented by different shapes to get different
filtering results. For example, the shape could be a box or a cross that is
centered over the pixel (or channel) being processed. Note that median is the
middle value separating the greater and lesser halves of a sorted data set.

As you can understand, sorting pixel values of a sliding window is a probable
bottleneck which requires considerable computational power, especially for large
window sizes. In this part of the project, your job is to accelerate the sorting
in the C-based implementation of the filter by replacing it (or extending it)
with inline assembly code. The code should implement the sorting entirely in
AVX-512 registers (ZMM0-ZMM31) by using modern x86 SIMD instructions.
Unfortunately, the AVX-512 is only available in Intel CPUs released from 2016.
It means your home machine may not support those instructions. To be able to
test your solution you should visit our labs G30 and G31 that have machines with
Intel Core i7-7700K CPUs that support AVX-512 instructions.

SIMD instructions can accelerate your code if you can parallelize your
algorithm. Unfortunately, not all sorting algorithms can be efficiently
parallelized. To deal with the problem, you have to implement a sorting network
algorithm, called bitonic sort. Your task will be to study and understand the
idea behind sorting networks and a certain implementation of the bitonic sort
adapted to work with AVX registers on your own from the following research
[paper](https://arxiv.org/pdf/1704.08579.pdf) written by Berenger Bramas. You
have to read the whole paper, understand it, and find out the necessary bits
that you need to use in your work. Note, that you only need to implement the
bitonic sort without merging for a box window of up to 4 by 4 channel values to
get a full grade.

![The Median Filter](https://i.imgur.com/x2gXSpe.png)

### Tasks

1. Read the paper, try to understand it, find out the bits that you need to use
   in your work.

2. Use the provided template, and specifically modify the
   `filters_apply_median(...)` function by inserting inline GCC assembly to
   accelerate your code. If you do not like GCC inline assembly, you can create
   a separate assembly `.s` file, modify the build rules in `Makefile` to
   assemble the file separately and then to link it with the other `.o` files
   from C. Do not forget to perform a call to your assembly function from the C
   file.

3. Benchmark your solution, by compiling and running `make profile`.

4. If everything works, and you got a speed-up, write a
   `Readme_Student_Part_4.md` file. Throughout your work, you should be keeping
   notes about your development process. What I would like to see is: how you
   implemented your functionality, what kind of speed-up you got, and challenges
   you experienced. Here, you can also clearly document your reference
   materials.

### Extra Tasks

1. Allow having a sliding window bigger than 4 by 4. For that, you need to
   implement the merging of multiple registers as described in the paper. This
   is not a trivial task. Thus it will be rewarded with 10 extra points.

2. If everything works, and you got a speed-up, extend the
   `Readme_Student_Part_4.md` file with information about your extra work.

## Compilation and Usage

1. Get access to a Linux machine running on a Skylake CPU. You can use lab
   machines or a dedicated test server at `arch.auca.space`. The same
   credentials as from the `auca.space` server can be used to login into it.

2. Ensure that the machine has `git`, `make`, `gcc`, and `gdb` installed.

3. Get the sources. Ensure that [git-lfs](https://git-lfs.github.com/) is
   installed to get the large test images together with sources.

```bash
git clone https://github.com/toksaitov/ips-arch-project.git
```

4. Go inside the project's directory.

```bash
cd ips-arch-project
```

5. Compile all the variants of the image processing program.

```bash
make
```

6. Run some versions of the compiled program (C, x87 ASM, SIMD ASM) without
   arguments. The executables will print the usage message.

```bash
./ips_c_unoptimized
```

7. Use the system to process some 24-bit BMP images. Try not to use large images
   with the median filter.

```bash
./ips_c_unoptimized brightness-contrast -100 2 test/test_image.bmp test/test_image_result_1.bmp
./ips_c_unoptimized sepia test/test_image.bmp test/test_image_result_2.bmp
./ips_c_unoptimized median test/test_image_small.bmp test/test_image_result_3.bmp
```

8. Try out the profile rule from the `Makefile`. It will compile all the
   programs with the profiling code enabled. Then, it will run them in turnes on
   the test images to compare the differences in timing. Note, that you can
   increase the number of profiling passes in `ips.c` to avoid the problem of
   having cold CPU caches.

```bash
make profile
```

9. Start working on the x87 and SIMD optimized assembly versions in the
   `filters.impl.h.c` file. You can either use the GCC inline assembly or create
   a separate `.s` file with exported (`.global`) labels to jump to or call from
   the C source file. Do not forget to add your custom `.s` files to the
   `Makefile` to assemble and link with the final executables. For inline
   assembly you do not need to modify the build system. Just call `make` to
   recompile all the versions.

## Deliverables

* Every file and directory from the template, including your own sources, zipped
  as `<AUCA_Login>_course_project.zip` and sent to my email address.

## Rubric

* _Part 1, Brightness and Contrast Adjustments with x87 Instructions_: **5 points**
* _Part 2, Brightness and Contrast Adjustments with AVX-512 SIMD Instructions_: **15 points**
* _Part 3, The Sepia Filter_: **15 points**
* _Part 4, the Median Filter_: **25 points**
* _Extra, the Median Filter with Register Merging_: **+10 points**

## Research Papers

* [Image Processing Acceleration Techniques using Intel Streaming SIMD Extensions](https://software.intel.com/en-us/articles/image-processing-acceleration-techniques-using-intel-streaming-simd-extensions-and-intel-advanced-vector-extensions)

* [A Novel Hybrid Quicksort Algorithm Vectorized using AVX-512 on Intel Skylake](https://arxiv.org/pdf/1704.08579.pdf)

## Documentation

    man make
    man gcc
    man as
    man gdb
    man objdump

## Links

### C, GDB, Radare2

* [Beej's Guide to C Programming](https://beej.us/guide/bgc)
* [GDB Quick Reference](http://users.ece.utexas.edu/~adnan/gdb-refcard.pdf)
* [Radare2 Visual Graphs](https://monosource.gitbooks.io/radare2-explorations/content/intro/visual_graphs.html)

### x86 ISA

* [Intel x86 Software Developer Manuals](https://software.intel.com/en-us/articles/intel-sdm)
* [System V AMD64 ABI](https://software.intel.com/sites/default/files/article/402129/mpx-linux64-abi.pdf)
* [X86 Opcode Reference](http://ref.x86asm.net/index.html)
* [X86 Instruction Reference](http://www.felixcloutier.com/x86)
* [Optimizing Subroutines in Assembly Language](http://www.agner.org/optimize/optimizing_assembly.pdf)
* [Jump Quick Reference](http://unixwiz.net/techtips/x86-jumps.html)

### Assemblers

* [Linux assemblers: A comparison of GAS and NASM](https://www.ibm.com/developerworks/library/l-gas-nasm/index.html)
* [GAS Syntax](https://en.wikibooks.org/wiki/X86_Assembly/GAS_Syntax)

## Books

* C Programming: A Modern Approach, 2nd Edition by K. N. King
* Assembly Language for x86 Processors, 7th Edition by Kip R. Irvine

