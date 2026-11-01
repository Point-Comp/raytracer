
#include <vector>
#include "pfm.h"
int main()
{
    std::vector<float> pixels = {
        1.0f, 0.0f, 0.0f, // top-left: red
        0.0f, 2.5f, 0.0f, // top-right: bright green
        0.0f, 0.0f, 0.3f, // bottom-left: dim blue
        0.5f, 0.5f, 0.5f  // bottom-right: grey
    };
    write_pfm("test.pfm", 2, 2, pixels);
}