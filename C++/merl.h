#ifndef MERL_H
#define MERL_H

#include "material.h"
#include "ggx.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class merl : public material {
  public:
    merl(const std::string& filename) {
        load(filename);
    }

    merl(const std::string& filename, const std::string& params_path, const std::string& material_name)
      : lobe(make_shared<ggx>(params_path, material_name)), p_ggx(0.8) {
        load(filename);
    }

    color eval(const vec3& wi, const vec3& wo) const override {
        if (wi.y() <= 0 || wo.y() <= 0) return color(0,0,0);

        vec3 in  = unit_vector(vec3(wi.z(), wi.x(), wi.y()));
        vec3 out = unit_vector(vec3(wo.z(), wo.x(), wo.y()));

        vec3 half = unit_vector(in + out);
        double theta_half = std::acos(std::fmin(1.0, std::fmax(-1.0, half.z())));
        double phi_half   = std::atan2(half.y(), half.x());

        vec3 normal(0, 0, 1);
        vec3 bi_normal(0, 1, 0);
        vec3 temp = rotate_vector(in, normal, -phi_half);
        vec3 diff = rotate_vector(temp, bi_normal, -theta_half);

        double theta_diff = std::acos(std::fmin(1.0, std::fmax(-1.0, diff.z())));
        double phi_diff   = std::atan2(diff.y(), diff.x());

        size_t ind = phi_diff_index(phi_diff)
                   + theta_diff_index(theta_diff) * (res_phi_d)
                   + theta_half_index(theta_half) * (res_phi_d * res_theta_d);
        size_t channel = size_t(res_theta_h) * res_theta_d * res_phi_d;

        return color(
            std::fmax(0.0, brdf[ind]               * red_scale),
            std::fmax(0.0, brdf[ind + channel]     * green_scale),
            std::fmax(0.0, brdf[ind + 2 * channel] * blue_scale)
        );
    }

    double pdf(const vec3& wi, const vec3& wo) const override {
        if (wi.y() <= 0 || wo.y() <= 0) return 0.0;
        double cosine = wi.y() / pi;
        if (!lobe) return cosine;
        return p_ggx * lobe->pdf(wi, wo) + (1 - p_ggx) * cosine;
    }

    bool sample_bsdf(const vec3& wo, bsdf_sample& out) const override {
        if (wo.y() <= 0) return false;

        vec3 dir;
        if (lobe && random_double() < p_ggx) {
            dir = lobe->sample_direction(wo);
        } else {
            dir = vec3(0,1,0) + random_unit_vector();
            if (dir.near_zero()) dir = vec3(0,1,0);
        }
        if (dir.y() <= 0) return false;
        out.direction = unit_vector(dir);

        out.value = eval(out.direction, wo);
        out.pdf   = pdf(out.direction, wo);
        return out.pdf > 0;
    }

  private:
    void load(const std::string& filename) {
        std::ifstream file(filename, std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("merl: cannot open file: " + filename);

        int dims[3];
        file.read((char*)dims, sizeof(dims));
        size_t n = size_t(dims[0]) * dims[1] * dims[2];
        if (n != size_t(res_theta_h) * res_theta_d * res_phi_d)
            throw std::runtime_error("merl: unexpected dimensions in " + filename);

        brdf.resize(3 * n);
        file.read((char*)brdf.data(), 3 * n * sizeof(double));
        if (!file)
            throw std::runtime_error("merl: file too short: " + filename);
    }

    static constexpr int res_theta_h = 90;
    static constexpr int res_theta_d = 90;
    static constexpr int res_phi_d   = 180;

    static constexpr double red_scale   = 1.0  / 1500.0;
    static constexpr double green_scale = 1.15 / 1500.0;
    static constexpr double blue_scale  = 1.66 / 1500.0;

    std::vector<double> brdf;
    shared_ptr<ggx> lobe;
    double p_ggx = 0.0;

    static vec3 rotate_vector(const vec3& v, const vec3& axis, double angle) {
        double c = std::cos(angle);
        double s = std::sin(angle);
        return v * c + axis * (dot(axis, v) * (1 - c)) + cross(axis, v) * s;
    }

    static int theta_half_index(double theta_half) {
        if (theta_half <= 0.0) return 0;
        double t = std::sqrt(theta_half / (pi / 2.0) * res_theta_h * res_theta_h);
        int i = int(t);
        return std::max(0, std::min(i, res_theta_h - 1));
    }

    static int theta_diff_index(double theta_diff) {
        int i = int(theta_diff / (pi * 0.5) * res_theta_d);
        return std::max(0, std::min(i, res_theta_d - 1));
    }

    static int phi_diff_index(double phi_diff) {
        if (phi_diff < 0.0) phi_diff += pi;
        int i = int(phi_diff / pi * res_phi_d);
        return std::max(0, std::min(i, res_phi_d - 1));
    }
};

#endif
