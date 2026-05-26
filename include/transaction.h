#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdbool.h>
#include <stddef.h>

#include "bank.h"
#include "buffer_pool.h"
#include "metrics.h"

typedef enum {
    TX_BALANCE,
    TX_DEPOSIT,
    TX_WITHDRAW,
    TX_TRANSFER,
    TX_STOP
} TransactionType;

typedef struct Transaction {
    int id;
    TransactionType type;
    int src_account;
    int dst_account;
    long amount;
} Transaction;

typedef struct {
    int worker_id;
    Bank *bank;
    BufferPool *pool;
    Metrics *metrics;
} WorkerArgs;

bool transaction_parse_line(const char *line, int tx_id, Transaction *tx);
bool transaction_execute(Bank *bank, const Transaction *tx, Metrics *metrics);
void *transaction_worker_main(void *arg);
const char *transaction_type_name(TransactionType type);
bool transaction_is_write(const Transaction *tx);

#endif
