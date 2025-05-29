#!/bin/bash

set -e
cd "$(dirname "$0")"

mkdir -p build_wasm
cd build_wasm

emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
