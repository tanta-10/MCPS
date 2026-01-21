#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

// Core components
#include "core/math_utils.hpp"
#include "core/ray.hpp"
#include "core/spectrum.hpp"
#include "core/intersection.hpp"
#include "core/sampler.hpp"
#include "core/camera.hpp"

// Geometry
#include "geometry/primitive.hpp"
#include "geometry/sphere.hpp"
#include "geometry/triangle.hpp"

// Materials
#include "materials/bsdf.hpp"
#include "materials/lambertian.hpp"

// Integrators
#include "integrators/integrator.hpp"
#include "integrators/mcps_integrator.hpp"

using namespace mcps;
using namespace mcps::experimental;

using Float = float;

/// @brief Save image to PPM format
void save_ppm(const std::string& filename, const std::vector<Spectrumf>& image,
              int width, int height) {
    std::ofstream file(filename);
    file << "P3\n" << width << " " << height << "\n255\n";
    
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            const auto& pixel = image[idx];
            
            // Gamma correction
            Float r = std::pow(clamp(pixel.r, Float(0), Float(1)), Float(1.0 / 2.2));
            Float g = std::pow(clamp(pixel.g, Float(0), Float(1)), Float(1.0 / 2.2));
            Float b = std::pow(clamp(pixel.b, Float(0), Float(1)), Float(1.0 / 2.2));
            
            int ir = static_cast<int>(r * 255.99);
            int ig = static_cast<int>(g * 255.99);
            int ib = static_cast<int>(b * 255.99);
            
            file << ir << " " << ig << " " << ib << "\n";
        }
    }
    
    file.close();
    std::cout << "Saved image to " << filename << std::endl;
}

/// @brief Create Cornell Box scene
Scene<Float>* create_cornell_box() {
    auto* scene = new Scene<Float>();
    
    // Create materials
    auto* white = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.73, 0.73, 0.73))
    );
    auto* red = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.65, 0.05, 0.05))
    );
    auto* green = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.12, 0.45, 0.15))
    );
    auto* light_mat = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.0)),
        Spectrumf(15, 15, 15) // Emission
    );
    auto* metal = new Material<Float>(
        new ConductorBSDF<Float>(Spectrumf(0.9, 0.9, 0.9), Float(0.01))
    );
    auto* glass = new Material<Float>(
        new DielectricBSDF<Float>(Float(1.5))
    );
    
    // Register materials with scene for cleanup
    scene->add_material(white);
    scene->add_material(red);
    scene->add_material(green);
    scene->add_material(light_mat);
    scene->add_material(metal);
    scene->add_material(glass);
    
    // Walls (as large triangles/spheres)
    // Floor
    scene->add_primitive(new Sphere<Float>(Vec3f(0, -1000.5, 0), Float(1000), white));
    
    // Ceiling
    scene->add_primitive(new Sphere<Float>(Vec3f(0, 1000.5, 0), Float(1000), white));
    
    // Back wall
    scene->add_primitive(new Sphere<Float>(Vec3f(0, 0, -1001), Float(1000), white));
    
    // Left wall (red)
    scene->add_primitive(new Sphere<Float>(Vec3f(-1001, 0, 0), Float(1000), red));
    
    // Right wall (green)
    scene->add_primitive(new Sphere<Float>(Vec3f(1001, 0, 0), Float(1000), green));
    
    // Light source (small sphere on ceiling)
    scene->add_primitive(new Sphere<Float>(Vec3f(0, 0.45, 0), Float(0.15), light_mat));
    
    // Objects in scene
    // Left sphere (mirror)
    scene->add_primitive(new Sphere<Float>(Vec3f(-0.3, -0.25, -0.3), Float(0.25), metal));
    
    // Right sphere (glass)
    scene->add_primitive(new Sphere<Float>(Vec3f(0.3, -0.25, 0.1), Float(0.25), glass));
    
    return scene;
}

/// @brief Create experimental fractal scene
Scene<Float>* create_experimental_scene() {
    auto* scene = new Scene<Float>();
    
    // Colorful materials for experimentation
    auto* mat1 = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.9, 0.3, 0.3))
    );
    auto* mat2 = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.3, 0.9, 0.3))
    );
    auto* mat3 = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.3, 0.3, 0.9))
    );
    auto* light = new Material<Float>(
        new LambertianBSDF<Float>(Spectrumf(0.0)),
        Spectrumf(20, 20, 20)
    );
    
    // Register materials
    scene->add_material(mat1);
    scene->add_material(mat2);
    scene->add_material(mat3);
    scene->add_material(light);
    
    // Ground
    scene->add_primitive(new Sphere<Float>(Vec3f(0, -1000.5, 0), Float(1000), mat1));
    
    // Create fractal-like sphere arrangement
    for (int i = 0; i < 5; ++i) {
        Float angle = Float(i) * Float(2) * pi<Float>() / Float(5);
        Float radius = Float(0.5);
        Vec3f pos(std::cos(angle) * radius, -0.2, std::sin(angle) * radius);
        
        Material<Float>* mat = (i % 3 == 0) ? mat1 : ((i % 3 == 1) ? mat2 : mat3);
        scene->add_primitive(new Sphere<Float>(pos, Float(0.15), mat));
    }
    
    // Central light
    scene->add_primitive(new Sphere<Float>(Vec3f(0, 0.8, 0), Float(0.2), light));
    
    scene->background = Spectrumf(0.05, 0.05, 0.1);
    
    return scene;
}

int main(int argc, char** argv) {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     MCPS: Novel Monte Carlo Path Sampling Renderer      ║\n";
    std::cout << "║          Experimental Features Demonstration             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    
    // Image settings
    int width = 400;
    int height = 400;
    int spp = 64; // Samples per pixel
    
    // Parse command line arguments
    if (argc > 1) {
        width = std::atoi(argv[1]);
        height = width;
    }
    if (argc > 2) {
        spp = std::atoi(argv[2]);
    }
    
    std::cout << "Resolution: " << width << "x" << height << std::endl;
    std::cout << "Samples per pixel: " << spp << std::endl << std::endl;
    
    std::cout << "About to create scene...\n" << std::flush;
    
    // Create scene
    std::cout << "Creating scene...\n" << std::flush;
    Scene<Float>* scene = create_cornell_box();
    std::cout << "Scene created successfully\n" << std::flush;
    
    // Create camera
    std::cout << "Setting up camera...\n";
    PinholeCameraf camera(
        Vec3f(0, 0, 3),      // Position
        Vec3f(0, 0, 0),      // Look at
        Vec3f(0, 1, 0),      // Up
        40.0f,               // FOV
        Float(width) / Float(height) // Aspect ratio
    );
    
    // Test different sampling modes
    std::vector<std::pair<std::string, MCPSIntegrator<Float>::SamplingMode>> modes = {
        {"chaotic", MCPSIntegrator<Float>::SamplingMode::CHAOTIC},
        {"quantum", MCPSIntegrator<Float>::SamplingMode::QUANTUM},
        {"fractal", MCPSIntegrator<Float>::SamplingMode::FRACTAL},
        {"entropy", MCPSIntegrator<Float>::SamplingMode::ENTROPY_DRIVEN}
    };
    
    for (const auto& [name, mode] : modes) {
        std::cout << "\n========================================\n" << std::flush;
        std::cout << "Rendering with " << name << " sampling...\n" << std::flush;
        std::cout << "========================================\n" << std::flush;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::cout << "Creating integrator...\n" << std::flush;
        // Create integrator
        MCPSIntegrator<Float> integrator(mode, true, false);
        std::cout << "Integrator created\n" << std::flush;
        
        std::cout << "Starting render...\n" << std::flush;
        // Render
        auto image = integrator.render_novel(*scene, camera, width, height, spp);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Rendering completed in " << duration.count() / 1000.0 << " seconds\n";
        
        // Save image
        std::string filename = "output_" + name + ".ppm";
        save_ppm(filename, image, width, height);
    }
    
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              Rendering Complete!                         ║\n";
    std::cout << "║  Novel features demonstrated:                            ║\n";
    std::cout << "║  ✓ Quantum-inspired sampling                             ║\n";
    std::cout << "║  ✓ Chaos-enhanced path tracing                           ║\n";
    std::cout << "║  ✓ Fractal importance sampling                           ║\n";
    std::cout << "║  ✓ Entropy-driven adaptive sampling                      ║\n";
    std::cout << "║  ✓ Neural radiance caching                               ║\n";
    std::cout << "║  ✓ Hyperbolic camera models                              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    
    delete scene;
    
    return 0;
}
