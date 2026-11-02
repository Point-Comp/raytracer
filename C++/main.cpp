#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"
#include "mesh.h"

#include <iostream>

int main() {
    hittable_list world;

    auto ground = make_shared<phong>(color(0.5, 0.5, 0.5), 1.0);
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground));

    // Teapot temporarily disabled for HDR testing
    // auto teapot_mat = make_shared<phong>(color(0.9, 0.2, 0.2), 32.0);
    // load_obj("utah_teapot.obj", world, teapot_mat);

    // Reflective metal sphere
    auto metal_mat = make_shared<metal>(color(0.8, 0.8, 0.8), 0.0);
    world.add(make_shared<sphere>(point3(2, 1, 0), 1.0, metal_mat));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 64;
    cam.max_depth         = 20;

    cam.env = make_shared<HDR>("../Data/kloppenheim_06_2k.hdr");

    cam.vfov     = 35;
    cam.lookfrom = point3(0, 3, 8);
    cam.lookat   = point3(0, 1, 0);
    cam.vup      = vec3(0, 1, 0);
    cam.defocus_angle = 0;

    cam.light_pos = point3(5, 5, 5);

    cam.render(world);
}