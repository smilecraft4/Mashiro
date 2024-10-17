#pragma once
#include <filesystem>
#include <functional>

// .ini file

// Think of a shortcut architecture that
// struct Shortcut {
//    std::string _name;
//    std::string _description;
//    int mods;
//    int key;
//
//    std::function<void()> OnPress;
//    std::function<void()> OnHold;
//    std::function<void()> OnReleased;
//    std::function<void(int, int)> OnRebind;
//
//    void Rebind(int key, int mods);
//    void Release();
//    void Hold();
//    void Press();
//
//    Shortcut(const std::string& name, const std::string& description);
//};

class Settings {
  public:
    Settings();

    void Open(){};
    void Save(){};

    void AddRecentFile(std::filesystem::path filename);
    void LoadSettings(){};

    // General
    size_t recent_file_max_len;
    std::vector<std::filesystem::path> _recent_files;

    // User preferences
        // Visual
        // Background color
        // Transparent Window

        // Shortcuts

    // Advanced
    std::uint32_t _tile_resolution;

  private:
    // Personal
    std::filesystem::path _filename;
};