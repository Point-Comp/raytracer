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
    auto tri_mat = make_shared<lambertian>(color(0.9, 0.2, 0.2));
    world.add(make_shared<triangle>(
        point3(-1, 0, -1),
        point3( 1, 0, -1),
        point3( 0, 2, -1),
        tri_mat));

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;   // small + fast
    cam.samples_per_pixel = 50;
    cam.max_depth         = 20;

    cam.vfov     = 40;
    cam.lookfrom = point3(0, 1, 3);  // looking at the triangle from the front
    cam.lookat   = point3(0, 1, -1);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;  // no blur, keep it crisp for testing

    cam.render(world);
}