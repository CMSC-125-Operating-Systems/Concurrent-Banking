#include "buffer_pool.h"

#include <stdlib.h>
#include <string.h>

#include "transaction.h"

bool buffer_pool_init(BufferPool *pool, size_t capacity) {
    pool->items = calloc(capacity, sizeof(*pool->items));
    if (pool->items == NULL) {
        return false;
    }

    pool->capacity = capacity;
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->closed = false;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pthread_cond_init(&pool->not_full, NULL);
    return true;
}

void buffer_pool_destroy(BufferPool *pool) {
    free(pool->items);
    pool->items = NULL;
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
}

bool buffer_pool_push(BufferPool *pool, const Transaction *tx) {
    pthread_mutex_lock(&pool->mutex);

    while (pool->count == pool->capacity && !pool->closed) {
        pthread_cond_wait(&pool->not_full, &pool->mutex);
    }

    if (pool->closed) {
        pthread_mutex_unlock(&pool->mutex);
        return false;
    }

    memcpy(&pool->items[pool->tail], tx, sizeof(*tx));
    pool->tail = (pool->tail + 1U) % pool->capacity;
    ++pool->count;

    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->mutex);
    return true;
}

bool buffer_pool_pop(BufferPool *pool, Transaction *out) {
    pthread_mutex_lock(&pool->mutex);

    while (pool->count == 0 && !pool->closed) {
        pthread_cond_wait(&pool->not_empty, &pool->mutex);
    }

    if (pool->count == 0 && pool->closed) {
        pthread_mutex_unlock(&pool->mutex);
        return false;
    }

    memcpy(out, &pool->items[pool->head], sizeof(*out));
    pool->head = (pool->head + 1U) % pool->capacity;
    --pool->count;

    pthread_cond_signal(&pool->not_full);
    pthread_mutex_unlock(&pool->mutex);
    return true;
}

void buffer_pool_close(BufferPool *pool) {
    pthread_mutex_lock(&pool->mutex);
    pool->closed = true;
    pthread_cond_broadcast(&pool->not_empty);
    pthread_cond_broadcast(&pool->not_full);
    pthread_mutex_unlock(&pool->mutex);
}
