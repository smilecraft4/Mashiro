#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <algorithm>
#include <catch.hpp>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <future>
#include <glad/glad.h>
#include <iostream>
#include <png.h>
#include <string>
#include <vector>

void static OpenglErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar *message, const void *userParam) {
    // glGetString()
    // TODO : convert this to a special logger
    std::cout << std::format("Opengl: {}, {}, {}, {}, {}", source, type, severity, id, message) << std::endl;
}

struct Texture {
    GLuint ID;
    GLuint width;
    GLuint height;
};

struct Raw {
    int height;
    int width;
    std::vector<std::uint32_t> pixels;

    bool operator==(const Raw &other) const {
        return (height == other.height) && (width == other.width) && (pixels == other.pixels);
    }
};

struct Png {
    std::string filename;
    int width;
    int height;
};

struct State {
    GLFWwindow *window;
};

static bool tile_init(State &state) {
    if (!glfwInit()) {
        printf_s("Failed to init GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    state.window = glfwCreateWindow(800, 600, "Test-tiles", nullptr, nullptr);
    if (!state.window) {
        printf_s("Failed to create window");
        return false;
    }

    glfwMakeContextCurrent(state.window);
    int version = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!version) {
        printf_s("Failed to load GLAD");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenglErrorCallback, nullptr);

    return true;
}

static bool tile_terminate(State &state) {
    glfwDestroyWindow(state.window);
    glfwTerminate();
    return true;
}

static Png save_raw(Raw &raw, const std::string &filename, int compression_level) {
    png_structp png_ptr;
    png_infop info_ptr;
    png_error_ptr user_error_ptr{};
    int transforms = PNG_TRANSFORM_IDENTITY;

    int width = raw.width;
    int height = raw.height;
    png_uint_32p buf = raw.pixels.data();

    std::vector<uint8_t *> rows(height, 0);
    for (size_t r = 0; r < rows.size(); r++) {
        const auto row_ptr = (uint8_t *)(raw.pixels.data() + width * r);
        rows[r] = reinterpret_cast<uint8_t *>(row_ptr);
    }

    FILE *fp = fopen(filename.c_str(), "wb");
    REQUIRE(fp);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr, nullptr, nullptr);
    REQUIRE(png_ptr);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        REQUIRE(nullptr);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        REQUIRE(nullptr);
    }

    png_init_io(png_ptr, fp);
    png_set_compression_level(png_ptr, compression_level);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png_ptr, info_ptr, rows.data());
    png_write_png(png_ptr, info_ptr, transforms, nullptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return {filename, width, height};
}

static Raw open_png(Png &png) {
    png_structp png_ptr;
    png_infop info_ptr, end_info;
    png_error_ptr user_error_ptr{};
    int transforms = PNG_TRANSFORM_IDENTITY;

    Raw raw{};
    raw.width = png.width;
    raw.height = png.height;
    raw.pixels.resize(size_t(raw.width) * raw.height);

    FILE *fp = fopen(png.filename.c_str(), "rb");
    REQUIRE(fp);

    std::uint8_t header[8];
    fread(header, 1, 8, fp);
    REQUIRE(!png_sig_cmp(header, 0, 8));

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)user_error_ptr, nullptr, nullptr);
    REQUIRE(png_ptr);

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        REQUIRE(nullptr);
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        REQUIRE(nullptr);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        REQUIRE(nullptr);
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    std::vector<uint8_t *> rows(raw.height, 0);
    for (size_t r = 0; r < rows.size(); r++) {
        rows[r] = reinterpret_cast<uint8_t *>(raw.pixels.data() + raw.width * r);
    }
    png_set_rows(png_ptr, info_ptr, rows.data());

    png_read_png(png_ptr, info_ptr, transforms, nullptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    fclose(fp);

    return raw;
}

static bool create_raw(int width, int height, std::vector<Raw> &raws) {
    for (auto &raw : raws) {
        raw.width = width;
        raw.height = height;
        raw.pixels.resize(size_t(width) * height);

        for (size_t x = 0; x < width; x++) {
            for (size_t y = 0; y < height; y++) {
                size_t i = x + (y * width);

                std::uint8_t r = (float)x / width * 255;
                std::uint8_t g = (float)y / width * 255;
                std::uint8_t b = i / raw.pixels.size();
                std::uint8_t a = rand() % 255;

                raw.pixels[i] = (a << 24) + (b << 16) + (g << 8) + r;
            }
        }
    }

    return true;
}

TEST_CASE("Png Test", "[Tile]") {
    State state{};

    REQUIRE(tile_init(state));

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::milliseconds duration;

    size_t count = 512;
    int compression_level = 6;
    int resolution = 512;

    // Create random raw texture
    start = std::chrono::high_resolution_clock::now();

    std::vector<Raw> random_raws(count);
    create_raw(resolution, resolution, random_raws);

    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Generated " << count << " random raw texture in : " << duration.count() << "ms" << std::endl;

    // Create opengl texture
    start = std::chrono::high_resolution_clock::now();

    std::vector<Texture> opengl_textures{};
    for (size_t i = 0; i < random_raws.size(); i++) {
        Texture texture{};
        texture.height = random_raws[i].height;
        texture.width = random_raws[i].width;

        glGenTextures(1, &texture.ID);

        glBindTexture(GL_TEXTURE_2D, texture.ID);
        std::string text = std::format("Texture {}", i).c_str();
        glObjectLabel(GL_TEXTURE, texture.ID, text.size(), text.c_str());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture.width, texture.height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE,
                        random_raws[i].pixels.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        opengl_textures.push_back(texture);
    }

    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Created " << count << " OpenGL from raw texture in  : " << duration.count() << "ms" << std::endl;

    // Get raw texture from opengl texture
    start = std::chrono::high_resolution_clock::now();

    std::vector<Raw> opengl_raws{};
    for (size_t i = 0; i < opengl_textures.size(); i++) {
        Raw raw{};
        raw.width = opengl_textures[i].width;
        raw.height = opengl_textures[i].height;
        raw.pixels.resize(raw.width * raw.height);
        glBindTexture(GL_TEXTURE_2D, opengl_textures[i].ID);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw.pixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);

        opengl_raws.push_back(raw);
    }

    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Read " << count << " OpenGL texture to raw texture in : " << duration.count() << "ms" << std::endl;

    for (size_t i = 0; i < random_raws.size(); i++) {
        REQUIRE(opengl_raws[i] == random_raws[i]);
    }

    std::vector<std::future<void>> futures;

    // Save/compress raw texture to png
    start = std::chrono::high_resolution_clock::now();
    std::vector<Png> pngs(opengl_raws.size());

    for (size_t i = 0; i < opengl_raws.size(); i++) {
        futures.emplace_back(std::async(std::launch::async, [i, &opengl_raws, &pngs, compression_level]() {
            auto filename = std::format("images/{}-{}_{}.png", opengl_raws[i].height, compression_level, i);
            pngs[i] = save_raw(opengl_raws[i], filename, compression_level);
        }));
    }

    for (auto &fut : futures) {
        fut.get();
    }

    futures.clear();

    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Saved " << count << " raw texture as png in : " << duration.count() << "ms" << std::endl;

    std::vector<Raw> png_raws(pngs.size());
    // Load/decompress png to raw texture
    start = std::chrono::high_resolution_clock::now();
    {
        for (size_t i = 0; i < pngs.size(); i++) {
            futures.emplace_back(std::async(
                std::launch::async, [i, &pngs, &png_raws, compression_level]() { png_raws[i] = open_png(pngs[i]); }));
        }
    }

    for (auto &fut : futures) {
        fut.get();
    }

    stop = std::chrono::high_resolution_clock::now();
    duration = duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Loaded " << count << " png as raw texture in : " << duration.count() << "ms" << std::endl;

    for (size_t i = 0; i < opengl_raws.size(); i++) {
        REQUIRE(png_raws[i] == opengl_raws[i]);
    }

    REQUIRE(tile_terminate(state));
}
