#!/bin/bash -eu
# Copy fuzzer executables to $OUT/
$CXX $CXXFLAGS -fsanitize=fuzzer $SRC/structify/.clusterfuzzlite/reformat_fuzzer.cpp -o $OUT/reformat_fuzzer -I$SRC/structify/include
