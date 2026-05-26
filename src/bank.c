#include "bank.h"

#include <stdio.h>
#include <stdlib.h>

bool bank_init(Bank *bank, size_t account_count) {
    size_t i;

    bank->accounts = calloc(account_count, sizeof(*bank->accounts));
    if (bank->accounts == NULL) {
        return false;
    }

    bank->account_count = account_count;
    bank->initial_total = 0;
    for (i = 0; i < account_count; ++i) {
        bank->accounts[i].id = (int)i;
        bank->accounts[i].balance = 0;
        pthread_rwlock_init(&bank->accounts[i].lock, NULL);
    }

    return true;
}

void bank_destroy(Bank *bank) {
    size_t i;

    if (bank->accounts == NULL) {
        return;
    }

    for (i = 0; i < bank->account_count; ++i) {
        pthread_rwlock_destroy(&bank->accounts[i].lock);
    }

    free(bank->accounts);
    bank->accounts = NULL;
    bank->account_count = 0;
    bank->initial_total = 0;
}

bool bank_load_accounts(Bank *bank, const char *path) {
    FILE *file;
    size_t count = 0;
    char line[256];
    int id;
    long balance;

    file = fopen(path, "r");
    if (file == NULL) {
        perror("fopen accounts");
        return false;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%d %ld", &id, &balance) == 2) {
            ++count;
        }
    }

    if (count == 0) {
        fclose(file);
        fprintf(stderr, "No accounts were found in %s\n", path);
        return false;
    }

    rewind(file);
    if (!bank_init(bank, count)) {
        fclose(file);
        return false;
    }

    count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%d %ld", &id, &balance) != 2) {
            continue;
        }

        if (count >= bank->account_count) {
            break;
        }

        bank->accounts[count].id = id;
        bank->accounts[count].balance = balance;
        bank->initial_total += balance;
        ++count;
    }

    fclose(file);
    return true;
}

Account *bank_get_account(Bank *bank, int account_id) {
    size_t i;

    for (i = 0; i < bank->account_count; ++i) {
        if (bank->accounts[i].id == account_id) {
            return &bank->accounts[i];
        }
    }

    return NULL;
}

long bank_total_balance(Bank *bank) {
    size_t i;
    long total = 0;

    for (i = 0; i < bank->account_count; ++i) {
        pthread_rwlock_rdlock(&bank->accounts[i].lock);
        total += bank->accounts[i].balance;
        pthread_rwlock_unlock(&bank->accounts[i].lock);
    }

    return total;
}

void bank_print_accounts(const Bank *bank) {
    size_t i;

    for (i = 0; i < bank->account_count; ++i) {
        printf("account %d => %ld\n", bank->accounts[i].id, bank->accounts[i].balance);
    }
}
