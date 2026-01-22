#pragma once

#include "math_utils.hpp"
#include <algorithm>

namespace mcps {

/// @brief Spectral/RGB color representation
template<typename T>
class Spectrum {
public:
    T r, g, b;
    
    Spectrum() : r(0), g(0), b(0) {}
    Spectrum(T v) : r(v), g(v), b(v) {}
    Spectrum(T r, T g, T b) : r(r), g(g), b(b) {}
    
    Spectrum operator+(const Spectrum& s) const { return Spectrum(r + s.r, g + s.g, b + s.b); }
    Spectrum operator-(const Spectrum& s) const { return Spectrum(r - s.r, g - s.g, b - s.b); }
    Spectrum operator*(const Spectrum& s) const { return Spectrum(r * s.r, g * s.g, b * s.b); }
    Spectrum operator*(T v) const { return Spectrum(r * v, g * v, b * v); }
    Spectrum operator/(T v) const { T inv = T(1) / v; return Spectrum(r * inv, g * inv, b * inv); }
    
    Spectrum& operator+=(const Spectrum& s) { r += s.r; g += s.g; b += s.b; return *this; }
    Spectrum& operator-=(const Spectrum& s) { r -= s.r; g -= s.g; b -= s.b; return *this; }
    Spectrum& operator*=(const Spectrum& s) { r *= s.r; g *= s.g; b *= s.b; return *this; }
    Spectrum& operator*=(T v) { r *= v; g *= v; b *= v; return *this; }
    Spectrum& operator/=(T v) { T inv = T(1) / v; r *= inv; g *= inv; b *= inv; return *this; }
    
    T operator[](int i) const { return (&r)[i]; }
    T& operator[](int i) { return (&r)[i]; }
    
    /// @brief Check if spectrum is black
    bool is_black() const { return r == T(0) && g == T(0) && b == T(0); }
    
    /// @brief Get luminance (Y in XYZ color space)
    T luminance() const {
        return T(0.212671) * r + T(0.715160) * g + T(0.072169) * b;
    }
    
    /// @brief Clamp spectrum values
    Spectrum clamp(T min_val = T(0), T max_val = T(1)) const {
        return Spectrum(
            std::max(min_val, std::min(r, max_val)),
            std::max(min_val, std::min(g, max_val)),
            std::max(min_val, std::min(b, max_val))
        );
    }
};

template<typename T>
Spectrum<T> operator*(T v, const Spectrum<T>& s) { return s * v; }

using Spectrumf = Spectrum<float>;
using Spectrumd = Spectrum<double>;

} // namespace mcps
