#ifndef COLOR_H
#define COLOR_H
#include <iostream>
#include "interval.h"
#include "vec3.h"
#include <vector>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using color = vec3;

inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0)
        return std::sqrt(linear_component);

    return 0;
}


inline void write_color(std::ostream& out, const color& pixel_color) { //this is PPM, responsible for image output
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();


    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);
    


    static const interval intensity(0.000, 0.999);
    int rbyte = int(256 * intensity.clamp(r));
    int gbyte = int(256 * intensity.clamp(g));
    int bbyte = int(256 * intensity.clamp(b));




    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

//PFM writer is necessary for error metrics

inline void write_pfm(const std::string& path, int width, int height, const std::vector<float>& pixels){


    std::ofstream out{path, std::ios::out | std::ios::binary};
    out << "PF\n";   
    out << width << " " << height<< "\n";
    out << "-1.0\n";  
    out.write((const char*)pixels.data(), width * height * 3 * sizeof(float));

}

#endif