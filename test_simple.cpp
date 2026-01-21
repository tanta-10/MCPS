#include <iostream>
#include "src/integrators/integrator.hpp"
#include "src/materials/bsdf.hpp"
#include "src/materials/lambertian.hpp"
#include "src/geometry/sphere.hpp"

using namespace mcps;

int main() {
    std::cout << "Creating scene..." << std::endl;
    Scene<float>* scene = new Scene<float>();
    
    auto* mat = new Material<float>(
        new LambertianBSDF<float>(Spectrumf(0.7, 0.7, 0.7))
    );
    scene->add_material(mat);
    
    scene->add_primitive(new Sphere<float>(Vec3f(0, 0, 0), 1.0f, mat));
    
    std::cout << "Scene created successfully" << std::endl;
    std::cout << "Primitives: " << scene->primitives.size() << std::endl;
    std::cout << "Materials: " << scene->materials.size() << std::endl;
    
    std::cout << "Deleting scene..." << std::endl;
    delete scene;
    
    std::cout << "Done!" << std::endl;
    return 0;
}
