#ifdef HAS_GTEST
#include <gtest/gtest.h>
#include "../src/materials/bsdf.hpp"
#include "../src/materials/lambertian.hpp"

using namespace mcps;

TEST(MaterialTest, LambertianBSDF) {
    Spectrumf albedo(0.5f, 0.5f, 0.5f);
    LambertianBSDF<float> bsdf(albedo);
    
    Vec3f wo(0, 0, 1);
    Vec3f wi;
    float pdf;
    
    Spectrumf f = bsdf.sample(wo, wi, pdf, 0.5f, 0.5f);
    
    EXPECT_GT(pdf, 0.0f);
    EXPECT_GE(wi.z, 0.0f); // Should be in upper hemisphere
}

TEST(MaterialTest, ConductorBSDF) {
    Spectrumf reflectance(0.9f, 0.9f, 0.9f);
    ConductorBSDF<float> bsdf(reflectance, 0.0f);
    
    EXPECT_TRUE(bsdf.is_delta());
}

TEST(MaterialTest, DielectricBSDF) {
    DielectricBSDF<float> bsdf(1.5f);
    
    EXPECT_TRUE(bsdf.is_delta());
}

#else
int main() {
    return 0;
}
#endif
