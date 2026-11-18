# Softmax CPU Benchmark

**C++17 · CMake · Python | Reproducible numerical stability and CPU performance study**

A small AI-assisted learning project comparing four CPU Softmax implementations, correctness tests and repeatable performance measurements. It is motivated by [Online normalizer calculation for softmax](https://arxiv.org/abs/1805.02867) and prior exploration of [Google Highway](https://github.com/google/highway).

> Important: This code does **not** use Google Highway, explicit SIMD, an AI Agent, or Huawei Kunpeng hardware. Benchmark measurements below come from a Linux x86-64 machine, not a Kunpeng system.

## Features

| Variant | Description | Numerically stable? |
| --- | --- | --- |
| naive | Direct exp(x) and normalization | No |
| stable | Subtract max, denominator pass, output pass | Yes |
| stable_cached | Cache exponentials before in-place normalization | Yes |
| online | Online max/normalizer update, then output | Yes |

All versions accept nonempty finite float vectors; accumulation is in double, outputs are float. Tests cover expected outputs, randomized inputs, overflow cases, empty inputs, NaN and infinity.

## Build and run

Requirements: C++17 compiler, CMake 3.16+, Python 3.8+. No third-party Python packages.

Linux/macOS: run `python3 scripts/run_experiment.py`.
Windows: run `py -3 scripts/run_experiment.py`.

Or build and run tests directly:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    ctest --test-dir build --output-on-failure -C Release

Quick benchmark:

    python3 scripts/run_experiment.py --sizes 128,2048 --repeats 3

The experiment script runs Release build, tests, benchmark, and data analysis, then generates the CSV and Markdown results.

## Measured sample results

Seven repeats per variant, median time. Baseline is the deliberately two-exp-pass stable version, **not** an optimized industry library.

| Elements | stable (us/call) | stable_cached (us/call) | Relative speed |
| ---: | ---: | ---: | ---: |
| 256 | 3.535 | 1.865 | 1.90x |
| 4096 | 57.050 | 30.525 | 1.87x |
| 65536 | 976.676 | 503.903 | 1.94x |
| 1048576 | 15571.607 | 8107.152 | 1.92x |

![Measured runtime comparison](docs/benchmark.svg)

Raw samples: `results/benchmarks.csv`; medians: `results/summary.csv` and `results/summary.md`; measurement environment: `results/environment.json`.

**Interpretation:** Cached exponentials avoid repeated calls to exp() but change memory traffic and float rounding; the benchmark includes allocations and is machine-dependent. These figures are **not SIMD acceleration** and are **not measurements on Huawei hardware**. Rerun before quoting a result.

## Project layout

- `src/softmax.cpp`, `include/softmax.hpp`: four implementation variants
- `tests/test_softmax.cpp`: numerical correctness and input validation
- `src/benchmark.cpp`: repeatable benchmark with fixed input seed
- `scripts/run_experiment.py`: build/test/benchmark orchestration
- `scripts/analyze_results.py`: median and relative speed calculation
- `results/`: measured sample outputs and environment
- `.github/workflows/ci.yml`: GitHub Actions C++ correctness and smoke benchmark

## Learning notes

- Softmax formula: p_i = exp(x_i - m) / sum_j exp(x_j - m), with m = max(x).
- Max subtraction protects the exponentials from overflow on finite inputs.
- Comparison is meaningful only with the same compiler, hardware and benchmark settings.
- The `online` algorithm is an online normalizer, not an AI Agent.
- The project is AI-assisted; it is intended as a reproducible learning exercise.

References: [Milakov & Gimelshein 2018](https://arxiv.org/abs/1805.02867); [Google Highway](https://github.com/google/highway) (related research, not used as a dependency).

MIT license; see LICENSE.
