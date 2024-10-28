#pragma once
#include "Framework.h"
#include <filesystem>
#include <glm/vec2.hpp>
#include <map>
#include <optional>
#include <span>
#include <vector>

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

class File {
  public:
    File(const File &) = delete;
    File(File &&) = delete;
    File &operator=(const File &) = delete;
    File &operator=(File &&) = delete;
    File();
    ~File();

    static std::unique_ptr<File> New(std::filesystem::path filename);
    static std::unique_ptr<File> Open(std::filesystem::path filename);

    void Rename(std::filesystem::path filename);
    std::filesystem::path GetFilename() const;

    bool IsSaved() const;
    bool IsNew() const;

    void Save(std::filesystem::path filename);
    tstring GetDisplayName();

    std::vector<std::pair<int, int>> GetSavedTileLocation() const;
    int GetTileResolution() const;

    bool HasTile(int x, int y) const;
    std::vector<uint32_t> ReadTileTexture(int x, int y);
    void WriteTileTexture(int x, int y, std::vector<uint32_t> pixels, int compression = 4);

  private:
    // store all the tile and is referenced by the canvas after

    std::filesystem::path _filename;
    bool _save_on_close;
    bool _saved;
    bool _new;

    // INFO
    struct Info {
        char _type[4];
        std::uint8_t _version[4];
        std::uint64_t _size;
        std::uint32_t _header_count;
        std::uint32_t _resolution;
    } _info;

    std::map<std::pair<int, int>, size_t> _textures_indexes;

    // BODY
    std::vector<std::vector<uint8_t>> _pngs;

    struct TileHeader {
        std::int32_t coord[2];
        std::uint64_t start;
        std::uint64_t len;
    };
};
