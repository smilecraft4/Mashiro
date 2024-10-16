#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch.hpp>
#include <future>
#include <iostream>

#include "../src/File.h"

constexpr int tile_resolution = 256;
constexpr int tile_compression = 5;
constexpr size_t tile_count_x = 10;
constexpr size_t tile_count_y = 10;
const std::string file_name = "./Save/Test.msh";

constexpr bool multithreading = true;

struct Texture {
    int _width;
    int _height;
    std::vector<uint32_t> _pixels;

    bool operator==(const Texture &other) const {
        return (_width == other._width) && (_height == other._height) && (_pixels == other._pixels);
    }

    Texture(int width, int height, std::vector<uint32_t> pixels) : _width(width), _height(height), _pixels(pixels) {
    }

    Texture(int width, int height) : _width(width), _height(height), _pixels(width * height, 0) {
        for (size_t y = 0; y < _height; y++) {
            for (size_t x = 0; x < _width; x++) {
                size_t i = x + (y * width);

                std::uint8_t r = (float)x / _width * 255;
                std::uint8_t g = (float)y / _height * 255;
                std::uint8_t b = i / _pixels.size();
                std::uint8_t a = rand() % 255;

                _pixels[i] = (a << 24) + (b << 16) + (g << 8) + r;
            }
        }
    }
};

struct Tile {
    int x;
    int y;
    Texture raw;

    bool operator==(const Tile &other) const {
        if ((x == other.x) && (y == other.y) && (raw == other.raw)) {
            return true;
        } else {
            return false;
        }
    }
};

TEST_CASE("Write a raw texture to memory as a png", "[Mashiro][I/O]") {
    std::vector<Tile> tiles_write;
    tiles_write.reserve(tile_count_x * tile_count_y);

    int min_x{}, max_x, min_y, max_y;

    int y = -50 + rand() % 25;
    int x = -50 + rand() % 25;
    min_x = x;
    // y = 0;
    min_y = y;
    for (size_t i_y = 0; i_y < tile_count_y; i_y++) {
        if (i_y != 0) {
            y += tile_count_y + rand() % 20;
            x = -50 + rand() % 25;
            // y = i_y;
        }

        // x = 0;
        min_x = std::min(x, min_x);

        for (size_t i_x = 0; i_x < tile_count_x; i_x++) {
            if (i_x != 0) {
                x += tile_count_x + rand() % 20;
                // x = i_x;
            }

            tiles_write.push_back({x, y, Texture(tile_resolution, tile_resolution)});
        }
        max_x = std::max(x, max_x);
    }
    max_y = y;

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::milliseconds duration;

    {
        start = std::chrono::high_resolution_clock::now();

        File file_write(tile_resolution);
        file_write.SetFilename(file_name);

        if (multithreading) {

            std::vector<std::future<void>> texture_saves;

            for (auto &tile : tiles_write) {
                texture_saves.emplace_back(std::async(std::launch::async, [&file_write, &tile]() {
                    file_write.SaveTexture(tile.x, tile.y, tile.raw._pixels);
                }));
            }

            for (auto &save : texture_saves) {
                save.wait();
            }
        } else {
            for (auto &tile : tiles_write) {
                file_write.SaveTexture(tile.x, tile.y, tile.raw._pixels);
            }
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Saved " << tiles_write.size() << " textrues of " << tile_resolution
                  << " in  : " << duration.count() << "ms. (Multithreading: " << multithreading << ")" << std::endl;

        start = std::chrono::high_resolution_clock::now();

        file_write.Save();
        stop = std::chrono::high_resolution_clock::now();

        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Saved file \"" << file_name << "\" in : " << duration.count() << "ms" << std::endl;
    }

    std::vector<Tile> tiles_read;
    {
        start = std::chrono::high_resolution_clock::now();

        File file_read(file_name);

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Openned file \"" << file_name << "\" in : " << duration.count() << "ms" << std::endl;

        start = std::chrono::high_resolution_clock::now();

        if (multithreading) {
            std::vector<std::future<void>> texture_saves;

            for (int ix = min_x; ix <= max_x; ix++) {
                for (int iy = min_y; iy <= max_y; iy++) {
                    texture_saves.emplace_back(std::async(std::launch::async, [&tiles_read, &file_read, ix, iy]() {
                        auto raw = file_read.GetTexture(ix, iy);
                        if (raw.has_value()) {
                            tiles_read.push_back(Tile(ix, iy, Texture(tile_resolution, tile_resolution, raw.value())));
                        }
                    }));
                }
            }

            for (auto &save : texture_saves) {
                save.wait();
            }
        } else {
            for (int ix = min_x; ix <= max_x; ix++) {
                for (int iy = min_y; iy <= max_y; iy++) {
                    auto raw = file_read.GetTexture(ix, iy);
                    if (raw.has_value()) {
                        tiles_read.push_back(Tile(ix, iy, Texture(tile_resolution, tile_resolution, raw.value())));
                    }
                }
            }
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Loaded " << tiles_read.size() << " textures in : " << duration.count()
                  << "ms. (Multithreading: " << multithreading << ")" << std::endl;
    }

    REQUIRE(tiles_read.size() == tiles_write.size());

    for (size_t i = 0; i < tiles_read.size(); i++) {
        for (size_t i = 0; i < tiles_write.size(); i++) {
            if ((tiles_read[i].x == tiles_write[i].x) && (tiles_read[i].y == tiles_write[i].y)) {
                REQUIRE(tiles_read[i] == tiles_write[i]);
            }
        }
    }
}