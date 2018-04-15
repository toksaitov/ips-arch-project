#ifndef PROFILER_H
#define PROFILER_H

#ifndef PROFILE
#define PROFILER_START(PROFILER_PASSES)
#define PROFILER_STOP()
#else

#include <time.h>

static inline void profiler_init_time(struct timespec *time);

static inline void profiler_get_time(struct timespec *result);

static inline void profiler_get_delta_time(
                       struct timespec *start_time,
                       struct timespec *end_time,
                       struct timespec *result
                   );

static inline void profiler_add_time(
                       struct timespec *start_time,
                       struct timespec *end_time,
                       struct timespec *result
                   );

static inline void profiler_divide_time(
                       struct timespec *time,
                       double value,
                       struct timespec *result
                   );

#include "profiler.impl.h.c"

#endif

#endif /* PROFILER_H */

