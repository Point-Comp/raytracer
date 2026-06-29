#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

class triangle : public hittable {
  public:
    triangle(const point3& v0, const point3& v1, const point3& v2, shared_ptr<material> mat)
      : v0(v0), v1(v1), v2(v2), mat(mat) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;
        vec3 h = cross(r.direction(), edge2);
        auto det = dot(edge1, h);

        if (std::fabs(det) < 1e-8)
            return false;   // ray parallel to triangle

        auto inv_det = 1.0 / det;
        vec3 s = r.origin() - v0;
        auto u = inv_det * dot(s, h);
        if (u < 0 || u > 1)
            return false;

        vec3 q = cross(s, edge1);
        auto v = inv_det * dot(r.direction(), q);
        if (v < 0 || u + v > 1)
            return false;

        auto root = inv_det * dot(edge2, q);
        if (!ray_t.surrounds(root))
            return false;

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = unit_vector(cross(edge1, edge2));
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

  private:
    point3 v0, v1, v2;
    shared_ptr<material> mat;
};

#endif