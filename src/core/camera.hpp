#pragma once

#include "math_utils.hpp"
#include "ray.hpp"
#include <cmath>

namespace mcps {

/// @brief Hyperbolic camera model for non-Euclidean rendering
/// This is a novel camera that renders scenes in hyperbolic space, creating
/// unique fish-eye and artistic distortions
template<typename T>
class HyperbolicCamera {
public:
    Vec3<T> position;
    Vec3<T> forward, up, right;
    T fov;
    T curvature; ///< Hyperbolic space curvature parameter (k < 0)
    
    HyperbolicCamera(const Vec3<T>& pos, const Vec3<T>& look_at, const Vec3<T>& up_vec,
                     T fov_degrees, T curvature = T(-1))
        : position(pos), fov(radians(fov_degrees)), curvature(curvature) {
        forward = (look_at - pos).normalized();
        right = cross(forward, up_vec).normalized();
        up = cross(right, forward);
    }
    
    Ray<T> generate_ray(T u, T v) const {
        // Map to [-1, 1] range
        T x = T(2) * u - T(1);
        T y = T(2) * v - T(1);
        
        // Apply hyperbolic transformation
        T r2 = x * x + y * y;
        T scale = std::tan(fov * T(0.5));
        
        // Hyperbolic distance
        T sinh_r = std::sinh(std::sqrt(r2) * scale / std::sqrt(-curvature));
        T hyperbolic_scale = sinh_r / std::sqrt(r2 + T(1e-8));
        
        Vec3<T> dir = (right * (x * hyperbolic_scale) +
                       up * (y * hyperbolic_scale) +
                       forward).normalized();
        
        return Ray<T>(position, dir);
    }
};

/// @brief Pinhole camera model (standard)
template<typename T>
class PinholeCamera {
public:
    Vec3<T> position;
    Vec3<T> forward, up, right;
    T fov;
    T aspect_ratio;
    
    PinholeCamera(const Vec3<T>& pos, const Vec3<T>& look_at, const Vec3<T>& up_vec,
                  T fov_degrees, T aspect = T(1))
        : position(pos), fov(radians(fov_degrees)), aspect_ratio(aspect) {
        forward = (look_at - pos).normalized();
        right = cross(forward, up_vec).normalized();
        up = cross(right, forward);
    }
    
    Ray<T> generate_ray(T u, T v) const {
        T tan_fov = std::tan(fov * T(0.5));
        T x = (T(2) * u - T(1)) * tan_fov * aspect_ratio;
        T y = (T(1) - T(2) * v) * tan_fov;
        
        Vec3<T> dir = (right * x + up * y + forward).normalized();
        return Ray<T>(position, dir);
    }
};

/// @brief Thin lens camera with depth of field
template<typename T>
class ThinLensCamera {
public:
    Vec3<T> position;
    Vec3<T> forward, up, right;
    T fov;
    T aspect_ratio;
    T aperture_radius;
    T focal_distance;
    
    ThinLensCamera(const Vec3<T>& pos, const Vec3<T>& look_at, const Vec3<T>& up_vec,
                   T fov_degrees, T aspect, T aperture, T focus_dist)
        : position(pos), fov(radians(fov_degrees)), aspect_ratio(aspect),
          aperture_radius(aperture), focal_distance(focus_dist) {
        forward = (look_at - pos).normalized();
        right = cross(forward, up_vec).normalized();
        up = cross(right, forward);
    }
    
    template<typename Sampler>
    Ray<T> generate_ray(T u, T v, Sampler& sampler) const {
        T tan_fov = std::tan(fov * T(0.5));
        T x = (T(2) * u - T(1)) * tan_fov * aspect_ratio;
        T y = (T(1) - T(2) * v) * tan_fov;
        
        // Point on focal plane
        Vec3<T> focal_point = position + forward * focal_distance +
                              right * (x * focal_distance) +
                              up * (y * focal_distance);
        
        // Sample point on lens
        T lens_u, lens_v;
        sampler.next_2d(lens_u, lens_v);
        T dx, dy;
        SamplingUtils<T>::sample_uniform_disk(lens_u, lens_v, dx, dy);
        
        Vec3<T> lens_pos = position + right * (dx * aperture_radius) +
                                      up * (dy * aperture_radius);
        
        Vec3<T> dir = (focal_point - lens_pos).normalized();
        return Ray<T>(lens_pos, dir);
    }
};

/// @brief Omnidirectional (360°) camera
template<typename T>
class OmnidirectionalCamera {
public:
    Vec3<T> position;
    Vec3<T> forward, up, right;
    
    OmnidirectionalCamera(const Vec3<T>& pos, const Vec3<T>& look_at, const Vec3<T>& up_vec)
        : position(pos) {
        forward = (look_at - pos).normalized();
        right = cross(forward, up_vec).normalized();
        up = cross(right, forward);
    }
    
    Ray<T> generate_ray(T u, T v) const {
        // Spherical coordinates
        T theta = u * T(2) * pi<T>();      // Azimuth: [0, 2π]
        T phi = v * pi<T>();                 // Elevation: [0, π]
        
        T sin_phi = std::sin(phi);
        T x = sin_phi * std::cos(theta);
        T y = sin_phi * std::sin(theta);
        T z = std::cos(phi);
        
        Vec3<T> dir = (right * x + up * y + forward * z).normalized();
        return Ray<T>(position, dir);
    }
};

using HyperbolicCameraf = HyperbolicCamera<float>;
using PinholeCameraf = PinholeCamera<float>;
using ThinLensCameraf = ThinLensCamera<float>;
using OmnidirectionalCameraf = OmnidirectionalCamera<float>;

} // namespace mcps
