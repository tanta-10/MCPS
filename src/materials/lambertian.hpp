#pragma once

#include "bsdf.hpp"

namespace mcps {

/// @brief Lambertian (diffuse) BSDF
template<typename T>
class LambertianBSDF : public BSDF<T> {
public:
    Spectrum<T> albedo;
    
    LambertianBSDF(const Spectrum<T>& albedo) : albedo(albedo) {}
    
    Spectrum<T> sample(const Vec3<T>& wo, Vec3<T>& wi, T& pdf, T u, T v) const override {
        // Cosine-weighted hemisphere sampling
        wi = SamplingUtils<T>::sample_cosine_hemisphere(u, v);
        pdf = SamplingUtils<T>::cosine_hemisphere_pdf(wi.z);
        
        return albedo * inv_pi<T>();
    }
    
    Spectrum<T> evaluate(const Vec3<T>& wo, const Vec3<T>& wi) const override {
        if (wi.z <= 0) return Spectrum<T>(0);
        return albedo * inv_pi<T>();
    }
    
    T pdf(const Vec3<T>& wo, const Vec3<T>& wi) const override {
        if (wi.z <= 0) return T(0);
        return SamplingUtils<T>::cosine_hemisphere_pdf(wi.z);
    }
};

/// @brief Conductor (metal) BSDF with Fresnel
template<typename T>
class ConductorBSDF : public BSDF<T> {
public:
    Spectrum<T> reflectance;
    T roughness;
    
    ConductorBSDF(const Spectrum<T>& reflectance, T roughness = T(0))
        : reflectance(reflectance), roughness(roughness) {}
    
    Spectrum<T> sample(const Vec3<T>& wo, Vec3<T>& wi, T& pdf, T u, T v) const override {
        if (roughness < T(0.01)) {
            // Perfect mirror
            wi = Vec3<T>(-wo.x, -wo.y, wo.z);
            pdf = T(1);
            
            T cos_theta = wi.z;
            return reflectance / std::abs(cos_theta);
        } else {
            // Rough conductor (simplified)
            wi = SamplingUtils<T>::sample_cosine_hemisphere(u, v);
            pdf = SamplingUtils<T>::cosine_hemisphere_pdf(wi.z);
            
            return reflectance * inv_pi<T>();
        }
    }
    
    Spectrum<T> evaluate(const Vec3<T>& wo, const Vec3<T>& wi) const override {
        if (roughness < T(0.01)) return Spectrum<T>(0); // Delta BSDF
        if (wi.z <= 0) return Spectrum<T>(0);
        
        return reflectance * inv_pi<T>();
    }
    
    T pdf(const Vec3<T>& wo, const Vec3<T>& wi) const override {
        if (roughness < T(0.01)) return T(0); // Delta BSDF
        if (wi.z <= 0) return T(0);
        
        return SamplingUtils<T>::cosine_hemisphere_pdf(wi.z);
    }
    
    bool is_delta() const override {
        return roughness < T(0.01);
    }
};

/// @brief Dielectric (glass) BSDF with refraction
template<typename T>
class DielectricBSDF : public BSDF<T> {
public:
    T ior; // Index of refraction
    
    DielectricBSDF(T ior) : ior(ior) {}
    
    Spectrum<T> sample(const Vec3<T>& wo, Vec3<T>& wi, T& pdf, T u, T v) const override {
        T cos_theta_i = wo.z;
        T eta = cos_theta_i > 0 ? (T(1) / ior) : ior;
        
        // Fresnel reflectance (Schlick's approximation)
        T r0 = (T(1) - eta) / (T(1) + eta);
        r0 = r0 * r0;
        T fresnel = r0 + (T(1) - r0) * std::pow(T(1) - std::abs(cos_theta_i), T(5));
        
        if (u < fresnel) {
            // Reflection
            wi = Vec3<T>(-wo.x, -wo.y, wo.z);
        } else {
            // Refraction
            Vec3<T> n = Vec3<T>(0, 0, cos_theta_i > 0 ? T(1) : T(-1));
            wi = refract(wo, n, eta);
            
            // Total internal reflection check
            if (wi.length_squared() < T(0.01)) {
                wi = Vec3<T>(-wo.x, -wo.y, wo.z);
            }
        }
        
        pdf = T(1);
        return Spectrum<T>(1) / std::abs(wi.z);
    }
    
    Spectrum<T> evaluate(const Vec3<T>& wo, const Vec3<T>& wi) const override {
        return Spectrum<T>(0); // Delta BSDF
    }
    
    T pdf(const Vec3<T>& wo, const Vec3<T>& wi) const override {
        return T(0); // Delta BSDF
    }
    
    bool is_delta() const override {
        return true;
    }
};

} // namespace mcps
