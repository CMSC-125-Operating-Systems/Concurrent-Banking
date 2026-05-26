#ifndef METRICS_H
#define METRICS_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    long processed;
    long committed;
    long aborted;
    long read_ops;
    long write_ops;
    long net_external_flow;
    pthread_mutex_t mutex;
} Metrics;

bool metrics_init(Metrics *metrics);
void metrics_destroy(Metrics *metrics);
void metrics_record_commit(Metrics *metrics, bool write_op, long external_delta);
void metrics_record_abort(Metrics *metrics, bool write_op);
void metrics_print(const Metrics *metrics);

#endif
