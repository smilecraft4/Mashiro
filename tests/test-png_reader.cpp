#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch.hpp>

#include <istream>
#include <span>
#include <vector>

#include <png.h>

#include <chrono>
#include <future>
#include <iostream>

class Raw {
  public:
    Raw(int width, int height);

    bool Read(std::vector<uint8_t> data);
    std::vector<uint8_t> Write(int compression = 9);

    bool operator==(const Raw &other) const {
        return (_width == other._width) && (_height == other._height) && (_pixels == other._pixels);
    }

    std::vector<uint32_t> _pixels;
  private:
    static void _png_read_from_memory(png_structp pngPtr, png_bytep data, png_size_t length);
    static void _png_write_to_memory(png_structp pngPtr, png_bytep data, png_size_t length);

    struct Context {
        std::vector<uint8_t> ptr;
        size_t offset;
    };

    int _width;
    int _height;
};

TEST_CASE("Write a raw texture to memory as a png") {
    Raw bar = Raw(256,256);
    Raw foo = bar;
    std::vector<uint8_t> data;

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::milliseconds duration;

    size_t count = 64;

    start = std::chrono::high_resolution_clock::now();
    size_t size = 0;
    size_t original = 0;
    for (size_t i = 0; i < count; i++) {
        data = foo.Write(5);
    }

    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Wrote " << count << " in : " << duration.count() << "ms" << std::endl;
    std::cout << "The size was : " << size / count << "/" << original / count << " bytes\n";

    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < count; i++) {
        foo.Read(data);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Read " << count << " in : " << duration.count() << "ms" << std::endl;
}

bool Raw::Read(std::vector<uint8_t> data) {
    _pixels.clear();

    uint8_t *ptr = data.data();
    size_t len = data.size();

    Context ctx{};
    ctx.ptr = data;
    ctx.offset = 0;

    constexpr int signature_len = 8;

    auto is_png = !png_sig_cmp(ptr, 0, signature_len);
    if (!is_png) {
        return false;
    }

    png_structp png_ptr{};
    png_infop info_ptr{}, end_info{};

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        return false;

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return false;
    }

    png_set_read_fn(png_ptr, reinterpret_cast<void *>(&ctx), _png_read_from_memory);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    int width = png_get_image_height(png_ptr, info_ptr);
    int height = png_get_image_width(png_ptr, info_ptr);

    _pixels.resize(width * height);

    const auto rows_ptr = png_get_rows(png_ptr, info_ptr);
    uint8_t *p_ptr = reinterpret_cast<uint8_t *>(_pixels.data());
    for (size_t r = 0; r < height; r++) {
        memcpy_s(p_ptr + sizeof(uint32_t) * width * r, sizeof(uint32_t) * width, rows_ptr[r], sizeof(4) * width);
    }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    return true;
}

std::vector<uint8_t> Raw::Write(int compression) {
    std::vector<uint8_t> data;

    uint8_t *ptr = reinterpret_cast<uint8_t *>(_pixels.data());
    size_t len = _pixels.size() * sizeof(uint32_t);

    png_structp png_ptr{};
    png_infop info_ptr{};

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        return data;

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return data;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return data;
    }

    png_set_write_fn(png_ptr, reinterpret_cast<void *>(&data), _png_write_to_memory, nullptr);

    std::vector<uint8_t *> rows(_height);
    for (size_t h = 0; h < _height; h++) {
        rows[h] = ptr + sizeof(uint32_t) * _width * h;
    }

    png_set_compression_level(png_ptr, compression);
    png_set_IHDR(png_ptr, info_ptr, _width, _height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png_ptr, info_ptr, rows.data());

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return data;
}

void Raw::_png_read_from_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto ctx = reinterpret_cast<Context *>(png_get_io_ptr(png_ptr));
    memcpy_s(data, length, ctx->ptr.data() + ctx->offset, length);
    ctx->offset += length;
}

void Raw::_png_write_to_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto *vec = reinterpret_cast<std::vector<uint8_t> *>(png_get_io_ptr(png_ptr));
    vec->insert(vec->end(), data, data + length);
}

Raw::Raw(int width, int height) : _width(width), _height(height), _pixels(width * height, 0) {
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