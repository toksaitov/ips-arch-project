#include "synchronized_queue.h"
#include "work_item.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static void *_thread_start(void *args)
{
    synchronized_queue_t *queue = (synchronized_queue_t *) args;
    while (true) {
        work_item_t *work_item = (work_item_t *) synchronized_queue_pop(queue);
        if (NULL == work_item) {
            continue;
        }

        work_item->task(work_item->task_data, work_item->result_callback);
        work_item_destroy(work_item);
    }

    return NULL;
}

static inline threadpool_t *threadpool_allocate(void)
{
    return (threadpool_t *) malloc(sizeof(threadpool_t));
}

static threadpool_t *threadpool_init(threadpool_t *threadpool, size_t pool_size)
{
    threadpool->thread_count =
        pool_size;

    threadpool->queue = synchronized_queue_create();
    if (NULL == threadpool->queue) {
        return NULL;
    }

    threadpool->threads = (pthread_t *) malloc(sizeof(pthread_t) * pool_size);
    if (NULL == threadpool->threads) {
        synchronized_queue_destroy(threadpool->queue);
        threadpool->queue = NULL;

        return NULL;
    }

    for (size_t i = 0; i < pool_size; ++i) {
        pthread_create(
            &threadpool->threads[i],
            NULL,
            _thread_start,
            (void *) threadpool->queue
        );
    }

    return threadpool;
}

static inline threadpool_t *threadpool_create(size_t pool_size)
{
    threadpool_t *threadpool = threadpool_allocate();
    if (NULL == threadpool) {
        return threadpool;
    }

    if (NULL == threadpool_init(threadpool, pool_size)) {
        free(threadpool);

        return NULL;
    }

    return threadpool;
}

static void threadpool_destroy(threadpool_t *threadpool)
{
    if (NULL == threadpool) {
        return;
    }

    if (NULL != threadpool->threads) {
        for (size_t i = 0; i < threadpool->thread_count; ++i) {
            pthread_join(threadpool->threads[i], NULL);
        }

        free(threadpool->threads);
        threadpool->threads = NULL;
    }

    if (NULL != threadpool->queue) {
        synchronized_queue_destroy(threadpool->queue);
        threadpool->queue = NULL;
    }

    free(threadpool);
}

static inline void threadpool_enqueue_task(
                       threadpool_t *threadpool,
                       void (*task)(void *task_data, void (*result_callback)(void *result)),
                       void *task_data,
                       void (*result_callback)(void *result)
                   )
{
    work_item_t *work_item = work_item_create(task, task_data, result_callback);
    if (NULL == work_item) {
        return;
    }

    synchronized_queue_enqueue(threadpool->queue, work_item);
}

