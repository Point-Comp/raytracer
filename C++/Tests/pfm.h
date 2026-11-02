
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

inline void write_pfm(const std::string& path, int width, int height, const std::vector<float>& pixels){


    std::ofstream out{path, std::ios::out | std::ios::binary};
    out << "PF\n";   
    out << width << " " << height<< "\n";
    out << "-1.0\n";  
    out.write((const char*)pixels.data(), width * height * 3 * sizeof(float));

}

