#ifndef HDR_H
#define HDR_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vec3.h"
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>

class HDR {
    private:
    float* data;
    int width;
    int height;
    int channels;
    
    public:
    HDR(const std::string& filename)
    : data(nullptr), width(0), height(0), channels(0) {
        
        data = stbi_loadf(filename.c_str(), &width, &height, &channels, 3);
        
        if (data == nullptr) {
            std::cerr << "Failed to load HDR image: "
            << filename << std::endl;
        }
    }
    
    ~HDR() {
        stbi_image_free(data);
    }
    
    vec3 sample(const vec3& direction) const {
        if (data == nullptr) {
            return vec3(0, 0, 0);
        }
        
        // acos needs a unit vector, or y can exceed 1 and it returns NaN
        vec3 d = unit_vector(direction);
        
        // Convert direction into spherical coordinates
        double theta = std::acos(d.y());
        double phi = std::atan2(-d.z(), d.x()) + pi;
        
        // Convert spherical coordinates into image coordinates
        double u = phi / (2.0 * pi);
        double v = theta / pi;
        
        int x = static_cast<int>(u * width);
        int y = static_cast<int>(v * height);
        
        // Keep coordinates inside the image
        x = std::max(0, std::min(x, width - 1));
        y = std::max(0, std::min(y, height - 1));
        
        int index = (y * width + x) * 3;
        
        return vec3(
            data[index],
            data[index + 1],
            data[index + 2]
        );
    }
};

#endif