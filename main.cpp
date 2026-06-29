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

    // ground plane for reference
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    // load the test model — THIS is the only triangle source
    auto mesh_mat = make_shared<lambertian>(color(0.9, 0.2, 0.2));
    load_obj("test.obj", world, mesh_mat);

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 50;
    cam.max_depth         = 20;

    cam.vfov     = 40;
    cam.lookfrom = point3(0, 1, 3);
    cam.lookat   = point3(0, 1, -1);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}