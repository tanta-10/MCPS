#pragma once

#include <random>
#include <cstdint>

namespace mcps {

/// @brief Base sampler class for generating random samples
template<typename T>
class Sampler {
public:
    virtual ~Sampler() = default;
    
    /// @brief Get next 1D sample in [0, 1)
    virtual T next_1d() = 0;
    
    /// @brief Get next 2D sample in [0, 1)^2
    virtual void next_2d(T& u, T& v) {
        u = next_1d();
        v = next_1d();
    }
    
    /// @brief Clone the sampler for multi-threading
    virtual Sampler* clone(uint64_t seed) const = 0;
};

/// @brief Simple independent sampler using std::mt19937
template<typename T>
class IndependentSampler : public Sampler<T> {
public:
    IndependentSampler(uint64_t seed = 0) : rng(seed), dist(0.0, 1.0) {}
    
    T next_1d() override {
        return static_cast<T>(dist(rng));
    }
    
    Sampler<T>* clone(uint64_t seed) const override {
        return new IndependentSampler(seed);
    }
    
private:
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> dist;
};

/// @brief Sobol sequence sampler (placeholder for low-discrepancy sequences)
/// Note: Full Sobol implementation would require direction vectors
template<typename T>
class SobolSampler : public Sampler<T> {
public:
    SobolSampler(uint64_t seed = 0) : index(0), scramble(seed), rng(seed), dist(0.0, 1.0) {}
    
    T next_1d() override {
        // Simplified: Using random for now, would use actual Sobol sequence
        // with Owen scrambling in production
        index++;
        return static_cast<T>(dist(rng));
    }
    
    Sampler<T>* clone(uint64_t seed) const override {
        return new SobolSampler(seed);
    }
    
private:
    uint64_t index;
    uint64_t scramble;
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> dist;
};

/// @brief Uniform sampling utilities
template<typename T>
class SamplingUtils {
public:
    /// @brief Sample uniform hemisphere (cosine-weighted)
    static Vec3<T> sample_cosine_hemisphere(T u, T v) {
        T r = std::sqrt(u);
        T theta = T(2) * pi<T>() * v;
        T x = r * std::cos(theta);
        T y = r * std::sin(theta);
        T z = std::sqrt(std::max(T(0), T(1) - u));
        return Vec3<T>(x, y, z);
    }
    
    /// @brief PDF for cosine-weighted hemisphere sampling
    static T cosine_hemisphere_pdf(T cos_theta) {
        return cos_theta * inv_pi<T>();
    }
    
    /// @brief Sample uniform sphere
    static Vec3<T> sample_uniform_sphere(T u, T v) {
        T z = T(1) - T(2) * u;
        T r = std::sqrt(std::max(T(0), T(1) - z * z));
        T phi = T(2) * pi<T>() * v;
        return Vec3<T>(r * std::cos(phi), r * std::sin(phi), z);
    }
    
    /// @brief PDF for uniform sphere sampling
    static T uniform_sphere_pdf() {
        return inv_4pi<T>();
    }
    
    /// @brief Sample uniform disk
    static void sample_uniform_disk(T u, T v, T& x, T& y) {
        T r = std::sqrt(u);
        T theta = T(2) * pi<T>() * v;
        x = r * std::cos(theta);
        y = r * std::sin(theta);
    }
};

} // namespace mcps
