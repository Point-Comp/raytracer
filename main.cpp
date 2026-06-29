#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"

#include <iostream>

int main() {
    hittable_list world;

    // ground so we have a floor for reference
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    // one test triangle
    auto m = make_shared<lambertian>(color(0.8, 0.3, 0.3));

    // 8 corners
    point3 v0(0,0,0), v1(1,0,0), v2(1,0,1), v3(0,0,1);
    point3 v4(0,1,0), v5(1,1,0), v6(1,1,1), v7(0,1,1);

    // front (z=1)
    world.add(make_shared<triangle>(v3, v2, v6, m));
    world.add(make_shared<triangle>(v3, v6, v7, m));
    // top (y=1)
    world.add(make_shared<triangle>(v7, v6, v5, m));
    world.add(make_shared<triangle>(v7, v5, v4, m));
    // back (z=0)
    world.add(make_shared<triangle>(v1, v0, v4, m));
    world.add(make_shared<triangle>(v1, v4, v5, m));
    // bottom (y=0)
    world.add(make_shared<triangle>(v0, v1, v2, m));
    world.add(make_shared<triangle>(v0, v2, v3, m));
    // left (x=0)
    world.add(make_shared<triangle>(v0, v3, v7, m));
    world.add(make_shared<triangle>(v0, v7, v4, m));
    // right (x=1)
    world.add(make_shared<triangle>(v2, v1, v5, m));
    world.add(make_shared<triangle>(v2, v5, v6, m));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;   // small + fast
    cam.samples_per_pixel = 50;
    cam.max_depth         = 20;

    cam.vfov     = 40;
    cam.lookfrom = point3(3, 3, 3);
    cam.lookat   = point3(0.5, 0.5, 0.5);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;  // no blur, keep it crisp for testing

    cam.render(world);
}