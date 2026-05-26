#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

struct Transaction;

typedef struct {
    struct Transaction *items;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    bool closed;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} BufferPool;

bool buffer_pool_init(BufferPool *pool, size_t capacity);
void buffer_pool_destroy(BufferPool *pool);
bool buffer_pool_push(BufferPool *pool, const struct Transaction *tx);
bool buffer_pool_pop(BufferPool *pool, struct Transaction *out);
void buffer_pool_close(BufferPool *pool);

#endif
