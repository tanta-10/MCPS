#ifdef HAS_GTEST
#include <gtest/gtest.h>
#include "../src/core/ray.hpp"
#include "../src/core/math_utils.hpp"

using namespace mcps;

TEST(RayTest, Construction) {
    Vec3f origin(0, 0, 0);
    Vec3f direction(0, 0, -1);
    Rayf ray(origin, direction);
    
    EXPECT_EQ(ray.origin.x, 0.0f);
    EXPECT_EQ(ray.origin.y, 0.0f);
    EXPECT_EQ(ray.origin.z, 0.0f);
    
    EXPECT_EQ(ray.direction.x, 0.0f);
    EXPECT_EQ(ray.direction.y, 0.0f);
    EXPECT_EQ(ray.direction.z, -1.0f);
}

TEST(RayTest, At) {
    Vec3f origin(0, 0, 0);
    Vec3f direction(1, 0, 0);
    Rayf ray(origin, direction);
    
    Vec3f point = ray.at(5.0f);
    
    EXPECT_EQ(point.x, 5.0f);
    EXPECT_EQ(point.y, 0.0f);
    EXPECT_EQ(point.z, 0.0f);
}

TEST(Vec3Test, Operations) {
    Vec3f a(1, 2, 3);
    Vec3f b(4, 5, 6);
    
    Vec3f sum = a + b;
    EXPECT_EQ(sum.x, 5.0f);
    EXPECT_EQ(sum.y, 7.0f);
    EXPECT_EQ(sum.z, 9.0f);
    
    float d = dot(a, b);
    EXPECT_FLOAT_EQ(d, 32.0f);
    
    Vec3f c = cross(Vec3f(1, 0, 0), Vec3f(0, 1, 0));
    EXPECT_FLOAT_EQ(c.z, 1.0f);
}

#else
// Fallback main when GTest is not available
int main() {
    return 0;
}
#endif
