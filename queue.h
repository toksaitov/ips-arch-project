#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct _queue_item
{
    struct _queue_item *next, *previous;
    void *content;
} queue_item_t;

typedef void (*queue_destroy_element_callback)(void *element);

static inline queue_item_t *queue_item_allocate(void);

static inline queue_item_t *queue_item_init(queue_item_t *queue_item);

static inline queue_item_t *queue_item_create(void);

static inline void queue_item_destroy(queue_item_t *queue_item);

static inline void *queue_item_get_content(queue_item_t *queue_item);

static inline void queue_item_set_content(queue_item_t *queue_item, void *content);

typedef struct _queue
{
    queue_item_t *first, *last;
    size_t size;
} queue_t;

static inline queue_t *queue_allocate(void);

static inline queue_t *queue_init(queue_t *queue);

static inline queue_t *queue_create(void);

static inline void queue_destroy(queue_t *queue);

static inline void queue_deinit(queue_t *queue);

static inline void queue_destroy_with_elements(
                       queue_t *queue,
                       queue_destroy_element_callback destroy_element_callback
                   );

static inline void queue_deinit_with_elements(
                       queue_t *queue,
                       queue_destroy_element_callback destroy_element_callback
                   );

static inline size_t queue_get_size(queue_t *queue);

static inline bool queue_is_empty(queue_t *queue);

static inline void *queue_get_first(queue_t *queue);

static inline void *queue_get_last(queue_t *queue);

static void *queue_push(queue_t *queue, void *element);

static void *queue_deque(queue_t *queue);

static void *queue_pop(queue_t *queue);

#include "queue.impl.h.c"

#endif // QUEUE_H

