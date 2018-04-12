CC=gcc
CFLAGS=-std=c99 -DPROFILE -DPROFILER_VERBOSE_OUTPUT

EXECUTABLES=ips_c_unoptimized ips_asm_unoptimized ips_c_optimized ips_asm_optimized
SOURCES=ips.c

PROFILE_IMAGE=test/test_image.bmp
PROFILE_OUTPUT=/tmp/output.bmp

.PHONY: all
all : $(EXECUTABLES)

ips_c_unoptimized : $(SOURCES)
	$(CC) $(CFLAGS) -DFILTERS_C_IMPLEMENTATION -O0 -o $@ $<

ips_asm_unoptimized : $(SOURCES)
	$(CC) $(CFLAGS) -DFILTERS_X87_ASM_IMPLEMENTATION -O0 -o $@ $<

ips_c_optimized : $(SOURCES)
	$(CC) $(CFLAGS) -DFILTERS_C_IMPLEMENTATION -O3 -ffast-math -flto -o $@ $<

ips_asm_optimized : $(SOURCES)
	$(CC) $(CFLAGS) -DFILTERS_SIMD_ASM_IMPLEMENTATION -O3 -ffast-math -flto -o $@ $<

profile : $(EXECUTABLES)
	for executable in $(EXECUTABLES) ; do ./$$executable $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done

.PHONY: clean
clean :
	rm -f $(EXECUTABLES)

