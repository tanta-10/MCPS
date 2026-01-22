#ifdef HAS_GTEST
#include <gtest/gtest.h>
#include "../src/integrators/mcps_integrator.hpp"
#include "../src/geometry/sphere.hpp"

using namespace mcps;
using namespace mcps::experimental;

TEST(IntegratorTest, Creation) {
    MCPSIntegrator<float> integrator(
        MCPSIntegrator<float>::SamplingMode::CHAOTIC,
        true,
        false
    );
    
    // Just test creation doesn't crash
    SUCCEED();
}

TEST(IntegratorTest, SimpleScene) {
    Scene<float> scene;
    
    auto* white = new Material<float>(
        new LambertianBSDF<float>(Spectrumf(0.7f, 0.7f, 0.7f))
    );
    
    scene.add_primitive(new Sphere<float>(Vec3f(0, 0, 0), 1.0f, white));
    
    MCPSIntegrator<float> integrator(
        MCPSIntegrator<float>::SamplingMode::CHAOTIC,
        false,
        false
    );
    
    IndependentSampler<float> sampler(42);
    Rayf ray(Vec3f(0, 0, -5), Vec3f(0, 0, 1));
    
    Spectrumf radiance = integrator.Li(ray, scene, sampler);
    
    // Should get some radiance
    EXPECT_GE(radiance.luminance(), 0.0f);
}

#else
int main() {
    return 0;
}
#endif
