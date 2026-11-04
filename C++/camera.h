#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include "onb.h"
#include <vector>
#include "hdr.h"

class camera {
  public:
    double aspect_ratio = 1.0;
    int    image_width  = 100;
    int    samples_per_pixel = 10;
    int    max_depth         = 10;

    double vfov = 90;
    point3 lookfrom = point3(0,0,0);
    point3 lookat   = point3(0,0,-1);
    vec3   vup      = vec3(0,1,0);

    double defocus_angle = 0;
    double focus_dist = 10;
    point3 light_pos = point3(5, 5, 5);
    color  light_color = color(1, 1, 1);
    shared_ptr<HDR> env;


    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0,0,0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                framebuffer[j * image_width + i] = pixel_samples_scale * pixel_color;
            }
        }

        for(int j=0; j<image_height; j++){
            for(int i=0; i<image_width; i++){
                write_color(std::cout, framebuffer[j * image_width + i]);
            }
        }

        std::vector<float> floats;
        floats.reserve(image_width * image_height * 3);

        for (int j = image_height - 1; j >= 0; j--) {
            for (int i = 0; i < image_width; i++) {
                const color& c = framebuffer[j * image_width + i];
                floats.push_back(float(c.x()));
                floats.push_back(float(c.y()));
                floats.push_back(float(c.z()));
            }
        }

        write_pfm("out.pfm", image_width, image_height, floats);


        std::clog << "\rDone.                 \n";
    }

  private:
    int    image_height;
    double pixel_samples_scale;
    point3 center;
    point3 pixel00_loc;
    vec3   pixel_delta_u;
    vec3   pixel_delta_v;
    vec3   u, v, w;
    vec3   defocus_disk_u;
    vec3   defocus_disk_v;
    std::vector<color> framebuffer;

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        pixel_samples_scale = 1.0 / samples_per_pixel;
        center = lookfrom;
        framebuffer.resize(image_width * image_height);

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j) const {

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }


    color ray_color(const ray& r, int depth, const hittable& world) const {
    if (depth <= 0)
        return color(0,0,0);

    hit_record rec;

    if (world.hit(r, interval(0.001, infinity), rec)) {
        if (auto ph = dynamic_cast<const phong*>(rec.mat.get())) {
            vec3 N = unit_vector(rec.normal);
            vec3 L = unit_vector(light_pos - rec.p);

            double dist_to_light = (light_pos - rec.p).length();
            ray shadow_ray(rec.p, L);
            hit_record shadow_rec;
            bool in_shadow = world.hit(shadow_ray, interval(0.001, dist_to_light), shadow_rec);

            color result = ph->albedo * 0.1;

            if (!in_shadow) {
                double diff = std::fmax(0.0, dot(N, L));
                result += diff * ph->albedo * light_color;

                vec3 V = unit_vector(-r.direction());
                vec3 R = reflect(-L, N);
                double spec = std::pow(std::fmax(0.0, dot(R, V)), ph->shininess);
                result += spec * light_color;
            }

            return result;
        }

        onb frame(rec.normal);
        vec3 wo = frame.to_local(unit_vector(-r.direction()));

        bsdf_sample bs;
        if (rec.mat->sample_bsdf(wo, bs)) {
            ray scattered(rec.p, frame.to_world(bs.direction));
            double cos_theta = bs.direction.y();
            color weight = bs.value * cos_theta / bs.pdf;
            return weight * ray_color(scattered, depth-1, world);
        }

        ray scattered;
        color attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth-1, world);
        return color(0,0,0);
    }

    vec3 unit_direction = unit_vector(r.direction());

    if (env)
        return env->sample(unit_direction);

    auto a = 0.5*(unit_direction.y() + 1.0);
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}
};

#endif