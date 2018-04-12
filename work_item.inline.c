#include "work_item.h"

static inline work_item_t *work_item_create(
                               void (*task)(void *task_data, void (*result_callback)(void *result)),
                               void *task_data,
                               void (*result_callback)(void *result)
                           )
{
    work_item_t *work_item = (work_item_t *) malloc(sizeof(*work_item));
    if (NULL == work_item) {
        return work_item;
    }

    work_item->task = task;
    work_item->task_data = task_data;
    work_item->result_callback = result_callback;

    return work_item;
}

static inline void work_item_destroy(work_item_t *work_item)
{
    if (NULL != work_item) {
        free(work_item);
    }
}

