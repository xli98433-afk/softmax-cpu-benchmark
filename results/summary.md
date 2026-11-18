# Benchmark results (measured, environment-specific)

Median runtime across repeated samples. **Speedup = stable baseline time / candidate time**; values below 1.0 mean the candidate is slower. The naive variant is **not numerically safe**.

| Size | Variant | Median (µs / call) | Relative to stable | Samples |
|---:|---|---:|---:|---:|
| 256 | naive | 2.903 | 1.22x | 7 |
| 256 | stable | 3.535 | 1.00x | 7 |
| 256 | stable_cached | 1.865 | 1.90x | 7 |
| 256 | online | 3.024 | 1.17x | 7 |
| 4096 | naive | 44.402 | 1.28x | 7 |
| 4096 | stable | 57.050 | 1.00x | 7 |
| 4096 | stable_cached | 30.525 | 1.87x | 7 |
| 4096 | online | 46.097 | 1.24x | 7 |
| 65536 | naive | 706.544 | 1.38x | 7 |
| 65536 | stable | 976.676 | 1.00x | 7 |
| 65536 | stable_cached | 503.903 | 1.94x | 7 |
| 65536 | online | 797.265 | 1.23x | 7 |
| 1048576 | naive | 11747.666 | 1.33x | 7 |
| 1048576 | stable | 15571.607 | 1.00x | 7 |
| 1048576 | stable_cached | 8107.152 | 1.92x | 7 |
| 1048576 | online | 11747.528 | 1.33x | 7 |

## Interpretation notes

- This is an educational CPU benchmark, not a Huawei Kunpeng or hardware-specific result.
- Results depend on compiler/CPU load/temperature, and include vector allocation overhead.
- `online` refers to the online *normalizer algorithm*, NOT an AI agent.
- `stable_cached` caches exponentials in an output buffer; it is not SIMD-optimized.
- The code does not use Google Highway and makes no SIMD speedup claims.
- Compare only measurements generated in the same environment and rerun to confirm.
