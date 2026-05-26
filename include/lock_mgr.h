#ifndef LOCK_MGR_H
#define LOCK_MGR_H

#include <stdbool.h>

#include "bank.h"

bool lock_mgr_lock_pair(Account *first, Account *second);
void lock_mgr_unlock_pair(Account *first, Account *second);

#endif
