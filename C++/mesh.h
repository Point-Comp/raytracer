#ifndef MESH_H
#define MESH_H

#include "hittable_list.h"
#include "triangle.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

static int parse_face_index(const std::string& token) {
    return std::stoi(token.substr(0, token.find('/')));
}

inline void load_obj(const std::string& filename, hittable_list& world, shared_ptr<material> mat) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("load_obj: cannot open file: " + filename);

    std::vector<point3> verts;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            double x, y, z;
            ss >> x >> y >> z;
            verts.emplace_back(x, y, z);
        } else if (token == "f") {
            std::vector<int> indices;
            std::string ft;
            while (ss >> ft)
                indices.push_back(parse_face_index(ft));


            for (int i = 1; i + 1 < (int)indices.size(); ++i) {
                const point3& a = verts[indices[0] - 1];
                const point3& b = verts[indices[i] - 1];
                const point3& c = verts[indices[i + 1] - 1];
                world.add(make_shared<triangle>(a, b, c, mat));
            }
        }

    }
}

#endif