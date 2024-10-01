#pragma once

#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <map>
#include <string>
#include <vector>

struct Uniform {
    GLuint location;
    GLsizei length;
    GLsizei count;
    GLenum type;
};

class Program {
  public:
    Program();
    Program(std::vector<std::filesystem::path> filenames);
    ~Program();

    Program(const Program &) = delete;
    Program &operator=(const Program &) = delete;

    Program(Program &&other) noexcept;
    Program &operator=(Program &&other) noexcept;

    void Compile(std::vector<std::filesystem::path> filenames);

    void Bind() const;

    // template this one
    // void SetUniform();

  //protected:
    void Release() noexcept;

    GLuint _ID = 0;
    std::map<std::string, Uniform> _uniforms{};
    std::vector<std::filesystem::path> _filenames{};
};