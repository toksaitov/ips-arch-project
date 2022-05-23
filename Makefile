CC     = gcc
CFLAGS = -std=gnu11 -DPROFILE -DPROFILER_VERBOSE_OUTPUT
LDLIBS = -lm -lpthread

EXECUTABLES = ips_c_unoptimized   \
              ips_asm_unoptimized \
              ips_c_optimized     \
              ips_asm_optimized   \
              ips_asm_intr_optimized

HEADERS = bmp.h                       \
          bmp.impl.h.c                \
          threadpool.h                \
          threadpool.impl.h.c         \
          queue.h                     \
          queue.impl.h.c              \
          synchronized_queue.h        \
          synchronized_queue.impl.h.c \
          work_item.h                 \
          work_item.impl.h.c          \
          filters.h                   \
          filters.impl.h.c            \
          filters_threading.h         \
          filters_threading.impl.h.c  \
          utils.h                     \
          utils.impl.h.c              \
          profiler.h                  \
          profiler.impl.h.c

SOURCES = ips.c

PROFILE_IMAGE   = test_image.bmp
PROFILE_IMAGE_2 = test_image_small.bmp
PROFILE_OUTPUT  = test_image_processed.bmp

.PHONY: all
all : $(EXECUTABLES)

ips_c_unoptimized : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_C_IMPLEMENTATION -O0 -o $@ $< $(LDLIBS)

ips_asm_unoptimized : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_X87_ASM_IMPLEMENTATION -O0 -o $@ $< $(LDLIBS)

ips_c_optimized : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_C_IMPLEMENTATION -O3 -mavx512f -ffast-math -flto -o $@ $< $(LDLIBS)

ips_asm_optimized : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_SIMD_ASM_IMPLEMENTATION -O0 -Wno-attributes -mavx512f -ffast-math -flto -o $@ $< $(LDLIBS)

ips_asm_intr_optimized : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DFILTERS_SIMD_ASM_IMPLEMENTATION -DINTRINSICS -O3 -Wno-attributes -mavx512f -ffast-math -flto -o $@ $< $(LDLIBS)

$(PROFILE_IMAGE) :
	curl --location -C - --output '$(PROFILE_IMAGE)' 'https://www.dropbox.com/s/jevpkoris58avyv/test_image.bmp?dl=1'

$(PROFILE_IMAGE_2) :
	curl --location -C - --output '$(PROFILE_IMAGE_2)' 'https://www.dropbox.com/s/jevpkoris58avyv/test_image.bmp?dl=1'

.PHONY: profile
profile : $(EXECUTABLES) $(PROFILE_IMAGE) $(PROFILE_IMAGE_2)
	for executable in $(EXECUTABLES) ; do echo "./$$executable brightness-contrast 10 2 $(PROFILE_IMAGE) $(PROFILE_OUTPUT)" ; ./$$executable brightness-contrast 10 2 $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done
	for executable in $(EXECUTABLES) ; do echo "./$$executable sepia $(PROFILE_IMAGE) $(PROFILE_OUTPUT)" ; ./$$executable sepia $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done
	for executable in $(EXECUTABLES) ; do echo "./$$executable median $(PROFILE_IMAGE) $(PROFILE_OUTPUT)" ; ./$$executable median $(PROFILE_IMAGE) $(PROFILE_OUTPUT) ; done

.PHONY: clean
clean :
	rm -f $(EXECUTABLES)

