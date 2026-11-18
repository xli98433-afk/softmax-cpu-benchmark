#!/usr/bin/env python3
"""Build, test, benchmark and generate reproducible report (Python stdlib only)."""
import argparse
import json
import platform
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run(args):
    print('+', ' '.join(map(str, args)), flush=True)
    subprocess.run([str(x) for x in args], check=True, cwd=ROOT)


def main():
    p = argparse.ArgumentParser(description='Run the Softmax benchmark end-to-end')
    p.add_argument('--sizes', default='256,4096,65536,1048576')
    p.add_argument('--repeats', type=int, default=7)
    args = p.parse_args()
    build = ROOT / 'build'
    report = ROOT / 'results'
    report.mkdir(exist_ok=True)
    run(['cmake', '-S', ROOT, '-B', build, '-DCMAKE_BUILD_TYPE=Release'])
    run(['cmake', '--build', build, '--config', 'Release', '-j', '2'])
    run(['ctest', '--test-dir', build, '--output-on-failure', '-C', 'Release'])
    binary = build / ('Release/softmax_bench.exe' if platform.system() == 'Windows' else 'softmax_bench')
    if not binary.exists():
        binary = build / 'Release' / 'softmax_bench.exe'
    run([binary, '--sizes', args.sizes, '--repeats', args.repeats,
         '--output', report / 'benchmarks.csv'])
    run([sys.executable, ROOT / 'scripts' / 'analyze_results.py',
         '--csv', report / 'benchmarks.csv', '--out', report])
    cmake = shutil.which('cmake')
    cxx = shutil.which('g++') or shutil.which('clang++') or shutil.which('cl')
    def version(cmd):
        if not cmd: return 'unavailable'
        try: return subprocess.check_output([cmd, '--version'], text=True, stderr=subprocess.STDOUT, timeout=10).splitlines()[0]
        except (OSError, subprocess.SubprocessError): return 'unavailable'
    cpu_model = platform.processor() or 'unknown'
    if platform.system() == 'Linux':
        try:
            for line in Path('/proc/cpuinfo').read_text(errors='replace').splitlines():
                if line.lower().startswith(('model name', 'hardware')) and ':' in line:
                    cpu_model = line.split(':', 1)[1].strip()
                    break
        except OSError:
            pass
    env = {
        'generated_at_utc': datetime.now(timezone.utc).isoformat(),
        'os': platform.system(), 'architecture': platform.machine(),
        'cpu_model': cpu_model,
        'python': platform.python_version(), 'cmake': version(cmake),
        'compiler': version(cxx), 'build_type': 'Release',
        'sizes': args.sizes, 'repeats': args.repeats,
        'note': 'Generic runner CPU; not Huawei Kunpeng or a platform endorsement.'
    }
    (report / 'environment.json').write_text(json.dumps(env, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    print('Complete. View results/summary.md and results/environment.json')


if __name__ == '__main__':
    main()
