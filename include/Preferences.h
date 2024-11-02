#pragma once

// TODO: Save preferences to .ini file in user folder %AppData% and load from it as well
class Preferences {
  public:
    Preferences(const Preferences &) = delete;
    Preferences(Preferences &&) = delete;
    Preferences &operator=(const Preferences &) = delete;
    Preferences &operator=(Preferences &&) = delete;

    static Preferences *Get();

    Preferences();
    ~Preferences();

    void Load();
    void Save();

    void SaveParameter(const std::wstring &key, const std::wstring &value);
    std::wstring LoadParameter(const std::wstring &key);

    int _tile_resolution;
    int _lazy_save_count;
    std::uint32_t _tile_default_color;

    int _file_recents_max;
    std::queue<std::filesystem::path> _file_recents;
    std::filesystem::path _file_last_openned;
    float _brush_step;
};
