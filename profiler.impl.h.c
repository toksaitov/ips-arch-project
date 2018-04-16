#include "profiler.h"

#include <stdio.h>
#include <math.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#else
#include <time.h>
#endif

static const long Profiler_Nanoseconds_in_Seconds = 1000000000;

static inline void profiler_init_time(struct timespec *time)
{
    if (NULL != time) {
        time->tv_sec  = 0;
        time->tv_nsec = 0;
    }
}

static inline void profiler_get_time(struct timespec *result)
{
#ifdef __MACH__
    clock_serv_t clock_service;
    mach_timespec_t mach_time;

    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &clock_service);
    clock_get_time(clock_service, &mach_time);
    mach_port_deallocate(mach_task_self(), clock_service);

    result->tv_sec  = (time_t) mach_time.tv_sec;
    result->tv_nsec = (long) mach_time.tv_nsec;
#else
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, result);
#endif
}

static inline void profiler_get_delta_time(
                       struct timespec *start_time,
                       struct timespec *end_time,
                       struct timespec *result
                   )
{
    if (0 > (end_time->tv_nsec - start_time->tv_nsec)) {
        result->tv_sec  = end_time->tv_sec - start_time->tv_sec - 1;
        result->tv_nsec = Profiler_Nanoseconds_in_Seconds +
                            end_time->tv_nsec - start_time->tv_nsec;
    } else {
        result->tv_sec  = end_time->tv_sec  - start_time->tv_sec;
        result->tv_nsec = end_time->tv_nsec - start_time->tv_nsec;
    }
}

static inline void profiler_add_time(
                       struct timespec *start_time,
                       struct timespec *end_time,
                       struct timespec *result
                   )
{
    long nanoseconds = start_time->tv_nsec + end_time->tv_nsec;
    if (nanoseconds > Profiler_Nanoseconds_in_Seconds) {
        result->tv_sec  = start_time->tv_sec + end_time->tv_sec + 1;
        result->tv_nsec = nanoseconds - Profiler_Nanoseconds_in_Seconds;
    } else {
        result->tv_sec  = start_time->tv_sec + end_time->tv_sec;
        result->tv_nsec = nanoseconds;
    }
}

static inline void profiler_divide_time(
                       struct timespec *time,
                       double value,
                       struct timespec *result
                   )
{
    double total_nanoseconds =
        (Profiler_Nanoseconds_in_Seconds * (double) time->tv_sec + (double) time->tv_nsec) /
            value;

    double seconds = floor(total_nanoseconds / Profiler_Nanoseconds_in_Seconds),
           nanoseconds = fmod(total_nanoseconds, Profiler_Nanoseconds_in_Seconds);

    result->tv_sec  = (time_t) seconds;
    result->tv_nsec = (long) nanoseconds;
}

#define PROFILER_START(PROFILER_PASSES)                                       \
do {                                                                          \
    struct timespec start_time, end_time,                                     \
    delta_time, average_delta_time;                                           \
                                                                              \
    profiler_init_time(&start_time); profiler_init_time(&end_time);           \
    profiler_init_time(&delta_time); profiler_init_time(&average_delta_time); \
                                                                              \
    fprintf(stderr,                                                           \
             "Profiler: making %d profile %s\n",                              \
                PROFILER_PASSES, (PROFILER_PASSES == 1 ? "pass" : "passes")); \
                                                                              \
    int profile_pass;                                                         \
    for (profile_pass = 0; profile_pass < PROFILER_PASSES; ++profile_pass) {  \
        profiler_get_time(&start_time);

#ifdef PROFILER_VERBOSE_OUTPUT
#define PROFILER_STOP()                                                            \
        profiler_get_time(&end_time);                                              \
        profiler_get_delta_time(&start_time, &end_time, &delta_time);              \
        profiler_add_time(&average_delta_time, &delta_time, &average_delta_time);  \
                                                                                   \
        fprintf(stderr,                                                            \
                 "Profiler: pass %d, processed in %.1f %s : %.1f %s\n",            \
                    profile_pass + 1,                                              \
                    (double) delta_time.tv_sec,                                    \
                    (delta_time.tv_sec == 1 ? "second" : "seconds"),               \
                    (double) delta_time.tv_nsec,                                   \
                    (delta_time.tv_nsec == 1 ? "nanosecond" : "nanoseconds"));     \
    }                                                                              \
                                                                                   \
    profiler_divide_time(&average_delta_time, profile_pass, &average_delta_time);  \
                                                                                   \
    fprintf(stderr,                                                                \
             "---\n"                                                               \
             "Profiler: average for %d profile %s is %.1f %s : %.1f %s\n",         \
                profile_pass,                                                      \
                (profile_pass ==  1 ? "pass" : "passes"),                          \
                (double) average_delta_time.tv_sec,                                \
                (average_delta_time.tv_sec == 1 ? "second" : "seconds"),           \
                (double) average_delta_time.tv_nsec,                               \
                (average_delta_time.tv_nsec == 1 ? "nanosecond" : "nanoseconds")); \
} while(0)
#else
#define PROFILER_STOP()                                                            \
        profiler_get_time(&end_time);                                              \
        profiler_get_delta_time(&start_time, &end_time, &delta_time);              \
        profiler_add_time(&average_delta_time, &delta_time, &average_delta_time);  \
    }                                                                              \
                                                                                   \
    profiler_divide_time(&average_delta_time, profile_pass, &average_delta_time);  \
                                                                                   \
    fprintf(stderr,                                                                \
             "---\n"                                                               \
             "Profiler: average for %d profile %s is %.1f %s : %.1f %s\n",         \
                profile_pass,                                                      \
                (profile_pass ==  1 ? "pass" : "passes"),                          \
                (double) average_delta_time.tv_sec,                                \
                (average_delta_time.tv_sec == 1 ? "second" : "seconds"),           \
                (double) average_delta_time.tv_nsec,                               \
                (average_delta_time.tv_nsec == 1 ? "nanosecond" : "nanoseconds")); \
} while(0)
#endif

