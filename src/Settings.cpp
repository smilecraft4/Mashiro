#include "Settings.h"

Settings::Settings() {
    recent_file_max_len = 10;
    _filename = "./settings.ini";
    _tile_resolution = 256;
    _recent_files = std::vector<std::filesystem::path>();
    _recent_files.reserve(recent_file_max_len);
}

void Settings::AddRecentFile(std::filesystem::path filename) {
    _recent_files.push_back(filename);
}
