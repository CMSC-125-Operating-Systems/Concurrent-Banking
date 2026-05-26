#include "transaction.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "lock_mgr.h"

static bool parse_keyword(const char *op, TransactionType *type) {
    if (strcmp(op, "BALANCE") == 0 || strcmp(op, "B") == 0) {
        *type = TX_BALANCE;
        return true;
    }
    if (strcmp(op, "DEPOSIT") == 0 || strcmp(op, "D") == 0) {
        *type = TX_DEPOSIT;
        return true;
    }
    if (strcmp(op, "WITHDRAW") == 0 || strcmp(op, "W") == 0) {
        *type = TX_WITHDRAW;
        return true;
    }
    if (strcmp(op, "TRANSFER") == 0 || strcmp(op, "T") == 0) {
        *type = TX_TRANSFER;
        return true;
    }
    return false;
}

bool transaction_parse_line(const char *line, int tx_id, Transaction *tx) {
    char op[32];
    int scanned;
    int src = -1;
    int dst = -1;
    long amount = 0;
    size_t i;

    while (isspace((unsigned char)*line)) {
        ++line;
    }
    if (*line == '\0' || *line == '#') {
        return false;
    }

    scanned = sscanf(line, "%31s %d %d %ld", op, &src, &dst, &amount);
    if (scanned < 2) {
        return false;
    }

    for (i = 0; op[i] != '\0'; ++i) {
        op[i] = (char)toupper((unsigned char)op[i]);
    }

    memset(tx, 0, sizeof(*tx));
    tx->id = tx_id;

    if (!parse_keyword(op, &tx->type)) {
        return false;
    }

    if (tx->type == TX_BALANCE) {
        tx->src_account = src;
        return scanned == 2;
    }

    if (tx->type == TX_TRANSFER) {
        tx->src_account = src;
        tx->dst_account = dst;
        tx->amount = amount;
        return scanned == 4;
    }

    tx->src_account = src;
    tx->amount = scanned >= 3 ? dst : 0;
    return scanned >= 3;
}

bool transaction_is_write(const Transaction *tx) {
    return tx->type == TX_DEPOSIT || tx->type == TX_WITHDRAW || tx->type == TX_TRANSFER;
}

const char *transaction_type_name(TransactionType type) {
    switch (type) {
        case TX_BALANCE:
            return "BALANCE";
        case TX_DEPOSIT:
            return "DEPOSIT";
        case TX_WITHDRAW:
            return "WITHDRAW";
        case TX_TRANSFER:
            return "TRANSFER";
        case TX_STOP:
            return "STOP";
    }

    return "UNKNOWN";
}

bool transaction_execute(Bank *bank, const Transaction *tx, Metrics *metrics) {
    Account *src;
    Account *dst;
    long observed_balance;

    src = bank_get_account(bank, tx->src_account);
    dst = bank_get_account(bank, tx->dst_account);

    switch (tx->type) {
        case TX_BALANCE:
            if (src == NULL) {
                metrics_record_abort(metrics, false);
                return false;
            }
            pthread_rwlock_rdlock(&src->lock);
            observed_balance = src->balance;
            pthread_rwlock_unlock(&src->lock);
            printf("[tx %d] balance(%d) = %ld\n", tx->id, tx->src_account, observed_balance);
            metrics_record_commit(metrics, false, 0);
            return true;

        case TX_DEPOSIT:
            if (src == NULL) {
                metrics_record_abort(metrics, true);
                return false;
            }
            pthread_rwlock_wrlock(&src->lock);
            src->balance += tx->amount;
            pthread_rwlock_unlock(&src->lock);
            metrics_record_commit(metrics, true, tx->amount);
            return true;

        case TX_WITHDRAW:
            if (src == NULL) {
                metrics_record_abort(metrics, true);
                return false;
            }
            pthread_rwlock_wrlock(&src->lock);
            if (src->balance < tx->amount) {
                pthread_rwlock_unlock(&src->lock);
                metrics_record_abort(metrics, true);
                return false;
            }
            src->balance -= tx->amount;
            pthread_rwlock_unlock(&src->lock);
            metrics_record_commit(metrics, true, -tx->amount);
            return true;

        case TX_TRANSFER:
            if (src == NULL || dst == NULL) {
                metrics_record_abort(metrics, true);
                return false;
            }
            lock_mgr_lock_pair(src, dst);
            if (src->balance < tx->amount) {
                lock_mgr_unlock_pair(src, dst);
                metrics_record_abort(metrics, true);
                return false;
            }
            src->balance -= tx->amount;
            dst->balance += tx->amount;
            lock_mgr_unlock_pair(src, dst);
            metrics_record_commit(metrics, true, 0);
            return true;

        case TX_STOP:
            return true;
    }

    metrics_record_abort(metrics, false);
    return false;
}

void *transaction_worker_main(void *arg) {
    WorkerArgs *worker;
    Transaction tx;

    worker = arg;
    while (buffer_pool_pop(worker->pool, &tx)) {
        if (tx.type == TX_STOP) {
            break;
        }

        transaction_execute(worker->bank, &tx, worker->metrics);
    }

    return NULL;
}
