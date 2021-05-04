#ifndef SYNCHRONIZED_QUEUE_H
#define SYNCHRONIZED_QUEUE_H

#include <stddef.h>
#include <pthread.h>

#include "queue.h"

typedef struct _synchronized_queue
{
    pthread_mutex_t access_mutex;
    pthread_cond_t not_empty_condition;
    queue_t implementation;
} synchronized_queue_t;

static inline synchronized_queue_t *synchronized_queue_allocate(void);

static inline synchronized_queue_t *synchronized_queue_init(synchronized_queue_t *queue);

static inline synchronized_queue_t *synchronized_queue_create(void);

static inline void synchronized_queue_destroy(synchronized_queue_t *queue);

static inline size_t synchronized_queue_get_size(synchronized_queue_t *queue);

static inline bool synchronized_queue_is_empty(synchronized_queue_t *queue);

static synchronized_queue_t *synchronized_queue_enqueue(synchronized_queue_t *queue, void *data);

static void *synchronized_queue_pop(synchronized_queue_t *queue);

#include "synchronized_queue.impl.h.c"

#endif // SYNCHRONIZED_QUEUE_H

