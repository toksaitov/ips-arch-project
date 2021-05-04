#ifndef WORK_ITEM_H
#define WORK_ITEM_H

typedef struct work_item
{
    void (*task)(void *task_data, void (*result_callback)(void *result));
    void *task_data;
    void (*result_callback)(void *result);
} work_item_t;

static inline work_item_t *work_item_create(
                               void (*task)(void *task_data, void (*result_callback)(void *result)),
                               void *task_data,
                               void (*result_callback)(void *result)
                           );

static inline void work_item_destroy(work_item_t *work_item);

#include "work_item.impl.h.c"

#endif // WORK_ITEM_H

