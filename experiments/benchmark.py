#!/usr/bin/env python3
"""
Performance benchmark script for MCPS renderer
Measures rendering time, memory usage, and convergence rate
"""

import subprocess
import time
import psutil
import json
from pathlib import Path

def benchmark_renderer(width, spp, runs=3):
    """Benchmark renderer with given settings"""
    times = []
    memory_usage = []
    
    for run in range(runs):
        print(f"  Run {run + 1}/{runs}...", end=' ', flush=True)
        
        process = subprocess.Popen(
            ['./build/mcps', str(width), str(spp)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        start = time.time()
        max_memory = 0
        
        # Monitor process
        try:
            ps_process = psutil.Process(process.pid)
            while process.poll() is None:
                try:
                    mem_info = ps_process.memory_info()
                    max_memory = max(max_memory, mem_info.rss / 1024 / 1024)  # MB
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    break
                time.sleep(0.1)
        except Exception:
            pass
        
        process.wait()
        elapsed = time.time() - start
        
        times.append(elapsed)
        memory_usage.append(max_memory)
        
        print(f"{elapsed:.2f}s, {max_memory:.1f}MB")
    
    return {
        'avg_time': sum(times) / len(times),
        'min_time': min(times),
        'max_time': max(times),
        'avg_memory_mb': sum(memory_usage) / len(memory_usage),
        'max_memory_mb': max(memory_usage)
    }

def main():
    print("=" * 60)
    print("MCPS Performance Benchmark")
    print("=" * 60)
    print()
    
    configurations = [
        {'width': 100, 'spp': 8, 'label': 'Small (100x100, 8 spp)'},
        {'width': 200, 'spp': 16, 'label': 'Medium (200x200, 16 spp)'},
        {'width': 400, 'spp': 32, 'label': 'Large (400x400, 32 spp)'},
        {'width': 800, 'spp': 64, 'label': 'XLarge (800x800, 64 spp)'},
    ]
    
    results = {}
    
    for config in configurations:
        print(f"\nBenchmarking {config['label']}:")
        print("-" * 60)
        
        result = benchmark_renderer(config['width'], config['spp'], runs=3)
        
        results[config['label']] = {
            **config,
            **result
        }
        
        print(f"\nResults for {config['label']}:")
        print(f"  Average time:   {result['avg_time']:.2f}s")
        print(f"  Time range:     {result['min_time']:.2f}s - {result['max_time']:.2f}s")
        print(f"  Average memory: {result['avg_memory_mb']:.1f}MB")
        print(f"  Peak memory:    {result['max_memory_mb']:.1f}MB")
    
    # Save results
    output_file = Path('experiments/benchmark_results.json')
    output_file.parent.mkdir(exist_ok=True)
    
    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2)
    
    print(f"\n\nResults saved to {output_file}")
    
    # Print summary table
    print("\n" + "=" * 60)
    print("Performance Summary")
    print("=" * 60)
    print(f"{'Configuration':<25} {'Avg Time':<12} {'Peak Mem':<12}")
    print("-" * 60)
    
    for label, data in results.items():
        print(f"{label:<25} {data['avg_time']:>10.2f}s {data['max_memory_mb']:>10.1f}MB")

if __name__ == '__main__':
    main()
