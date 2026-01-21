#ifdef HAS_GTEST
#include <gtest/gtest.h>
#include "../src/geometry/sphere.hpp"
#include "../src/core/ray.hpp"

using namespace mcps;

TEST(SphereTest, Intersection) {
    Sphere<float> sphere(Vec3f(0, 0, 0), 1.0f);
    Rayf ray(Vec3f(0, 0, -5), Vec3f(0, 0, 1));
    
    Intersectionf isect;
    bool hit = sphere.intersect(ray, isect);
    
    EXPECT_TRUE(hit);
    EXPECT_NEAR(isect.t, 4.0f, 1e-5f);
}

TEST(SphereTest, Miss) {
    Sphere<float> sphere(Vec3f(0, 0, 0), 1.0f);
    Rayf ray(Vec3f(5, 0, -5), Vec3f(0, 0, 1));
    
    Intersectionf isect;
    bool hit = sphere.intersect(ray, isect);
    
    EXPECT_FALSE(hit);
}

TEST(SphereTest, Bounds) {
    Sphere<float> sphere(Vec3f(1, 2, 3), 0.5f);
    AABBf bounds = sphere.bounds();
    
    EXPECT_FLOAT_EQ(bounds.min.x, 0.5f);
    EXPECT_FLOAT_EQ(bounds.max.x, 1.5f);
}

#else
int main() {
    return 0;
}
#endif
