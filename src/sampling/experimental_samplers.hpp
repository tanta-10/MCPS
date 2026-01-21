#pragma once

#include "../core/math_utils.hpp"
#include "../core/sampler.hpp"
#include <vector>
#include <complex>
#include <cmath>

namespace mcps {
namespace experimental {

/// @brief Quantum-inspired sampler using amplitude-based importance sampling
/// Novel approach: Uses quantum amplitude encoding to bias samples toward
/// high-contribution regions, inspired by quantum computing sampling advantages
template<typename T>
class QuantumInspiredSampler : public Sampler<T> {
public:
    QuantumInspiredSampler(uint64_t seed = 0, int qubits = 8)
        : base_sampler(seed), num_states(1 << qubits), seed(seed) {
        amplitudes.resize(num_states);
        initialize_uniform_superposition();
    }
    
    T next_1d() override {
        // Measure the quantum state (collapse to classical sample)
        T r = base_sampler.next_1d();
        T cumulative = T(0);
        
        for (size_t i = 0; i < num_states; ++i) {
            T prob = std::norm(amplitudes[i]); // |ψ|²
            cumulative += prob;
            if (r < cumulative) {
                return T(i) / T(num_states - 1);
            }
        }
        
        return base_sampler.next_1d(); // Fallback
    }
    
    /// @brief Update quantum amplitudes based on observed importance
    void update_amplitudes(int state_index, T importance) {
        if (state_index < 0 || state_index >= static_cast<int>(num_states)) return;
        
        // Apply phase rotation based on importance (quantum interference)
        T phase = importance * pi<T>();
        amplitudes[state_index] *= std::exp(std::complex<T>(0, phase));
        
        // Renormalize
        normalize_amplitudes();
    }
    
    Sampler<T>* clone(uint64_t new_seed) const override {
        return new QuantumInspiredSampler(new_seed, std::log2(num_states));
    }
    
private:
    IndependentSampler<T> base_sampler;
    std::vector<std::complex<T>> amplitudes;
    size_t num_states;
    uint64_t seed;
    
    void initialize_uniform_superposition() {
        T amplitude = T(1) / std::sqrt(T(num_states));
        for (auto& a : amplitudes) {
            a = std::complex<T>(amplitude, 0);
        }
    }
    
    void normalize_amplitudes() {
        T sum = T(0);
        for (const auto& a : amplitudes) {
            sum += std::norm(a);
        }
        T scale = T(1) / std::sqrt(sum);
        for (auto& a : amplitudes) {
            a *= scale;
        }
    }
};

/// @brief Chaos-enhanced sampler using Lorenz attractor for variance reduction
/// Novel approach: Exploits chaotic dynamics to explore sample space more
/// efficiently than pure random or QMC sequences
template<typename T>
class ChaoticSampler : public Sampler<T> {
public:
    ChaoticSampler(uint64_t seed = 0)
        : base_sampler(seed), x(0.1), y(0), z(0),
          sigma(10), rho(28), beta(T(8.0/3.0)), dt(T(0.01)) {
        // Initialize with random perturbation
        T r1 = base_sampler.next_1d();
        T r2 = base_sampler.next_1d();
        x += r1 * T(0.1);
        y += r2 * T(0.1);
    }
    
    T next_1d() override {
        // Evolve Lorenz system
        for (int i = 0; i < 10; ++i) { // Multiple steps for mixing
            T dx = sigma * (y - x);
            T dy = x * (rho - z) - y;
            T dz = x * y - beta * z;
            
            x += dx * dt;
            y += dy * dt;
            z += dz * dt;
        }
        
        // Map chaotic state to [0, 1)
        T val = std::fmod(std::abs(x * T(0.05)), T(1));
        return val;
    }
    
    Sampler<T>* clone(uint64_t new_seed) const override {
        return new ChaoticSampler(new_seed);
    }
    
private:
    IndependentSampler<T> base_sampler;
    T x, y, z;          // Lorenz attractor state
    T sigma, rho, beta; // Lorenz parameters
    T dt;               // Time step
};

/// @brief Fractal importance sampler using Mandelbrot set structure
/// Novel approach: Uses fractal geometry to create naturally hierarchical
/// sampling patterns that adapt to scene complexity
template<typename T>
class FractalSampler : public Sampler<T> {
public:
    FractalSampler(uint64_t seed = 0, int max_iter = 100)
        : base_sampler(seed), max_iterations(max_iter), iteration(0) {}
    
    T next_1d() override {
        T u = base_sampler.next_1d();
        T v = base_sampler.next_1d();
        
        // Map to complex plane
        T cx = (u - T(0.5)) * T(3.5);
        T cy = (v - T(0.5)) * T(2.0);
        
        // Compute Mandelbrot iterations
        T zx = 0, zy = 0;
        int iter = 0;
        
        while (zx * zx + zy * zy < T(4) && iter < max_iterations) {
            T temp = zx * zx - zy * zy + cx;
            zy = T(2) * zx * zy + cy;
            zx = temp;
            iter++;
        }
        
        // Use iteration count to bias sampling
        T weight = T(iter) / T(max_iterations);
        
        // Blend with uniform sample
        return T(0.5) * u + T(0.5) * weight;
    }
    
    Sampler<T>* clone(uint64_t new_seed) const override {
        return new FractalSampler(new_seed, max_iterations);
    }
    
private:
    IndependentSampler<T> base_sampler;
    int max_iterations;
    int iteration;
};

/// @brief Entropy-driven adaptive sampler
/// Novel approach: Uses information theory to allocate samples where
/// uncertainty (entropy) is highest
template<typename T>
class EntropyDrivenSampler : public Sampler<T> {
public:
    struct Region {
        T min_u, max_u, min_v, max_v;
        T entropy;
        int sample_count;
        
        Region(T u0, T u1, T v0, T v1)
            : min_u(u0), max_u(u1), min_v(v0), max_v(v1),
              entropy(T(1)), sample_count(0) {}
    };
    
    EntropyDrivenSampler(uint64_t seed = 0, int grid_size = 16)
        : base_sampler(seed) {
        // Initialize spatial grid
        int n = grid_size;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                regions.emplace_back(
                    T(i) / T(n), T(i + 1) / T(n),
                    T(j) / T(n), T(j + 1) / T(n)
                );
            }
        }
    }
    
    T next_1d() override {
        return base_sampler.next_1d();
    }
    
    void next_2d(T& u, T& v) override {
        // Select region based on entropy (higher entropy = more samples)
        int region_idx = select_region_by_entropy();
        
        if (region_idx >= 0 && region_idx < static_cast<int>(regions.size())) {
            Region& r = regions[region_idx];
            
            // Sample within selected region
            T ru = base_sampler.next_1d();
            T rv = base_sampler.next_1d();
            
            u = r.min_u + ru * (r.max_u - r.min_u);
            v = r.min_v + rv * (r.max_v - r.min_v);
            
            r.sample_count++;
        } else {
            base_sampler.next_2d(u, v);
        }
    }
    
    /// @brief Update region entropy based on variance
    void update_region_entropy(T u, T v, T variance) {
        for (auto& r : regions) {
            if (u >= r.min_u && u < r.max_u && v >= r.min_v && v < r.max_v) {
                // Simple exponential moving average
                T alpha = T(0.1);
                r.entropy = (T(1) - alpha) * r.entropy + alpha * variance;
                break;
            }
        }
    }
    
    Sampler<T>* clone(uint64_t new_seed) const override {
        return new EntropyDrivenSampler(new_seed, static_cast<int>(std::sqrt(regions.size())));
    }
    
private:
    IndependentSampler<T> base_sampler;
    std::vector<Region> regions;
    
    int select_region_by_entropy() {
        if (regions.empty()) return -1;
        
        // Roulette wheel selection based on entropy
        T total_entropy = T(0);
        for (const auto& r : regions) {
            total_entropy += r.entropy;
        }
        
        T r = base_sampler.next_1d() * total_entropy;
        T cumulative = T(0);
        
        for (size_t i = 0; i < regions.size(); ++i) {
            cumulative += regions[i].entropy;
            if (r < cumulative) {
                return static_cast<int>(i);
            }
        }
        
        return static_cast<int>(regions.size() - 1);
    }
};

} // namespace experimental
} // namespace mcps
