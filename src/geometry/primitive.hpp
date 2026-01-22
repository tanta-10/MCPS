#pragma once

#include "../core/ray.hpp"
#include "../core/intersection.hpp"
#include "../core/math_utils.hpp"

namespace mcps {

/// @brief Abstract primitive interface
template<typename T>
class Primitive {
public:
    virtual ~Primitive() = default;
    
    /// @brief Test ray intersection
    virtual bool intersect(const Ray<T>& ray, Intersection<T>& isect) const = 0;
    
    /// @brief Get bounding box
    virtual AABB<T> bounds() const = 0;
    
    /// @brief Sample point on surface
    virtual Vec3<T> sample_surface(T u, T v, Vec3<T>& normal, T& pdf) const = 0;
    
    /// @brief Get surface area
    virtual T surface_area() const = 0;
};

} // namespace mcps
