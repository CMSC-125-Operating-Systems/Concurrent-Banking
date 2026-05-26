#include "lock_mgr.h"

bool lock_mgr_lock_pair(Account *first, Account *second) {
    Account *low;
    Account *high;

    if (first == NULL || second == NULL) {
        return false;
    }

    if (first == second) {
        pthread_rwlock_wrlock(&first->lock);
        return true;
    }

    low = first->id < second->id ? first : second;
    high = first->id < second->id ? second : first;

    pthread_rwlock_wrlock(&low->lock);
    pthread_rwlock_wrlock(&high->lock);
    return true;
}

void lock_mgr_unlock_pair(Account *first, Account *second) {
    if (first == NULL || second == NULL) {
        return;
    }

    if (first == second) {
        pthread_rwlock_unlock(&first->lock);
        return;
    }

    pthread_rwlock_unlock(&first->lock);
    pthread_rwlock_unlock(&second->lock);
}
