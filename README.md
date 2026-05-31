# Data Structures in C

Self-paced implementation of classical data structures in C.
Focus on systems/server track preparation: clean modular design,
memory safety, and clear documentation.

## Implementations

| Data Structure  | Status   | Operations                          |
|-----------------|----------|-------------------------------------|
| AVL Tree        | Complete | insert, delete, search, destroy     |
| 2-3 Tree        | Complete | insert, delete, search, destroy     |
| Red-Black Tree  | Complete | insert, delete, search, destroy     |
| Trie            | Complete | insert, delete, search, destroy     |

## Planned

- Dynamic array (vector) — `realloc`, amortized analysis
- Hash table — chaining + open addressing
- B-tree — DB index, filesystem

## Build conventions

Every project uses the same Makefile structure:

```bash
make            # build
make run        # build and run unit tests
make valgrind   # run tests under valgrind
make clean      # remove build artifacts
```

Compile flags: `-Wall -Wextra -Wpedantic -std=c11 -g -O0`

## Environment

- WSL2 Ubuntu 24.04
- gcc 13.2
- valgrind 3.22
