#pragma once

#include "math_utils.hpp"
#include "ray.hpp"

namespace mcps {

// Forward declarations
template<typename T> class Material;
template<typename T> class Primitive;

/// @brief Intersection record with all necessary shading information
template<typename T>
struct Intersection {
    bool hit;                      ///< Whether an intersection occurred
    T t;                           ///< Distance along ray
    Vec3<T> position;              ///< Intersection position in world space
    Vec3<T> normal;                ///< Shading normal (interpolated for smooth surfaces)
    Vec3<T> geometric_normal;      ///< True geometric normal
    Vec3<T> tangent;               ///< Tangent vector for texture coordinates
    Vec3<T> bitangent;             ///< Bitangent vector for texture coordinates
    T u, v;                        ///< Texture coordinates
    const Material<T>* material;   ///< Material at intersection point
    const Primitive<T>* primitive; ///< Intersected primitive
    
    Intersection() : hit(false), t(0), position(), normal(), geometric_normal(),
                     tangent(), bitangent(), u(0), v(0), material(nullptr), primitive(nullptr) {}
    
    /// @brief Compute orthonormal basis from normal
    void compute_tangent_space() {
        coordinate_system(normal, tangent, bitangent);
    }
};

using Intersectionf = Intersection<float>;
using Intersectiond = Intersection<double>;

} // namespace mcps
