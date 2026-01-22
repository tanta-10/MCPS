#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

namespace mcps {

/// @brief 3D vector class for mathematical operations
template<typename T>
struct Vec3 {
    T x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    explicit Vec3(T v) : x(v), y(v), z(v) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(T s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator/(T s) const { T inv = T(1) / s; return Vec3(x * inv, y * inv, z * inv); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(T s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(T s) { T inv = T(1) / s; x *= inv; y *= inv; z *= inv; return *this; }
    
    T operator[](int i) const { return (&x)[i]; }
    T& operator[](int i) { return (&x)[i]; }
    
    T length() const { return std::sqrt(x * x + y * y + z * z); }
    T length_squared() const { return x * x + y * y + z * z; }
    
    Vec3 normalized() const {
        T len = length();
        return len > T(0) ? (*this) / len : Vec3(0);
    }
};

template<typename T>
Vec3<T> operator*(T s, const Vec3<T>& v) { return v * s; }

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

/// @brief Dot product
template<typename T>
inline T dot(const Vec3<T>& a, const Vec3<T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// @brief Cross product
template<typename T>
inline Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b) {
    return Vec3<T>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

/// @brief Reflect vector around normal
template<typename T>
inline Vec3<T> reflect(const Vec3<T>& v, const Vec3<T>& n) {
    return v - n * (T(2) * dot(v, n));
}

/// @brief Refract vector through interface
template<typename T>
inline Vec3<T> refract(const Vec3<T>& v, const Vec3<T>& n, T eta) {
    T cos_theta = std::min(dot(-v, n), T(1));
    Vec3<T> r_perp = (v + n * cos_theta) * eta;
    Vec3<T> r_parallel = n * (-std::sqrt(std::abs(T(1) - r_perp.length_squared())));
    return r_perp + r_parallel;
}

/// @brief Build orthonormal basis from a normal vector
template<typename T>
inline void coordinate_system(const Vec3<T>& v1, Vec3<T>& v2, Vec3<T>& v3) {
    if (std::abs(v1.x) > std::abs(v1.y)) {
        v2 = Vec3<T>(-v1.z, T(0), v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
    } else {
        v2 = Vec3<T>(T(0), v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
    }
    v3 = cross(v1, v2);
}

/// @brief Transform vector from local to world space
template<typename T>
inline Vec3<T> local_to_world(const Vec3<T>& local, const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z) {
    return x * local.x + y * local.y + z * local.z;
}

/// @brief Transform vector from world to local space
template<typename T>
inline Vec3<T> world_to_local(const Vec3<T>& world, const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z) {
    return Vec3<T>(dot(world, x), dot(world, y), dot(world, z));
}

/// @brief Compute cosine of angle between vectors
template<typename T>
inline T cos_theta(const Vec3<T>& w) {
    return w.z;
}

/// @brief Compute sine squared of angle
template<typename T>
inline T sin_theta_squared(const Vec3<T>& w) {
    return std::max(T(0), T(1) - cos_theta(w) * cos_theta(w));
}

/// @brief Compute sine of angle
template<typename T>
inline T sin_theta(const Vec3<T>& w) {
    return std::sqrt(sin_theta_squared(w));
}

/// @brief Axis-aligned bounding box
template<typename T>
struct AABB {
    Vec3<T> min, max;
    
    AABB() : min(Vec3<T>(std::numeric_limits<T>::max())),
             max(Vec3<T>(std::numeric_limits<T>::lowest())) {}
    
    AABB(const Vec3<T>& p) : min(p), max(p) {}
    AABB(const Vec3<T>& min, const Vec3<T>& max) : min(min), max(max) {}
    
    Vec3<T> center() const { return (min + max) * T(0.5); }
    Vec3<T> diagonal() const { return max - min; }
    
    T surface_area() const {
        Vec3<T> d = diagonal();
        return T(2) * (d.x * d.y + d.y * d.z + d.z * d.x);
    }
    
    AABB union_with(const AABB& other) const {
        return AABB(
            Vec3<T>(std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z)),
            Vec3<T>(std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z))
        );
    }
    
    AABB union_with(const Vec3<T>& p) const {
        return AABB(
            Vec3<T>(std::min(min.x, p.x), std::min(min.y, p.y), std::min(min.z, p.z)),
            Vec3<T>(std::max(max.x, p.x), std::max(max.y, p.y), std::max(max.z, p.z))
        );
    }
};

using AABBf = AABB<float>;
using AABBd = AABB<double>;

/// @brief Constants
template<typename T>
constexpr T pi() { return T(3.14159265358979323846); }

template<typename T>
constexpr T inv_pi() { return T(0.31830988618379067154); }

template<typename T>
constexpr T inv_2pi() { return T(0.15915494309189533577); }

template<typename T>
constexpr T inv_4pi() { return T(0.07957747154594766788); }

/// @brief Convert degrees to radians
template<typename T>
inline T radians(T degrees) {
    return degrees * pi<T>() / T(180);
}

/// @brief Clamp value between min and max
template<typename T>
inline T clamp(T x, T min_val, T max_val) {
    return std::max(min_val, std::min(x, max_val));
}

} // namespace mcps
