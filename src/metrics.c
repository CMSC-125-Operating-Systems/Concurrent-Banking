#include "metrics.h"

#include <stdio.h>

bool metrics_init(Metrics *metrics) {
    metrics->processed = 0;
    metrics->committed = 0;
    metrics->aborted = 0;
    metrics->read_ops = 0;
    metrics->write_ops = 0;
    metrics->net_external_flow = 0;
    return pthread_mutex_init(&metrics->mutex, NULL) == 0;
}

void metrics_destroy(Metrics *metrics) {
    pthread_mutex_destroy(&metrics->mutex);
}

void metrics_record_commit(Metrics *metrics, bool write_op, long external_delta) {
    pthread_mutex_lock(&metrics->mutex);
    ++metrics->processed;
    ++metrics->committed;
    metrics->net_external_flow += external_delta;
    if (write_op) {
        ++metrics->write_ops;
    } else {
        ++metrics->read_ops;
    }
    pthread_mutex_unlock(&metrics->mutex);
}

void metrics_record_abort(Metrics *metrics, bool write_op) {
    pthread_mutex_lock(&metrics->mutex);
    ++metrics->processed;
    ++metrics->aborted;
    if (write_op) {
        ++metrics->write_ops;
    } else {
        ++metrics->read_ops;
    }
    pthread_mutex_unlock(&metrics->mutex);
}

void metrics_print(const Metrics *metrics) {
    printf("processed=%ld committed=%ld aborted=%ld read_ops=%ld write_ops=%ld net_external_flow=%ld\n",
           metrics->processed,
           metrics->committed,
           metrics->aborted,
           metrics->read_ops,
           metrics->write_ops,
           metrics->net_external_flow);
}
