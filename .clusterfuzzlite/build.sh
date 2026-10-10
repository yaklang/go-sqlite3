#!/bin/bash -eu

cd "$SRC/go-sqlite3"

# The existing fuzz/example imports use the upstream path. Resolve them to
# this checkout without changing the fork's published module or Go API.
go mod edit -require=github.com/mattn/go-sqlite3@v1.14.48 -replace=github.com/mattn/go-sqlite3=.
compile_go_fuzzer github.com/mattn/go-sqlite3/_example/fuzz FuzzOpenExec fuzz_open_exec

go run _example/simple/simple.go
printf 'select id, name from foo\0' > capture.fuzc
cat foo.db >> capture.fuzc
zip -j "$OUT/fuzz_open_exec_seed_corpus.zip" capture.fuzc
