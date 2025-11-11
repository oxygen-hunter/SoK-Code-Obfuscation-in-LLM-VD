#include <iostream>
#include <string>
#include <vector>
#include <Python.h>
#include <stdexcept>

extern "C" {
#include <libtiff/tiffio.h>
}

std::vector<std::string> repro_read_strip = {
    "images/crash_1.tif",
    "images/crash_2.tif",
};

void load_image(const std::string& path) {
    TIFF* tiff = TIFFOpen(path.c_str(), "r");
    if (!tiff) {
        throw std::runtime_error("Cannot open TIFF file: " + path);
    }
    TIFFClose(tiff);
}

int main() {
    for (const auto& path : repro_read_strip) {
        try {
            load_image(path);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    return 0;
}