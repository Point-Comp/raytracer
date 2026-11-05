#include "../rtweekend.h"
#include "../ggx.h"
#include <fstream>
#include <iostream>
#include <cmath>

static int failures = 0;

bool close(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps * std::fmax(1.0, std::fabs(b));
}

int main() {
    std::ifstream file("Tests/ggx_reference.txt");
    if (!file.is_open()) {
        std::cout << "cannot open Tests/ggx_reference.txt\n";
        return 1;
    }

    std::string name;
    double roughness, fr, fg, fb, ix, iy, iz, ox, oy, oz, r, g, b;
    int count = 0;
    while (file >> name >> roughness >> fr >> fg >> fb >> ix >> iy >> iz >> ox >> oy >> oz >> r >> g >> b) {
        shared_ptr<ggx> mat = (name == "fixed")
            ? make_shared<ggx>(color(fr, fg, fb), roughness)
            : make_shared<ggx>("../Data/ggx_params.json", name);

        color got = mat->eval(vec3(ix, iy, iz), vec3(ox, oy, oz));
        count++;

        if (!close(got.x(), r) || !close(got.y(), g) || !close(got.z(), b)) {
            std::cout << "  FAIL line " << count << " (" << name << ", roughness " << roughness
                      << "): expected (" << r << ", " << g << ", " << b << ") got ("
                      << got.x() << ", " << got.y() << ", " << got.z() << ")\n";
            failures++;
        }
    }

    std::cout << count - failures << "/" << count << " values match\n";
    return failures == 0 && count > 0 ? 0 : 1;
}
