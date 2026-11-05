#include "../rtweekend.h"
#include "../merl.h"
#include <iostream>
#include <cmath>

static int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::cout << "  FAIL: " << what << "\n";
        failures++;
    }
}

// Monte Carlo estimate of the red-channel reflectance, integral of f * cos(theta_i) over wi
double sampled_albedo(const material& mat, const vec3& wo, int n, int& accepted) {
    double sum = 0;
    accepted = 0;
    for (int s = 0; s < n; s++) {
        bsdf_sample bs;
        if (mat.sample_bsdf(wo, bs)) {
            accepted++;
            sum += bs.value.x() * bs.direction.y() / bs.pdf;
        }
    }
    return sum / n;
}

int main() {
    const std::string path = "../Data/merl/gold-metallic-paint.binary";
    merl mixture(path, "../Data/ggx_params.json", "gold-metallic-paint");
    merl cosine(path);

    const int n_theta = 1000;
    const int n_phi   = 2000;
    const int n_samples = 2000000;

    for (double theta_o_deg : {0.0, 30.0, 60.0, 80.0}) {
        double theta_o = degrees_to_radians(theta_o_deg);
        vec3 wo(std::sin(theta_o), std::cos(theta_o), 0);

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
                pdf_integral += mixture.pdf(wi, wo) * dw;
                albedo_grid  += mixture.eval(wi, wo).x() * wi.y() * dw;
            }
        }

        int accepted_mix, accepted_cos;
        double albedo_mix = sampled_albedo(mixture, wo, n_samples, accepted_mix);
        double albedo_cos = sampled_albedo(cosine,  wo, n_samples, accepted_cos);
        double accept_rate = double(accepted_mix) / n_samples;

        // The returned pdf must be the combined pdf, whichever branch drew the direction
        double worst_pdf_mismatch = 0;
        for (int s = 0; s < 100000; s++) {
            bsdf_sample bs;
            if (mixture.sample_bsdf(wo, bs)) {
                double expected = mixture.pdf(bs.direction, wo);
                worst_pdf_mismatch = std::fmax(worst_pdf_mismatch, std::fabs(bs.pdf - expected) / expected);
            }
        }

        std::cout << "theta_o " << theta_o_deg
                  << ": pdf integral " << pdf_integral << " (accept rate " << accept_rate << ")"
                  << ", albedo grid " << albedo_grid
                  << " mixture " << albedo_mix
                  << " cosine " << albedo_cos
                  << ", worst pdf mismatch " << worst_pdf_mismatch << "\n";

        check(std::fabs(pdf_integral - accept_rate) < 0.01, "combined pdf does not integrate to the acceptance rate");
        check(worst_pdf_mismatch < 1e-12, "sample_bsdf pdf differs from pdf()");
        check(std::fabs(albedo_mix - albedo_grid) < 0.01 * albedo_grid, "mixture albedo disagrees with grid");
        check(std::fabs(albedo_cos - albedo_grid) < 0.01 * albedo_grid, "cosine albedo disagrees with grid");
    }

    std::cout << (failures == 0 ? "all passed" : "FAILURES") << "\n";
    return failures == 0 ? 0 : 1;
}
