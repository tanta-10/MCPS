#pragma once

#include "../core/ray.hpp"
#include "../core/spectrum.hpp"
#include "../core/intersection.hpp"
#include "../core/sampler.hpp"
#include <vector>

namespace mcps {

/// @brief Scene container
template<typename T>
class Scene {
public:
    std::vector<Primitive<T>*> primitives;
    std::vector<Material<T>*> materials; // Own materials
    Spectrum<T> background;
    
    Scene() : background(T(0.1), T(0.1), T(0.2)) {}
    
    ~Scene() {
        for (auto* p : primitives) delete p;
        for (auto* m : materials) delete m;
    }
    
    bool intersect(const Ray<T>& ray, Intersection<T>& isect) const {
        bool hit_anything = false;
        T closest = ray.tmax;
        
        for (const auto* prim : primitives) {
            Intersection<T> temp_isect;
            if (prim->intersect(ray, temp_isect) && temp_isect.t < closest) {
                closest = temp_isect.t;
                isect = temp_isect;
                hit_anything = true;
            }
        }
        
        return hit_anything;
    }
    
    void add_primitive(Primitive<T>* prim) {
        primitives.push_back(prim);
    }
    
    void add_material(Material<T>* mat) {
        materials.push_back(mat);
    }
};

/// @brief Abstract integrator interface
template<typename T>
class Integrator {
public:
    virtual ~Integrator() = default;
    
    /// @brief Compute radiance along ray
    virtual Spectrum<T> Li(const Ray<T>& ray, const Scene<T>& scene, Sampler<T>& sampler, int depth = 0) = 0;
    
    /// @brief Render full image (placeholder - use specific camera type with render_novel)
    virtual void render(const Scene<T>& scene, int width, int height, int spp = 64) = 0;
};

} // namespace mcps
