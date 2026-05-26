#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bank.h"
#include "buffer_pool.h"
#include "metrics.h"
#include "timer.h"
#include "transaction.h"

static bool load_trace(const char *path, BufferPool *pool, int worker_count) {
    FILE *file;
    char line[256];
    int tx_id = 1;
    int i;

    file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen trace");
        return false;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        Transaction tx;

        if (!transaction_parse_line(line, tx_id, &tx)) {
            continue;
        }

        if (!buffer_pool_push(pool, &tx)) {
            fclose(file);
            return false;
        }

        ++tx_id;
    }

    fclose(file);

    for (i = 0; i < worker_count; ++i) {
        Transaction stop_tx;
        memset(&stop_tx, 0, sizeof(stop_tx));
        stop_tx.type = TX_STOP;
        if (!buffer_pool_push(pool, &stop_tx)) {
            return false;
        }
    }

    return true;
}

static void usage(const char *program) {
    fprintf(stderr, "usage: %s <accounts.txt> <trace.txt> [workers] [buffer_capacity]\n", program);
}

int main(int argc, char **argv) {
    const char *accounts_path;
    const char *trace_path;
    int worker_count = 4;
    int buffer_capacity = 8;
    pthread_t *workers = NULL;
    WorkerArgs *worker_args = NULL;
    Bank bank = {0};
    BufferPool pool = {0};
    Metrics metrics = {0};
    Timer timer;
    int i;
    bool ok = false;

    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    accounts_path = argv[1];
    trace_path = argv[2];
    if (argc >= 4) {
        worker_count = atoi(argv[3]);
    }
    if (argc >= 5) {
        buffer_capacity = atoi(argv[4]);
    }

    if (worker_count <= 0 || buffer_capacity <= 0) {
        fprintf(stderr, "workers and buffer_capacity must be positive\n");
        return 1;
    }

    if (!bank_load_accounts(&bank, accounts_path)) {
        goto cleanup;
    }

    if (!buffer_pool_init(&pool, (size_t)buffer_capacity)) {
        goto cleanup;
    }

    if (!metrics_init(&metrics)) {
        goto cleanup;
    }

    workers = calloc((size_t)worker_count, sizeof(*workers));
    worker_args = calloc((size_t)worker_count, sizeof(*worker_args));
    if (workers == NULL || worker_args == NULL) {
        goto cleanup;
    }

    timer_start(&timer);
    for (i = 0; i < worker_count; ++i) {
        worker_args[i].worker_id = i;
        worker_args[i].bank = &bank;
        worker_args[i].pool = &pool;
        worker_args[i].metrics = &metrics;
        pthread_create(&workers[i], NULL, transaction_worker_main, &worker_args[i]);
    }

    if (!load_trace(trace_path, &pool, worker_count)) {
        goto cleanup;
    }

    for (i = 0; i < worker_count; ++i) {
        pthread_join(workers[i], NULL);
    }
    timer_stop(&timer);

    printf("Completed in %.3f ms\n", timer_elapsed_ms(&timer));
    bank_print_accounts(&bank);
    metrics_print(&metrics);
    printf("Initial total: %ld\n", bank.initial_total);
    printf("Expected total after external flow: %ld\n", bank.initial_total + metrics.net_external_flow);
    printf("Final total:                 %ld\n", bank_total_balance(&bank));

    ok = true;

cleanup:
    buffer_pool_close(&pool);
    free(workers);
    free(worker_args);
    metrics_destroy(&metrics);
    buffer_pool_destroy(&pool);
    bank_destroy(&bank);
    return ok ? 0 : 1;
}
