#include "../rtweekend.h"
#include "../merl.h"
#include <fstream>
#include <iostream>
#include <cmath>

static int failures = 0;

bool close(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) <= eps * std::fmax(1.0, std::fabs(b));
}

int main() {
    merl mat("../Data/merl/gold-metallic-paint.binary");

    std::ifstream file("Tests/merl_reference.txt");
    if (!file.is_open()) {
        std::cout << "cannot open Tests/merl_reference.txt\n";
        return 1;
    }

    double ix, iy, iz, ox, oy, oz, r, g, b;
    int count = 0;
    while (file >> ix >> iy >> iz >> ox >> oy >> oz >> r >> g >> b) {
        color got = mat.eval(vec3(ix, iy, iz), vec3(ox, oy, oz));
        count++;

        if (!close(got.x(), r) || !close(got.y(), g) || !close(got.z(), b)) {
            std::cout << "  FAIL pair " << count << ": expected ("
                      << r << ", " << g << ", " << b << ") got ("
                      << got.x() << ", " << got.y() << ", " << got.z() << ")\n";
            failures++;
        }
    }

    std::cout << count - failures << "/" << count << " pairs match\n";
    return failures == 0 && count > 0 ? 0 : 1;
}
