#pragma once
#include <cstdint>
#include <filesystem>
#include <glm/vec2.hpp>
#include <map>
#include <vector>

/* Custom file format for Mashiro
 * INFO
 * file_type:    char[8]
 * file_version: uint8_t[4]
 * header_size:  uint64_t[1]
 *
 * HEADER
 * tile_resolution uint32_t[1]
 * entry:
 *   coord: int32_t[2]
 *   index: uint64_t[1];
 *
 *
 * BODY
 * saved webp losless of all the texture available
 */

class Tile;

struct Entry {
    uint16_t layer;
    int32_t coord[2];
    uint64_t offset;
    uint64_t length;
};

struct Format {
    // INFO
    char type[4];
    uint8_t version[2];
    uint32_t size;

    // HEADER
    // compress the header
    // add support for layers ??
    uint64_t header_size;
    uint16_t resolution;
    Entry entries;

    // BODY
    // data is already saved in compressed webp lossless RGBA format
    uint8_t _data;
};

class File {
  public:
    File(std::filesystem::path filename){};
    ~File(){};

    bool LoadTile(glm::ivec2 coord, Tile *tile){};
    bool SaveTile(glm::ivec2 coord, Tile *tile){};

    bool SaveAll();

  private:
    // store all the tile and is referenced by the canvas after
    std::filesystem::path _filename;

    std::map<std::pair<int, int>, size_t> _texture_indexes;
    std::vector<std::vector<std::uint32_t>> _texture_data;
};