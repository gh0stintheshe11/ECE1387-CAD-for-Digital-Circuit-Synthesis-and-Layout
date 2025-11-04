import subprocess
import re
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# ============================================================
# CONFIGURATION - CHANGE THESE VALUES
# ============================================================
CIRCUIT_FILE = 'cct3.txt'        # Options: 'cct2.txt', 'cct3.txt'
SPREAD_MODE = 'he'               # Options: 'ho' (homogeneous), 'he' (heterogeneous)

# Parameter ranges to sweep
PSI_INIT_VALUES = [0.5, 1.0, 2.0, 4.0, 6.0, 8.0, 10.0]
PSI_INCR_VALUES = [0.1, 0.25, 0.5, 1.0, 2.0, 3.0, 4.0]

# Timeout per run (seconds)
TIMEOUT = 60
# ============================================================

def run_placer(circuit, mode, psi_init, psi_incr):
    """Run placer and extract results"""
    cmd = [
        './placer',
        '-f', circuit,
        '-s', mode,
        '--psi-init', str(psi_init),
        '--psi-incr', str(psi_incr)
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        output = result.stdout
        
        # Parse output
        converged = '✓ All bins meet capacity constraints!' in output
        
        # Extract iterations
        iterations_match = re.search(r'Iterations:\s+(\d+)', output)
        iterations = int(iterations_match.group(1)) if iterations_match and converged else None
        
        # Extract final psi
        final_psi_match = re.search(r'Final ψ:\s+([\d.]+)', output)
        final_psi = float(final_psi_match.group(1)) if final_psi_match and converged else None
        
        # Extract displacement
        displacement_match = re.search(r'Total Cell Displacement:\s+([\d.]+)', output)
        displacement = float(displacement_match.group(1)) if displacement_match and converged else None
        
        # Extract HPWL before spreading
        hpwl_before_match = re.search(r'Before spreading:\s+([\d.]+)', output)
        hpwl_before = float(hpwl_before_match.group(1)) if hpwl_before_match and converged else None
        
        # Extract HPWL after spreading
        hpwl_after_match = re.search(r'After spreading:\s+([\d.]+)', output)
        hpwl_after = float(hpwl_after_match.group(1)) if hpwl_after_match and converged else None
        
        return {
            'converged': converged,
            'iterations': iterations,
            'final_psi': final_psi,
            'displacement': displacement,
            'hpwl_before': hpwl_before,
            'hpwl_after': hpwl_after
        }
    except subprocess.TimeoutExpired:
        print(f" Timeout after {TIMEOUT}s")
        return {'converged': False, 'iterations': None, 'final_psi': None, 
                'displacement': None, 'hpwl_before': None, 'hpwl_after': None}
    except Exception as e:
        print(f" Error: {e}")
        return {'converged': False, 'iterations': None, 'final_psi': None, 
                'displacement': None, 'hpwl_before': None, 'hpwl_after': None}

def sweep_parameters(circuit, mode):
    """Sweep psi parameters and collect results"""
    
    results = []
    total = len(PSI_INIT_VALUES) * len(PSI_INCR_VALUES)
    count = 0
    
    print(f"\n{'='*60}")
    print(f"Starting sweep for: {circuit} ({mode})")
    print(f"Testing {total} parameter combinations")
    print(f"{'='*60}\n")
    
    for psi_init in PSI_INIT_VALUES:
        for psi_incr in PSI_INCR_VALUES:
            count += 1
            print(f"[{count}/{total}] psi_init={psi_init:.2f}, psi_incr={psi_incr:.2f}...", end='')
            
            result = run_placer(circuit, mode, psi_init, psi_incr)
            result['psi_init'] = psi_init
            result['psi_incr'] = psi_incr
            result['circuit'] = circuit
            result['mode'] = mode
            
            if result['converged']:
                print(f" Converged in {result['iterations']} iter, disp={result['displacement']:.1f}, HPWL={result['hpwl_after']:.1f}")
            else:
                print(" Did not converge")
            
            results.append(result)
    
    return pd.DataFrame(results)

def plot_results(df, circuit, mode):
    """Create 4 plots: iterations, displacement, HPWL before, HPWL after"""
    
    fig, axes = plt.subplots(2, 2, figsize=(16, 10))
    mode_name = 'Homogeneous' if mode == 'ho' else 'Heterogeneous'
    fig.suptitle(f'Spreading Parameter Sweep: {circuit} ({mode_name})', 
                 fontsize=16, fontweight='bold')
    
    # Plot 1: Iterations vs psi_init
    ax = axes[0, 0]
    for incr in sorted(df['psi_incr'].unique()):
        data = df[df['psi_incr'] == incr].sort_values('psi_init')
        data_conv = data[data['iterations'].notna()]
        if len(data_conv) > 0:
            ax.plot(data_conv['psi_init'], data_conv['iterations'], marker='o', linewidth=2, 
                    markersize=6, label=f'incr={incr}')
    ax.set_xlabel('ψ initial', fontsize=12)
    ax.set_ylabel('Iterations to Converge', fontsize=12)
    ax.set_title('Iterations vs Initial ψ', fontsize=12, fontweight='bold')
    ax.legend(title='ψ increment', fontsize=9, loc='best')
    ax.grid(True, alpha=0.3)
    
    # Plot 2: Displacement vs psi_init
    ax = axes[0, 1]
    for incr in sorted(df['psi_incr'].unique()):
        data = df[df['psi_incr'] == incr].sort_values('psi_init')
        data_conv = data[data['displacement'].notna()]
        if len(data_conv) > 0:
            ax.plot(data_conv['psi_init'], data_conv['displacement'], marker='o', linewidth=2,
                    markersize=6, label=f'incr={incr}')
    ax.set_xlabel('ψ initial', fontsize=12)
    ax.set_ylabel('Total Cell Displacement', fontsize=12)
    ax.set_title('Displacement vs Initial ψ', fontsize=12, fontweight='bold')
    ax.legend(title='ψ increment', fontsize=9, loc='best')
    ax.grid(True, alpha=0.3)
    
    # Plot 3: HPWL Before Spreading vs psi_init
    ax = axes[1, 0]
    for incr in sorted(df['psi_incr'].unique()):
        data = df[df['psi_incr'] == incr].sort_values('psi_init')
        data_conv = data[data['hpwl_before'].notna()]
        if len(data_conv) > 0:
            ax.plot(data_conv['psi_init'], data_conv['hpwl_before'], marker='o', linewidth=2,
                    markersize=6, label=f'incr={incr}')
    ax.set_xlabel('ψ initial', fontsize=12)
    ax.set_ylabel('HPWL Before Spreading', fontsize=12)
    ax.set_title('HPWL Before Spreading vs Initial ψ', fontsize=12, fontweight='bold')
    ax.legend(title='ψ increment', fontsize=9, loc='best')
    ax.grid(True, alpha=0.3)
    
    # Plot 4: HPWL After Spreading vs psi_init
    ax = axes[1, 1]
    for incr in sorted(df['psi_incr'].unique()):
        data = df[df['psi_incr'] == incr].sort_values('psi_init')
        data_conv = data[data['hpwl_after'].notna()]
        if len(data_conv) > 0:
            ax.plot(data_conv['psi_init'], data_conv['hpwl_after'], marker='o', linewidth=2,
                    markersize=6, label=f'incr={incr}')
    ax.set_xlabel('ψ initial', fontsize=12)
    ax.set_ylabel('HPWL After Spreading', fontsize=12)
    ax.set_title('HPWL After Spreading vs Initial ψ', fontsize=12, fontweight='bold')
    ax.legend(title='ψ increment', fontsize=9, loc='best')
    ax.grid(True, alpha=0.3)
    
    total_tested = len(df)
    converged_total = len(df[df['converged'] == True])
    
    print(f"\n{'='*60}")
    print(f"Convergence rate: {converged_total}/{total_tested} ({100*converged_total/total_tested:.1f}%)")
    print(f"{'='*60}")
    
    plt.tight_layout()
    filename = f'sweep_{circuit.replace(".txt", "")}_{mode}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Plot saved: {filename}\n")
    plt.show()

def find_best_strategy(df):
    """Find the best converged strategy"""
    df_conv = df[df['converged'] == True].copy()
    
    if len(df_conv) == 0:
        print("\nWARNING: No converged strategies found!")
        print("Try increasing MAX_ITERATIONS in spreader.cpp or use larger psi values.\n")
        return None
    
    # Sort by displacement (ascending)
    df_sorted = df_conv.sort_values('displacement')
    best = df_sorted.iloc[0]
    
    print("\n" + "="*80)
    print("BEST STRATEGY:")
    print("="*80)
    print(f"psi_init={best['psi_init']:.2f}, psi_incr={best['psi_incr']:.2f}")
    print(f"Iterations: {best['iterations']}, Displacement: {best['displacement']:.1f}")
    print(f"HPWL: {best['hpwl_before']:.1f} → {best['hpwl_after']:.1f} (+{(best['hpwl_after']-best['hpwl_before'])/best['hpwl_before']*100:.1f}%)")
    print("="*80)
    
    return best

if __name__ == '__main__':
    print("\n" + "="*60)
    print("PSI PARAMETER SWEEP TOOL")
    print("="*60)
    print(f"Circuit: {CIRCUIT_FILE}")
    print(f"Mode: {SPREAD_MODE} ({'Homogeneous' if SPREAD_MODE == 'ho' else 'Heterogeneous'})")
    print(f"psi_init range: {PSI_INIT_VALUES}")
    print(f"psi_incr range: {PSI_INCR_VALUES}")
    print(f"Timeout per run: {TIMEOUT}s")
    
    # Run sweep
    df = sweep_parameters(CIRCUIT_FILE, SPREAD_MODE)
    
    # Save results
    csv_filename = f'sweep_results_{CIRCUIT_FILE.replace(".txt", "")}_{SPREAD_MODE}.csv'
    df.to_csv(csv_filename, index=False)
    print(f"\nResults saved to: {csv_filename}")
    
    # Find best strategy
    best = find_best_strategy(df)
    
    # Create plots
    plot_results(df, CIRCUIT_FILE, SPREAD_MODE)
    
    print("\nSweep complete!\n")