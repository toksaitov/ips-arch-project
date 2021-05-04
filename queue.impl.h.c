#include "queue.h"

#include <stdlib.h>
#include <string.h>

static inline queue_item_t *queue_item_allocate()
{
    return (queue_item_t *) malloc(sizeof(queue_item_t));
}

static inline queue_item_t *queue_item_init(queue_item_t *queue_item)
{
    if (NULL != queue_item) {
        memset(queue_item, 0, sizeof(*queue_item));
    }

    return queue_item;
}

static inline queue_item_t *queue_item_create()
{
    return queue_item_init(queue_item_allocate());
}

static inline void queue_item_destroy(queue_item_t *queue_item)
{
    if (NULL != queue_item) {
        free(queue_item);
    }
}

static inline void *queue_item_get_content(queue_item_t *queue_item)
{
    return queue_item->content;
}

static inline void queue_item_set_content(queue_item_t *queue_item, void *content)
{
    queue_item->content = content;
}

static inline queue_t *queue_allocate()
{
    return (queue_t *) malloc(sizeof(queue_t));
}

static inline queue_t *queue_init(queue_t *queue)
{
    if (NULL != queue) {
        memset(queue, 0, sizeof(*queue));
    }

    return queue;
}

static inline queue_t *queue_create()
{
    return queue_init(queue_allocate());
}

static inline void queue_destroy(queue_t *queue)
{
    queue_item_t *item;

    if (queue) {
        for (item = queue->first; item;) {
            queue_item_t *next = item->next;
            free(item);
            item = next;
        }

        free(queue);
    }
}

static inline void queue_deinit(queue_t *queue)
{
    queue_item_t *item;

    if (queue) {
        for (item = queue->first; item;) {
            queue_item_t *next = item->next;
            free(item);
            item = next;
        }
    }
}

static inline void queue_destroy_with_elements(
                       queue_t *queue,
                       queue_destroy_element_callback destroy_element_callback
                   )
{
    queue_item_t *item;

    if (queue) {
        for (item = queue->first; item;) {
            queue_item_t *next = item->next;
            destroy_element_callback(item->content);
            free(item);
            item = next;
        }
        free(queue);
    }
}

static inline void queue_deinit_with_elements(
                       queue_t *queue,
                       queue_destroy_element_callback destroy_element_callback
                   )
{
    queue_item_t *item;

    if (queue) {
        for (item = queue->first; item;) {
            queue_item_t *next = item->next;
            destroy_element_callback(item->content);
            free(item);
            item = next;
        }
    }
}

static inline size_t queue_get_size(queue_t *queue)
{
    return queue->size;
}

static inline bool queue_is_empty(queue_t *queue)
{
    return 0 == queue->size;
}

static inline void *queue_get_first(queue_t *queue)
{
    return queue_item_get_content(queue->first);
}

static inline void *queue_get_last(queue_t *queue)
{
    return queue_item_get_content(queue->last);
}

static void *queue_push(queue_t *queue, void *element)
{
    queue_item_t *item = NULL;

    if (NULL != queue && NULL != element) {
        item = queue_item_create();
        item->content = element;

        if (NULL == queue->first) {
            queue->first = item;
        }

        if (NULL != queue->last) {
            queue->last->next = item;
            item->previous = queue->last;
        }

        queue->last = item;
        queue->size += 1;
    }

    return element;
}

static void *queue_deque(queue_t *queue)
{
    void *result = NULL;

    queue_item_t *item = NULL;

    if (NULL != queue && 0 < queue->size) {
        item = queue->first;
        if (NULL != item) {
            queue->first = item->next;
            queue->size -= 1;

            if (0 == queue->size) {
                queue->last = NULL;
            }

            result = item->content;
            queue_item_destroy(item);
        }
    }

    return result;
}

static void *queue_pop(queue_t *queue)
{
    void *result = NULL;

    queue_item_t *item = NULL;
    if (NULL != queue && 0 < queue->size) {
        item = queue->last;
        if (NULL != item) {
            queue->last = item->previous;
            queue->size -= 1;

            if (0 == queue->size) {
                queue->first = NULL;
            }

            result = item->content;
            queue_item_destroy(item);
        }
    }

    return result;
}

