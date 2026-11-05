#include "rtweekend.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangle.h"
#include "mesh.h"
#include "bvh.h"
#include "merl.h"
#include "ggx.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    std::string teapot_name = argc > 1 ? argv[1] : "ggx";

    hittable_list world;

    auto ground = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground));

    shared_ptr<material> teapot_mat;
    if (teapot_name == "merl")
        teapot_mat = make_shared<merl>("../Data/merl/gold-metallic-paint.binary", "../Data/ggx_params.json", "gold-metallic-paint");
    else if (teapot_name == "merl-cosine")
        teapot_mat = make_shared<merl>("../Data/merl/gold-metallic-paint.binary");
    else
        teapot_mat = make_shared<ggx>("../Data/ggx_params.json", "gold-metallic-paint");
    load_obj("utah_teapot.obj", world, teapot_mat);

    auto mirror = make_shared<metal>(color(0.9, 0.9, 0.9), 0.0);
    world.add(make_shared<sphere>(point3(2.5, 1.0, 1.0), 1.0, mirror));

    // Wrap the scene in a BVH so each ray tests ~log2(n) boxes instead of every object.
    // Set use_bvh to false to render the plain list, for comparing against the old code.
    const bool use_bvh = true;
    bvh_node bvh(world);

    camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 200;
    cam.samples_per_pixel = argc > 2 ? std::stoi(argv[2]) : 20;
    cam.max_depth         = 10;
    cam.env = make_shared<HDR>("../Data/kloppenheim_06_2k.hdr");

    cam.vfov     = 35;
    cam.lookfrom = point3(0, 3, 8);
    cam.lookat   = point3(0, 1, 0);
    cam.vup      = vec3(0, 1, 0);
    cam.defocus_angle = 0;

    if (use_bvh)
        cam.render(bvh);
    else
        cam.render(world);
}