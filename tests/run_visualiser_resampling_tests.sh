#!/bin/sh
set -eu
module_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/osci-resampling-tests.XXXXXX")
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM
"${CXX:-clang++}" -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$module_dir/third_party/chowdsp_utils/modules/dsp/chowdsp_simd/third_party/xsimd/include" \
    "$module_dir/tests/VisualiserResamplingTests.cpp" -o "$build_dir/tests"
"$build_dir/tests"
