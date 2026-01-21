#pragma once

#include "math_utils.hpp"
#include <limits>

namespace mcps {

/// @brief Ray representation with origin, direction, and parametric bounds
template<typename T>
class Ray {
public:
    Vec3<T> origin;
    Vec3<T> direction;
    T tmin, tmax;
    
    Ray() : origin(), direction(0, 0, 1), tmin(0), tmax(std::numeric_limits<T>::max()) {}
    
    Ray(const Vec3<T>& origin, const Vec3<T>& direction,
        T tmin = T(1e-4), T tmax = std::numeric_limits<T>::max())
        : origin(origin), direction(direction), tmin(tmin), tmax(tmax) {}
    
    /// @brief Evaluate ray at parameter t
    Vec3<T> at(T t) const {
        return origin + direction * t;
    }
};

using Rayf = Ray<float>;
using Rayd = Ray<double>;

} // namespace mcps
