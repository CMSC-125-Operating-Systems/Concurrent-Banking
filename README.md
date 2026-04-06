# Concurrent-Banking
Lab 3

##PROJECT STRUCTURE
```bash
Concurrent-Banking/
|-- Makefile
|-- README.md
|-- include/
|   |-- bank.h           # Bank and account structures
|   |-- transaction.h    # Transaction and operation types
|   |-- timer.h          # Timer thread and clock functions
|   |-- lock_mgr.h       # Lock ordering or deadlock detection
|   |-- buffer_pool.h    # Buffer pool with semaphores
|   +-- metrics.h        # Statistics collection
|-- src/
|   |-- main.c           # CLI parsing, initialization
|   |-- bank.c           # Account operations
|   |-- transaction.c    # Transaction execution thread
|   |-- timer.c          # Timer thread implementation
|   |-- lock_mgr.c       # Deadlock prevention or detection
|   |-- buffer_pool.c    # Bounded buffer implementation
|   |-- metrics.c        # Metrics calculation and reporting
|   +-- utils.c          # Parsing, error handling
|-- tests/
|   |-- accounts.txt           # Initial account balances
|   |-- trace_simple.txt       # Test 1
|   |-- trace_readers.txt      # Test 2
|   |-- trace_deadlock.txt     # Test 3
|   |-- trace_abort.txt        # Test 4
|   +-- trace_buffer.txt       # Test 5
+-- docs/
    +-- design.md              # Design justifications

```
