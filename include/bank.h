#ifndef BANK_H
#define BANK_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int id;
    long balance;
    pthread_rwlock_t lock;
} Account;

typedef struct {
    Account *accounts;
    size_t account_count;
    long initial_total;
} Bank;

bool bank_init(Bank *bank, size_t account_count);
void bank_destroy(Bank *bank);
bool bank_load_accounts(Bank *bank, const char *path);
Account *bank_get_account(Bank *bank, int account_id);
long bank_total_balance(Bank *bank);
void bank_print_accounts(const Bank *bank);

#endif
