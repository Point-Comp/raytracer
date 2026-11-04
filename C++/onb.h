#ifndef ONB_H
#define ONB_H

#include "rtweekend.h"
#include <cmath>
#include <cmath>

class onb {
public:
    onb(const vec3& normal) {
        n = unit_vector(normal);

        vec3 a = (std::fabs(n.x()) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);

        b = unit_vector(cross(n, a));
        t = cross(n, b);
    }

    vec3 to_local(const vec3& v) const {
        return vec3(dot(v, t), dot(v, n), dot(v, b));
    }

    vec3 to_world(const vec3& v) const {
        return v.x() * t + v.y() * n + v.z() * b;
    }

    const vec3& tangent()   const { return t; }
    const vec3& normal()    const { return n; }
    const vec3& bitangent() const { return b; }

private:
    vec3 t, n, b;
};

#endif