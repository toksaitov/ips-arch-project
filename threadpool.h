#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>

#include "synchronized_queue.h"

typedef struct _threadpool
{
    synchronized_queue_t *queue;

    pthread_t *threads;
    size_t thread_count;
} threadpool_t;

static inline threadpool_t *threadpool_allocate(void);

static threadpool_t *threadpool_init(threadpool_t *threadpool, size_t pool_size);

static inline threadpool_t *threadpool_create(size_t pool_size);

static void threadpool_destroy(threadpool_t *threadpool);

static inline void threadpool_enqueue_task(
                       threadpool_t *threadpool,
                       void (*task)(void *task_data, void (*result_callback)(void *result)),
                       void *task_data,
                       void (*result_callback)(void *result)
                   );

#include "threadpool.impl.h.c"

#endif // THREADPOOL_H

