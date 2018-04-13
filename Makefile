CC     = gcc
CFLAGS = -std=c99 -DPROFILE -DPROFILER_VERBOSE_OUTPUT

EXECUTABLES = ips_c_unoptimized   \
              ips_asm_unoptimized \
              ips_c_optimized     \
              ips_asm_optimized

HEADERS = bmp.h                       \
          bmp.inline.c                \
          threadpool.h                \
          threadpool.inline.c         \
          queue.h                     \
          queue.inline.c              \
          synchronized_queue.h        \
          synchronized_queue.inline.c \
          work_item.h                 \
          work_item.inline.c          \
          filters.h                   \
          filters.inline.c            \
          filters_threading.h         \
          filters_threading.inline.c  \
          utils.h                     \
          utils.inline.c              \
          profiler.h                  \
          profiler.inline.c

SOURCES = ips.c

PROFILE_IMAGE  = test/test_image.bmp
PROFILE_OUTPUT = test/test_image_processed.bmp

.PHONY: all
all : $(EXECUTABLES)

ips_c_unoptimized : ${SOURCES} $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_C_IMPLEMENTATION -O0 -o $@ $<

ips_asm_unoptimized : ${SOURCES} $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_X87_ASM_IMPLEMENTATION -O0 -o $@ $<

ips_c_optimized : ${SOURCES} $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_C_IMPLEMENTATION -O3 -ffast-math -flto -o $@ $<

ips_asm_optimized : ${SOURCES} $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_SIMD_ASM_IMPLEMENTATION -O3 -ffast-math -flto -o $@ $<

profile : $(EXECUTABLES)
	for executable in $(EXECUTABLES) ; do ./$$executable brightness-contrast 10 2 $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done
	for executable in $(EXECUTABLES) ; do ./$$executable sepia $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done
	for executable in $(EXECUTABLES) ; do ./$$executable median $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done

.PHONY: clean
clean :
	rm -f $(EXECUTABLES)

