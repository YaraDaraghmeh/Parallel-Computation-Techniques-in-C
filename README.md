# Parallel Computation Techniques in C

## Overview

This project demonstrates four different parallel computing approaches for performing a computational task (power function calculation) across a large dataset:
1. Serial Computation
2. Shared Memory Parallelization
3. Message Passing Parallelization
4. Multithreading Parallelization

## Project Details

### Key Features
- Large dataset size: 12,113,513 elements
- 8 parallel computation processes/threads
- Performance timing for each approach
- Result verification across computation methods

### Computation Method
- Calculates `power(packet1[i], packet2[i])` for each index
- Randomly generated input data
- Compares results between serial and parallel methods

### Parallel Techniques Demonstrated
- **Shared Memory**: Using `fork()` and `mmap()` for inter-process communication
- **Message Passing**: Using named pipes (FIFOs) for data transfer
- **Multithreading**: Using POSIX threads (`pthreads`)

## Prerequisites

- GCC compiler
- POSIX-compliant system (Linux/Unix)
- Libraries: 
  - pthread
  - math

## Compilation

```bash
gcc -o parallel_computation parallel_computation.c -lm -lpthread
```

## Execution

```bash
./parallel_computation
```

## Performance Considerations

The program measures and prints execution times for:
- Serial computation
- Shared memory parallel computation
- Message passing parallel computation
- Multithreading parallel computation

## Note

Actual performance may vary based on:
- Hardware specifications
- System load
- Compiler optimizations

## License



## Author

[Yara Daraghmeh]# Parallel-Computation-Techniques-in-C
