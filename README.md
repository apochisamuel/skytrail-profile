# Skytrail Profile Library

Skytrail is a C library for decoding Skytrail Binary Profiles (`.sbp`). The format models named nodes, timed paths, membership grids, annotations, palettes, and link records through a directory-based binary container.

## Build

```bash
make
make test
```

ClusterFuzzLite builds use the repository root as `SRC` and emit fuzzing binaries into `OUT`:

```bash
SRC="$PWD" OUT="$PWD/out" CC=clang CXX=clang++ \
CFLAGS='-O1 -g' CXXFLAGS='-O1 -g' \
LIB_FUZZING_ENGINE='-fsanitize=fuzzer,address' ./build.sh
```

## Public API

`src/skytrail.h` provides memory-based profile opening, information lookup, derived weights, reporting, and deterministic profile walking. The library does not retain the input buffer after `sf_profile_close`.

## Fuzz Targets

- `parse_fuzzer` runs the full binary pipeline and public walk interface.
- `graph_fuzzer` exercises graph, path, and timeline queries.
- `query_fuzzer` exercises lookup, reporting, annotation, and palette queries.

Each target has a small ordinary seed corpus under `fuzz/corpus/`.
