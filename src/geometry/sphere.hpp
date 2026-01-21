#pragma once

#include "primitive.hpp"

namespace mcps {

/// @brief Sphere primitive
template<typename T>
class Sphere : public Primitive<T> {
public:
    Vec3<T> center;
    T radius;
    const Material<T>* material;
    
    Sphere(const Vec3<T>& center, T radius, const Material<T>* mat = nullptr)
        : center(center), radius(radius), material(mat) {}
    
    bool intersect(const Ray<T>& ray, Intersection<T>& isect) const override {
        Vec3<T> oc = ray.origin - center;
        T a = ray.direction.length_squared();
        T half_b = dot(oc, ray.direction);
        T c = oc.length_squared() - radius * radius;
        T discriminant = half_b * half_b - a * c;
        
        if (discriminant < 0) return false;
        
        T sqrt_d = std::sqrt(discriminant);
        T root = (-half_b - sqrt_d) / a;
        
        if (root < ray.tmin || root > ray.tmax) {
            root = (-half_b + sqrt_d) / a;
            if (root < ray.tmin || root > ray.tmax) {
                return false;
            }
        }
        
        isect.hit = true;
        isect.t = root;
        isect.position = ray.at(root);
        isect.normal = (isect.position - center) / radius;
        isect.geometric_normal = isect.normal;
        isect.material = material;
        isect.primitive = this;
        
        // Compute UV coordinates
        T theta = std::acos(-isect.normal.y);
        T phi = std::atan2(-isect.normal.z, isect.normal.x) + pi<T>();
        isect.u = phi / (T(2) * pi<T>());
        isect.v = theta / pi<T>();
        
        isect.compute_tangent_space();
        
        return true;
    }
    
    AABB<T> bounds() const override {
        Vec3<T> r(radius);
        return AABB<T>(center - r, center + r);
    }
    
    Vec3<T> sample_surface(T u, T v, Vec3<T>& normal, T& pdf) const override {
        // Uniform sphere sampling
        normal = SamplingUtils<T>::sample_uniform_sphere(u, v);
        pdf = T(1) / surface_area();
        return center + normal * radius;
    }
    
    T surface_area() const override {
        return T(4) * pi<T>() * radius * radius;
    }
};

} // namespace mcps
