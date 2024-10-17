#pragma once
#include <cstdint>
#include <filesystem>
#include <glm/vec2.hpp>
#include <map>
#include <vector>
#include <optional>
#include <span>


/* Custom file format for Mashiro
 * INFO
 * BOM:          endiness[2]
 * file_type:    char[6]
 * file_version: uint8_t[4] 255.255.255.255
 * header_size:  uint64_t[1]
 *
 * HEADER
 * tile_resolution uint32_t[1]
 * entry:
 *   coord: int32_t[2]
 *   start: uint64_t[1];
 *   len:   uint64_t[1];
 *
 *
 * BODY
 * saved webp losless of all the texture available in uint8_t
 */

class Tile;

class File {
  public:
    File(const File &) = delete;
    File(File &&) = delete;
    File &operator=(const File &) = delete;
    File &operator=(File &&) = delete;

    File(int resolution);
    File(std::filesystem::path filename);
    ~File();

    static std::optional<std::unique_ptr<File>> Find(std::filesystem::path directory);

    void SetFilename(std::filesystem::path filename);
    std::filesystem::path GetFilename() const;

    void Open(std::filesystem::path filename);
    void Save();
    void SaveAs(std::filesystem::path filename);

    std::vector<std::pair<int, int>> GetSavedTileLocation() const;
    int GetTileResolution() const;

    std::optional<std::vector<uint32_t>> GetTexture(int x, int y);
    void SaveTexture(int x, int y, std::span<uint32_t> pixels, int compression = 4);

  private:
    // store all the tile and is referenced by the canvas after
    std::filesystem::path _filename;
    bool _save_on_close;

    // INFO
    struct Info {
        char _type[4];
        uint8_t _version[4];
        uint64_t _size;
        uint32_t _header_count;
        uint32_t _resolution;
    } _info;

    std::map<std::pair<int, int>, size_t> _textures_indexes;

    // BODY
    std::vector<std::vector<uint8_t>> _pngs;

    struct TileHeader {
        int32_t coord[2];
        uint64_t start;
        uint64_t len;
    };

};
