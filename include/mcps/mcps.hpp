#pragma once

/**
 * @file mcps.hpp
 * @brief MCPS (Monte Carlo Path Sampling) - Novel Experimental Path Tracer
 * 
 * This is the main public API header for the MCPS renderer.
 * Include this file to access all MCPS functionality.
 * 
 * Novel Features:
 * - Quantum-inspired importance sampling
 * - Chaos-enhanced variance reduction (Lorenz attractor)
 * - Genetic algorithm path optimization
 * - Fractal-based adaptive sampling
 * - Entropy-driven sample allocation
 * - Neural radiance caching with online learning
 * - Hyperbolic geometry camera models
 * - Multi-resolution hash encoding
 * 
 * @author MCPS Project
 * @version 1.0.0
 */

#define MCPS_VERSION_MAJOR 1
#define MCPS_VERSION_MINOR 0
#define MCPS_VERSION_PATCH 0

// Core components
#include "../src/core/math_utils.hpp"
#include "../src/core/ray.hpp"
#include "../src/core/spectrum.hpp"
#include "../src/core/intersection.hpp"
#include "../src/core/sampler.hpp"
#include "../src/core/camera.hpp"

// Geometry
#include "../src/geometry/primitive.hpp"
#include "../src/geometry/sphere.hpp"
#include "../src/geometry/triangle.hpp"

// Materials
#include "../src/materials/bsdf.hpp"
#include "../src/materials/lambertian.hpp"

// Integrators
#include "../src/integrators/integrator.hpp"
#include "../src/integrators/mcps_integrator.hpp"

// Experimental features
#include "../src/sampling/experimental_samplers.hpp"
#include "../src/sampling/genetic_path_optimizer.hpp"
#include "../src/neural/neural_components.hpp"

/**
 * @namespace mcps
 * @brief Main namespace for MCPS renderer
 */
namespace mcps {
    // Type aliases for convenience
    using Float = float;
    using Vec3 = Vec3f;
    using Ray = Rayf;
    using Spectrum = Spectrumf;
    using AABB = AABBf;
    using Camera = PinholeCameraf;
}

/**
 * @namespace mcps::experimental
 * @brief Experimental and novel rendering techniques
 */
namespace mcps::experimental {
    // Experimental features are in this namespace
}
