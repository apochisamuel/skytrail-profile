CC ?= clang
CXX ?= clang++
CFLAGS ?= -std=c11 -O1 -g -Wall -Wextra -Werror
CXXFLAGS ?= -std=c++17 -O1 -g -Wall -Wextra -Werror

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:.c=.o)
LIBRARY := libskytrail.a

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

src/%.o: src/%.c src/internal.h src/skytrail.h
	$(CC) $(CFLAGS) -Isrc -c -o $@ $<

test_basic: tests/test_basic.c $(LIBRARY)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/test_basic.c $(LIBRARY)

test: test_basic
	./test_basic fuzz/corpus/parse_fuzzer/seed_variant_1.sbp

fuzzers: $(LIBRARY)
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -Isrc -o parse_fuzzer fuzz/parse_fuzzer.cc $(LIBRARY)
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -Isrc -o graph_fuzzer fuzz/graph_fuzzer.cc $(LIBRARY)
	$(CXX) $(CXXFLAGS) -fsanitize=fuzzer,address -Isrc -o query_fuzzer fuzz/query_fuzzer.cc $(LIBRARY)

clean:
	rm -f src/*.o *.o $(LIBRARY) test_basic parse_fuzzer graph_fuzzer query_fuzzer

inspect: tools/inspect.c $(LIBRARY)
	$(CC) $(CFLAGS) -Isrc -o $@ tools/inspect.c $(LIBRARY)

gen_seeds: tools/gen_seeds.c
	$(CC) $(CFLAGS) -o $@ tools/gen_seeds.c
