#include "utility.hpp"
#include <filesystem>

namespace fs = std::filesystem;

void create_frames_directory(const std::string& path) {
    if (!fs::exists(path)) {
        fs::create_directory(path);
    }
}