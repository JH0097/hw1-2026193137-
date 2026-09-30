CC ?= cc
CFLAGS ?= -std=c17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -Isrc
PYTHON ?= python3
COMMON = src/sort.c src/input.c
HEADERS = src/sort.h src/input.h
.PHONY: all run test bench sanitize
all: build/bench.out build/stats.out
build:
	mkdir -p build
build/bench.out: src/main.c $(COMMON) $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c $(COMMON) -o $@
build/stats.out: src/main.c $(COMMON) $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DCOUNT_STATS src/main.c $(COMMON) -o $@
build/test.out: tests/test_sort.c $(COMMON) $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -DCOUNT_STATS tests/test_sort.c $(COMMON) -o $@
build/test-plain.out: tests/test_sort.c $(COMMON) $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_sort.c $(COMMON) -o $@
test: build/test.out build/test-plain.out
	./build/test.out
	./build/test-plain.out
run: build/bench.out
	./build/bench.out
bench: all
	$(PYTHON) tools/run_benchmark.py --compiler "$(CC)" --flags "$(CPPFLAGS) $(CFLAGS)"
sanitize: | build
	$(CC) $(CPPFLAGS) -std=c17 -O1 -g -Wall -Wextra -DCOUNT_STATS -fsanitize=address,undefined -fno-omit-frame-pointer tests/test_sort.c $(COMMON) -o build/sanitize.out
	./build/sanitize.out
