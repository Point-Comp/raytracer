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


    auto red_phong = make_shared<phong>(color(0.9, 0.2, 0.2), 32.0);
    world.add(make_shared<sphere>(point3(-1.5, 1, 0), 1.0, red_phong));


    auto blue_phong = make_shared<phong>(color(0.2, 0.4, 0.9), 32.0);
    load_obj("cube.obj", world, blue_phong);

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 30;
    cam.max_depth         = 20;

    cam.vfov     = 35;
    cam.lookfrom = point3(5, 3, 8);
    cam.lookat   = point3(0, 0.75, 0);
    cam.vup      = vec3(0, 1, 0);
    cam.defocus_angle = 0;

    cam.light_pos = point3(5, 5, 5);

    cam.render(world);
}