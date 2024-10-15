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

TEST_CASE("Webp Test", "[Tile]") {
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

    //for (size_t i = 0; i < opengl_raws.size(); i++) {
    //    futures.emplace_back(std::async(std::launch::async, [i, &opengl_raws, &pngs, compression_level]() {
    //        auto filename = std::format("images/{}-{}_{}.png", opengl_raws[i].height, compression_level, i);
    //        pngs[i] = save_webp(opengl_raws[i], filename, compression_level);
    //    }));
    //}

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
        //for (size_t i = 0; i < pngs.size(); i++) {
        //    futures.emplace_back(std::async(
        //        std::launch::async, [i, &pngs, &png_raws, compression_level]() { png_raws[i] = open_webp(pngs[i]); }));
        //}
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
