# Data Structures in C

Self-paced implementation of classical data structures in C.
Focus on systems/server track preparation: clean modular design,
memory safety, and clear documentation.

## Implementations

| Data Structure       | Status   | Operations / Notes                              |
|----------------------|----------|-------------------------------------------------|
| AVL Tree             | Complete | insert, delete, search, destroy                 |
| 2-3 Tree             | Complete | insert, delete, search, destroy                 |
| Red-Black Tree       | Complete | insert, delete, search, destroy                 |
| Trie                 | Complete | insert, delete, search, destroy                 |
| Hash Table (generic) | Complete | chaining + open addressing, library owns nodes  |
| Hash Table (intrusive)| Complete | `container_of`, kernel-style, caller owns nodes |
| k-d Tree             | Complete | insert, nearest-neighbor search with pruning    |

The two hash tables live under `hash/` and share one interface but differ in
who owns the nodes: the generic version allocates and copies keys/values
into library-owned nodes, while the intrusive version embeds the link in the
caller's own struct (the pattern the Linux kernel uses for `list_head`,
`hlist_node`, `rb_node`).

## Planned

- Dynamic array (vector) — `realloc`, amortized analysis
- Min/max heap — binary heap, `heapify`, priority queue
- B-tree — DB index, filesystem on-disk layout
- Graph — adjacency list, BFS/DFS, shortest paths
- Skip list — probabilistic balancing as an alternative to balanced BSTs

## Build conventions

Every project uses the same Makefile structure:

```bash
make            # build
make run        # build and run unit tests
make valgrind   # run tests under valgrind
make clean      # remove build artifacts
```

Compile flags: `-Wall -Wextra -Wpedantic -std=c11 -g -O0`

Every implementation is checked for warnings under these flags and for leaks
under valgrind before being marked complete.

## Layout

```
data-structures/
├── avl-tree/
├── two-three-tree/
├── red-black-tree/
├── trie/
├── hash/
│   ├── generic/      # library owns nodes, copies key/value
│   └── intrusive/    # caller owns nodes, container_of
└── kd-tree/
```

## Environment

- WSL2 Ubuntu 24.04
- gcc 13.2
- valgrind 3.22
