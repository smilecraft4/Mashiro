#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <catch.hpp>
#include <glad/glad.h>

#include <chrono>
#include <cstdint>
#include <format>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include <fstream>
#include <istream>
#include <streambuf>

#include <webp/decode.h>
#include <webp/encode.h>
#include <webp/types.h>

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

struct Webp {
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

TEST_CASE("Webp Test", "[Tile]") {
    State state{};

    REQUIRE(tile_init(state));

    size_t count = 1;
    int resolution = 64;

    std::vector<Raw> source(count);
    std::vector<Texture> opengl(count);
    std::vector<Raw> retrieved(count);
    std::vector<Webp> saved(count);
    std::vector<Raw> loaded(count);

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::milliseconds duration;

    // Create random raw texture
    {
        start = std::chrono::high_resolution_clock::now();

        for (auto &src : source) {
            src.width = resolution;
            src.height = resolution;
            src.pixels.resize(size_t(resolution) * resolution);

            for (size_t x = 0; x < resolution; x++) {
                for (size_t y = 0; y < resolution; y++) {
                    size_t i = x + (y * resolution);

                    std::uint8_t r = (float)x / resolution * 255;
                    std::uint8_t g = (float)y / resolution * 255;
                    std::uint8_t b = i / src.pixels.size();
                    std::uint8_t a = rand() % 255;

                    src.pixels[i] = (a << 24) + (b << 16) + (g << 8) + r;
                }
            }
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "webgl: Generated " << count << " random raw texture in : " << duration.count() << "ms" << std::endl;
    }

    // Create opengl texture
    {
        start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < source.size(); i++) {
            Texture texture{};
            texture.height = source[i].height;
            texture.width = source[i].width;

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
                            source[i].pixels.data());
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            opengl[i] = texture;
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "webgl: Created " << count << " OpenGL from raw texture in  : " << duration.count() << "ms" << std::endl;
    }

    // Get raw texture from opengl texture
    {
        start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < opengl.size(); i++) {
            Raw raw{};
            raw.width = opengl[i].width;
            raw.height = opengl[i].height;
            raw.pixels.resize(raw.width * raw.height);
            glBindTexture(GL_TEXTURE_2D, opengl[i].ID);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw.pixels.data());
            glBindTexture(GL_TEXTURE_2D, 0);

            retrieved[i] = raw;
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "webgl: Read " << count << " OpenGL texture to raw texture in : " << duration.count() << "ms"
                  << std::endl;
    }

    for (size_t i = 0; i < source.size(); i++) {
        REQUIRE(retrieved[i] == source[i]);
    }

    // Save/compress raw texture to png
    {
        std::vector<std::future<void>> futures;

        start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < retrieved.size(); i++) {
            futures.emplace_back(std::async(std::launch::async, [i, &retrieved, &saved]() {
                auto filename = std::format("./images/{}_{}.webp", retrieved[i].height, i);

                uint8_t *ptr{};
                size_t len = WebPEncodeLosslessRGBA(reinterpret_cast<const uint8_t *>(retrieved[i].pixels.data()),
                                                    retrieved[i].width, retrieved[i].height,
                                                    sizeof(uint32_t) * retrieved[i].width, &ptr);

                std::ofstream file(filename, std::ios_base::out | std::ios_base::binary | std::ios::app);
                file.write(reinterpret_cast<char *>(ptr), len);

                saved[i].filename = filename;
                saved[i].width = retrieved[i].width;
                saved[i].height = retrieved[i].height;
            }));
        }

        for (auto &fut : futures) {
            fut.get();
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "webgl: Saved " << count << " webgl texture as png in : " << duration.count() << "ms" << std::endl;
    }

    // Load/decompress png to raw texture
    {
        start = std::chrono::high_resolution_clock::now();
        std::vector<std::future<void>> futures;

        {
            for (size_t i = 0; i < saved.size(); i++) {
                futures.emplace_back(std::async(std::launch::async, [i, &saved, &loaded]() {
                    std::ifstream file(saved[i].filename, std::ios_base::in | std::ios_base::binary);
                    REQUIRE(file);

                    // read file size
                    file.seekg(0, std::ios::end);
                    size_t len = file.tellg();
                    file.seekg(0, std::ios::beg);

                    std::vector<uint8_t> buf(len, 0);
                    file.read(reinterpret_cast<char *>(buf.data()), len);

                    int width{}, height{};
                    REQUIRE(WebPGetInfo(buf.data(), buf.size(), &width, &height));

                    WebPBitstreamFeatures features{};
                    REQUIRE(!WebPGetFeatures(buf.data(), buf.size(), &features));
                    REQUIRE(features.format == 2);

                    loaded[i].height = features.height;
                    loaded[i].width = features.width;

                    const auto ptr = WebPDecodeRGBA(buf.data(), buf.size(), &loaded[i].width, &loaded[i].height);
                    loaded[i].pixels = std::vector<std::uint32_t>(reinterpret_cast<uint32_t*>(ptr), reinterpret_cast<uint32_t*>(ptr) + loaded[i].width * loaded[i].height);
                }));
            }
        }

        for (auto &fut : futures) {
            fut.get();
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Loaded " << count << "webgl: webgl as raw texture in : " << duration.count() << "ms" << std::endl;
    }

    for (size_t i = 0; i < retrieved.size(); i++) {
        REQUIRE(loaded[i].width == retrieved[i].width);
        REQUIRE(loaded[i].height == retrieved[i].height);

        REQUIRE(loaded[i].pixels.size() == retrieved[i].pixels.size());
        for (size_t j = 0; j < loaded[i].pixels.size(); j++) {
            REQUIRE(loaded[i].pixels[j] == retrieved[i].pixels[j]);
        }
    }

    REQUIRE(tile_terminate(state));
}
