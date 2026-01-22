#pragma once

#include "primitive.hpp"

namespace mcps {

/// @brief Triangle primitive
template<typename T>
class Triangle : public Primitive<T> {
public:
    Vec3<T> v0, v1, v2;           // Vertices
    Vec3<T> n0, n1, n2;           // Normals (for smooth shading)
    const Material<T>* material;
    
    Triangle(const Vec3<T>& v0, const Vec3<T>& v1, const Vec3<T>& v2,
             const Material<T>* mat = nullptr)
        : v0(v0), v1(v1), v2(v2), material(mat) {
        // Compute geometric normal
        Vec3<T> edge1 = v1 - v0;
        Vec3<T> edge2 = v2 - v0;
        Vec3<T> n = cross(edge1, edge2).normalized();
        n0 = n1 = n2 = n;
    }
    
    bool intersect(const Ray<T>& ray, Intersection<T>& isect) const override {
        // Möller–Trumbore algorithm
        const T EPSILON = T(1e-8);
        
        Vec3<T> edge1 = v1 - v0;
        Vec3<T> edge2 = v2 - v0;
        Vec3<T> h = cross(ray.direction, edge2);
        T a = dot(edge1, h);
        
        if (std::abs(a) < EPSILON) return false;
        
        T f = T(1) / a;
        Vec3<T> s = ray.origin - v0;
        T u = f * dot(s, h);
        
        if (u < 0 || u > 1) return false;
        
        Vec3<T> q = cross(s, edge1);
        T v = f * dot(ray.direction, q);
        
        if (v < 0 || u + v > 1) return false;
        
        T t = f * dot(edge2, q);
        
        if (t < ray.tmin || t > ray.tmax) return false;
        
        isect.hit = true;
        isect.t = t;
        isect.position = ray.at(t);
        
        // Interpolate normal (smooth shading)
        T w = T(1) - u - v;
        isect.normal = (n0 * w + n1 * u + n2 * v).normalized();
        isect.geometric_normal = cross(edge1, edge2).normalized();
        
        isect.u = u;
        isect.v = v;
        isect.material = material;
        isect.primitive = this;
        
        isect.compute_tangent_space();
        
        return true;
    }
    
    AABB<T> bounds() const override {
        AABB<T> box;
        box = box.union_with(v0);
        box = box.union_with(v1);
        box = box.union_with(v2);
        return box;
    }
    
    Vec3<T> sample_surface(T u, T v, Vec3<T>& normal, T& pdf) const override {
        // Uniform triangle sampling
        T sqrt_u = std::sqrt(u);
        T alpha = T(1) - sqrt_u;
        T beta = v * sqrt_u;
        T gamma = T(1) - alpha - beta;
        
        normal = (n0 * alpha + n1 * beta + n2 * gamma).normalized();
        pdf = T(1) / surface_area();
        
        return v0 * alpha + v1 * beta + v2 * gamma;
    }
    
    T surface_area() const override {
        Vec3<T> edge1 = v1 - v0;
        Vec3<T> edge2 = v2 - v0;
        return cross(edge1, edge2).length() * T(0.5);
    }
};

} // namespace mcps
