#include "synchronized_queue.h"

#include <stdlib.h>
#include <stdio.h>

static inline synchronized_queue_t *synchronized_queue_allocate()
{
    return (synchronized_queue_t *) malloc(sizeof(synchronized_queue_t));
}

static inline synchronized_queue_t *synchronized_queue_init(synchronized_queue_t *queue)
{
    if (0 != pthread_mutex_init(&queue->access_mutex, NULL)) {
        return NULL;
    }

    if (0 != pthread_cond_init(&queue->not_empty_condition, NULL)) {
        pthread_mutex_destroy(&queue->access_mutex);

        return NULL;
    }
    queue_init(&queue->implementation);

    return queue;
}

static inline synchronized_queue_t *synchronized_queue_create()
{
    synchronized_queue_t *queue = synchronized_queue_allocate();
    if (NULL == queue) {
        return queue;
    }

    if (NULL == synchronized_queue_init(queue)) {
        free(queue);

        return NULL;
    }

    return queue;
}

static inline void synchronized_queue_destroy(synchronized_queue_t *queue)
{
    if (NULL == queue) {
        return;
    }

    pthread_mutex_destroy(&queue->access_mutex);
    pthread_cond_destroy(&queue->not_empty_condition);
    queue_destroy(&queue->implementation);
    free(queue);
}

static inline size_t synchronized_queue_get_size(synchronized_queue_t *queue)
{
    return (size_t) queue_get_size(&queue->implementation);
}

static inline bool synchronized_queue_is_empty(synchronized_queue_t *queue)
{
    return queue_is_empty(&queue->implementation);
}

static synchronized_queue_t *synchronized_queue_enqueue(synchronized_queue_t *queue, void *data)
{
    if (0 != pthread_mutex_lock(&queue->access_mutex)) {
        return NULL;
    }

    queue_push(&queue->implementation, data);
    pthread_cond_broadcast(&queue->not_empty_condition);

    if (0 != pthread_mutex_unlock(&queue->access_mutex)) {
        return NULL;
    }

    return queue;
}

static void *synchronized_queue_pop(synchronized_queue_t *queue)
{
    void *data = NULL;

    if (0 != pthread_mutex_lock(&queue->access_mutex)) {
        return data;
    }

    while (queue_is_empty(&queue->implementation)) {
        if (0 != pthread_cond_wait(&queue->not_empty_condition, &queue->access_mutex)) {
            return data;
        }
    }

    data = queue_pop(&queue->implementation);

    if (0 != pthread_mutex_unlock(&queue->access_mutex)) {
        return data;
    }

    return data;
}

