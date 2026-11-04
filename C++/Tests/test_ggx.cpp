#include "../rtweekend.h"
#include "../ggx.h"
#include <iostream>
#include <cmath>

static int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::cout << "  FAIL: " << what << "\n";
        failures++;
    }
}

int main() {
    const int n_theta = 1000;
    const int n_phi   = 2000;
    const int n_samples = 1000000;

    for (double roughness : {0.3, 0.6, 1.0}) {
        ggx mat(color(0.9, 0.6, 0.3), roughness);

        for (double theta_o_deg : {0.0, 30.0, 60.0, 80.0}) {
            double theta_o = degrees_to_radians(theta_o_deg);
            vec3 wo(std::sin(theta_o), std::cos(theta_o), 0);

            // Midpoint-rule integration over the upper hemisphere
            double pdf_integral = 0;
            double albedo_grid  = 0;
            double d_theta = (pi / 2) / n_theta;
            double d_phi   = (2 * pi) / n_phi;
            for (int i = 0; i < n_theta; i++) {
                double theta = (i + 0.5) * d_theta;
                double dw = std::sin(theta) * d_theta * d_phi;
                for (int j = 0; j < n_phi; j++) {
                    double phi = (j + 0.5) * d_phi;
                    vec3 wi(std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi));
                    pdf_integral += mat.pdf(wi, wo) * dw;
                    albedo_grid  += mat.eval(wi, wo).x() * wi.y() * dw;
                }
            }

            // Same quantities estimated by sampling
            int accepted = 0;
            double albedo_sampled = 0;
            for (int s = 0; s < n_samples; s++) {
                bsdf_sample bs;
                if (mat.sample_bsdf(wo, bs)) {
                    accepted++;
                    albedo_sampled += bs.value.x() * bs.direction.y() / bs.pdf;
                }
            }
            double accept_rate = double(accepted) / n_samples;
            albedo_sampled /= n_samples;

            std::cout << "roughness " << roughness << ", theta_o " << theta_o_deg
                      << ": pdf integral " << pdf_integral
                      << " (accept rate " << accept_rate << ")"
                      << ", albedo grid " << albedo_grid
                      << " sampled " << albedo_sampled << "\n";

            check(std::fabs(pdf_integral - accept_rate) < 0.01, "pdf does not integrate to the acceptance rate");
            check(pdf_integral <= 1.01, "pdf integrates to more than 1");
            check(std::fabs(albedo_grid - albedo_sampled) < 0.01 * std::fmax(albedo_grid, 0.1),
                  "sampled albedo disagrees with grid albedo");
        }
    }

    std::cout << (failures == 0 ? "all passed" : "FAILURES") << "\n";
    return failures == 0 ? 0 : 1;
}
