#ifndef GGX_H
#define GGX_H

#include "material.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

// Microfacet reflection with the GGX distribution, in the local shading frame (normal = +y)
class ggx : public material {
  public:
    ggx(const color& f0, double roughness)
      : f0(f0), alpha(std::fmax(1e-3, roughness * roughness)) {}

    // Loads parameters fitted by Python/fit_ggx.py, e.g. ggx("../Data/ggx_params.json", "gold-metallic-paint")
    ggx(const std::string& params_path, const std::string& material_name) {
        std::ifstream file(params_path);
        if (!file.is_open())
            throw std::runtime_error("ggx: cannot open " + params_path);
        std::stringstream ss;
        ss << file.rdbuf();
        std::string json = ss.str();

        size_t entry = json.find("\"" + material_name + "\"");
        if (entry == std::string::npos)
            throw std::runtime_error("ggx: no entry for " + material_name + " in " + params_path);
        size_t end = json.find('}', entry);

        double roughness = read_number(json, json.find("\"roughness\"", entry), end, material_name);
        size_t f0_pos = json.find('[', json.find("\"f0\"", entry));
        double c[3];
        for (int i = 0; i < 3; i++) {
            c[i] = read_number(json, f0_pos, end, material_name);
            f0_pos = json.find(',', f0_pos + 1);
        }

        f0 = color(c[0], c[1], c[2]);
        alpha = std::fmax(1e-3, roughness * roughness);
    }

    color eval(const vec3& wi, const vec3& wo) const override {
        if (wi.y() <= 0 || wo.y() <= 0) return color(0,0,0);

        vec3 h = unit_vector(wi + wo);
        return D(h) * F(dot(wi, h)) * G1(wi) * G1(wo) / (4 * wi.y() * wo.y());
    }

    // Sampling picks h with density D(h) * cos(theta_h); reflecting about h
    // changes variables to wi, with Jacobian 1 / (4 * dot(wo, h)).
    double pdf(const vec3& wi, const vec3& wo) const override {
        if (wi.y() <= 0 || wo.y() <= 0) return 0.0;

        vec3 h = unit_vector(wi + wo);
        double wo_dot_h = dot(wo, h);
        if (wo_dot_h <= 0) return 0.0;
        return D(h) * h.y() / (4 * wo_dot_h);
    }

    // Draws h from D(h) * cos(theta_h) and reflects wo about it; the result can be below the surface
    vec3 sample_direction(const vec3& wo) const {
        double u1 = random_double();
        double u2 = random_double();
        double tan2_theta = alpha * alpha * u1 / (1 - u1);
        double cos_theta  = 1 / std::sqrt(1 + tan2_theta);
        double sin_theta  = std::sqrt(std::fmax(0.0, 1 - cos_theta * cos_theta));
        double phi        = 2 * pi * u2;
        vec3 h(sin_theta * std::cos(phi), cos_theta, sin_theta * std::sin(phi));

        return 2 * dot(wo, h) * h - wo;
    }

    bool sample_bsdf(const vec3& wo, bsdf_sample& out) const override {
        if (wo.y() <= 0) return false;

        out.direction = sample_direction(wo);
        if (out.direction.y() <= 0) return false;

        out.value = eval(out.direction, wo);
        out.pdf   = pdf(out.direction, wo);
        return out.pdf > 0;
    }

  private:
    color  f0;
    double alpha;

    // Parses the first number after pos (skipping the key, ':' and '['), stopping before end
    static double read_number(const std::string& json, size_t pos, size_t end, const std::string& name) {
        if (pos == std::string::npos || pos >= end)
            throw std::runtime_error("ggx: malformed entry for " + name);
        size_t start = json.find_first_of("-0123456789", json.find_first_of(":[,", pos) + 1);
        if (start == std::string::npos || start >= end)
            throw std::runtime_error("ggx: malformed entry for " + name);
        return std::strtod(json.c_str() + start, nullptr);
    }

    double D(const vec3& h) const {
        double a2 = alpha * alpha;
        double c2 = h.y() * h.y();
        double denom = c2 * (a2 - 1) + 1;
        return a2 / (pi * denom * denom);
    }

    color F(double cos_theta) const {
        return f0 + (color(1,1,1) - f0) * std::pow(1 - std::fmax(0.0, cos_theta), 5);
    }

    double G1(const vec3& v) const {
        double c  = v.y();
        double a2 = alpha * alpha;
        return 2 * c / (c + std::sqrt(a2 + (1 - a2) * c * c));
    }
};

#endif
