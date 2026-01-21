#pragma once

#include "../core/math_utils.hpp"
#include "../core/spectrum.hpp"
#include "../core/intersection.hpp"

namespace mcps {

/// @brief BSDF interface for bidirectional scattering distribution function
template<typename T>
class BSDF {
public:
    virtual ~BSDF() = default;
    
    /// @brief Sample outgoing direction given incoming direction
    /// @param wo Outgoing direction (toward camera)
    /// @param wi Sampled incident direction (output)
    /// @param pdf Probability density of sampled direction (output)
    /// @param u, v Random samples
    /// @return BSDF value
    virtual Spectrum<T> sample(const Vec3<T>& wo, Vec3<T>& wi, T& pdf, T u, T v) const = 0;
    
    /// @brief Evaluate BSDF for given directions
    virtual Spectrum<T> evaluate(const Vec3<T>& wo, const Vec3<T>& wi) const = 0;
    
    /// @brief Get PDF for sampling wi given wo
    virtual T pdf(const Vec3<T>& wo, const Vec3<T>& wi) const = 0;
    
    /// @brief Check if BSDF is delta distribution (mirror/glass)
    virtual bool is_delta() const { return false; }
};

/// @brief Material combining BSDF with optional emission
template<typename T>
class Material {
public:
    BSDF<T>* bsdf;
    Spectrum<T> emission;
    
    Material(BSDF<T>* bsdf = nullptr, const Spectrum<T>& emission = Spectrum<T>(0))
        : bsdf(bsdf), emission(emission) {}
    
    virtual ~Material() {
        // Note: BSDF ownership managed by material
        if (bsdf) delete bsdf;
    }
    
    // Prevent copying to avoid double-delete
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    
    bool is_emissive() const { return !emission.is_black(); }
};

} // namespace mcps
