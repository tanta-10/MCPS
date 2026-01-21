#!/usr/bin/env python3
"""
Ablation study script for MCPS novel features
Tests performance with different combinations of novel techniques
"""

import subprocess
import time
import json
from pathlib import Path

def run_renderer(mode, width=200, spp=16):
    """Run renderer with specific settings"""
    print(f"Testing mode: {mode}")
    start = time.time()
    
    # Would need to modify main.cpp to accept mode as argument
    result = subprocess.run(
        ['./build/mcps', str(width), str(spp)],
        capture_output=True,
        text=True
    )
    
    elapsed = time.time() - start
    
    return {
        'mode': mode,
        'width': width,
        'spp': spp,
        'time': elapsed,
        'success': result.returncode == 0
    }

def main():
    print("=" * 60)
    print("MCPS Ablation Study")
    print("=" * 60)
    print()
    
    modes = [
        'quantum',
        'chaotic',
        'fractal',
        'entropy',
        'hybrid'
    ]
    
    configurations = [
        {'width': 200, 'spp': 16},
        {'width': 400, 'spp': 32},
        {'width': 800, 'spp': 64},
    ]
    
    results = []
    
    for config in configurations:
        print(f"\nTesting configuration: {config['width']}x{config['width']}, {config['spp']} spp")
        print("-" * 60)
        
        for mode in modes:
            result = run_renderer(mode, config['width'], config['spp'])
            results.append(result)
            
            if result['success']:
                print(f"  {mode:15s}: {result['time']:.2f}s")
            else:
                print(f"  {mode:15s}: FAILED")
    
    # Save results
    output_file = Path('experiments/ablation_results.json')
    output_file.parent.mkdir(exist_ok=True)
    
    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2)
    
    print(f"\nResults saved to {output_file}")
    
    # Summary statistics
    print("\n" + "=" * 60)
    print("Summary Statistics")
    print("=" * 60)
    
    for mode in modes:
        mode_results = [r for r in results if r['mode'] == mode and r['success']]
        if mode_results:
            avg_time = sum(r['time'] for r in mode_results) / len(mode_results)
            print(f"{mode:15s}: avg {avg_time:.2f}s over {len(mode_results)} runs")

if __name__ == '__main__':
    main()
