#pragma once
#include <cstdint>
#include <filesystem>
#include <glm/vec2.hpp>
#include <map>
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

class Tile;

class File {
  public:
    File(std::filesystem::path filename){};
    ~File(){};

    void Open(std::filesystem::path filename);
    void Save(std::filesystem::path filename);
    void Close();

    Tile *GetTile(int x, int y){};

  private:
    // store all the tile and is referenced by the canvas after

    // INFO
    char bom[2];
    char type[4];
    uint8_t version[2];
    uint32_t size;

    // HEADER
    uint32_t resolution;

    // BODY
    std::map<std::pair<int, int>, size_t> _coord_indexes;
    std::vector<Tile> _textures;
};