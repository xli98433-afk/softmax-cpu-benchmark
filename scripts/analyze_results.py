#!/usr/bin/env python3
"""Summarize raw benchmark samples; no third-party dependencies."""
import argparse
import csv
import statistics
from collections import defaultdict
from pathlib import Path


def analyze(csv_path: Path, output_dir: Path):
    groups = defaultdict(list)
    with csv_path.open(newline='', encoding='utf-8') as f:
        for row in csv.DictReader(f):
            groups[(int(row['size']), row['variant'])].append(float(row['ns_per_call']))
    if not groups:
        raise RuntimeError('No benchmark rows found')
    results = []
    sizes = sorted({size for size, _ in groups})
    for size in sizes:
        baseline_samples = groups.get((size, 'stable'))
        if not baseline_samples:
            raise RuntimeError(f'Missing stable baseline for size {size}')
        baseline = statistics.median(baseline_samples)
        for variant in ('naive', 'stable', 'stable_cached', 'online'):
            samples = groups.get((size, variant))
            if not samples:
                raise RuntimeError(f'Missing {variant} samples for size {size}')
            med = statistics.median(samples)
            results.append(dict(size=size, variant=variant, median_ns_per_call=med,
                                median_us_per_call=med / 1000,
                                relative_to_stable=baseline / med, samples=len(samples)))
    output_dir.mkdir(parents=True, exist_ok=True)
    with (output_dir / 'summary.csv').open('w', encoding='utf-8', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=list(results[0].keys()))
        writer.writeheader()
        writer.writerows(results)
    md = [
        '# Benchmark results (measured, environment-specific)', '',
        'Median runtime across repeated samples. **Speedup = stable baseline time / candidate time**; '
        'values below 1.0 mean the candidate is slower. The naive variant is **not numerically safe**.', '',
        '| Size | Variant | Median (µs / call) | Relative to stable | Samples |',
        '|---:|---|---:|---:|---:|',
    ]
    for row in results:
        md.append(f"| {row['size']} | {row['variant']} | {row['median_us_per_call']:.3f} | "
                  f"{row['relative_to_stable']:.2f}x | {row['samples']} |")
    md.extend(['', '## Interpretation notes', '',
               '- This is an educational CPU benchmark, not a Huawei Kunpeng or hardware-specific result.',
               '- Results depend on compiler/CPU load/temperature, and include vector allocation overhead.',
               '- `online` refers to the online *normalizer algorithm*, NOT an AI agent.',
               '- `stable_cached` caches exponentials in an output buffer; it is not SIMD-optimized.',
               '- The code does not use Google Highway and makes no SIMD speedup claims.',
               '- Compare only measurements generated in the same environment and rerun to confirm.', ''])
    (output_dir / 'summary.md').write_text('\n'.join(md), encoding='utf-8')
    print(f'Wrote {output_dir / "summary.csv"} and {output_dir / "summary.md"}')


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--csv', type=Path, default=Path('results/benchmarks.csv'))
    ap.add_argument('--out', type=Path, default=Path('results'))
    a = ap.parse_args()
    analyze(a.csv, a.out)
