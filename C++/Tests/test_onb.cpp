#include "onb.h"
#include <iostream>
#include <cmath>
#include <vector>

static int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::cout << "  FAIL: " << what << "\n";
        failures++;
    }
}

bool close(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}

bool close(const vec3& a, const vec3& b, double eps = 1e-9) {
    return (a - b).length() < eps;
}

int main() {
    std::vector<vec3> normals = {
        vec3(0, 1, 0),              
        vec3(1, 0, 0),
        vec3(0, 0, -1),
        vec3(0.3, 0.8, -0.5),       // arbitrary
        vec3(-2.0, 0.1, 0.4)        // not unit length on purpose
    };

    std::vector<vec3> test_vectors = {
        vec3(1, 0, 0),
        vec3(0.2, -0.7, 0.4),
        vec3(-1.5, 2.0, 0.3)
    };

    for (const vec3& normal : normals) {
        std::cout << "normal (" << normal.x() << ", " << normal.y()
                  << ", " << normal.z() << ")\n";

        onb frame(normal);

        check(close(frame.tangent().length(),   1.0), "tangent not unit length");
        check(close(frame.normal().length(),    1.0), "normal not unit length");
        check(close(frame.bitangent().length(), 1.0), "bitangent not unit length");

        check(close(dot(frame.tangent(), frame.normal()),    0.0), "t . n != 0");
        check(close(dot(frame.tangent(), frame.bitangent()), 0.0), "t . b != 0");
        check(close(dot(frame.normal(), frame.bitangent()),  0.0), "n . b != 0");

        check(close(frame.to_local(frame.normal()), vec3(0, 1, 0)),
              "to_local(normal) != (0,1,0)");

        for (const vec3& v : test_vectors) {
            check(close(frame.to_world(frame.to_local(v)), v),
                  "to_world(to_local(v)) != v");
            check(close(frame.to_local(frame.to_world(v)), v),
                  "to_local(to_world(v)) != v");
        }

        for (const vec3& v : test_vectors) {
            check(close(frame.to_local(v).length(), v.length()),
                  "to_local changed the vector's length");
        }
    }

    if (failures == 0)
        std::cout << "\nAll checks passed.\n";
    else
        std::cout << "\n" << failures << " check(s) failed.\n";

    return failures;
}