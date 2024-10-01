#include "Program.h"

#include <spdlog/spdlog.h>

const std::map<std::string, GLenum> extension_map = {
    {".vert", GL_VERTEX_SHADER}, {".frag", GL_FRAGMENT_SHADER}, {".comp", GL_COMPUTE_SHADER}};

static GLenum GetShaderType(std::filesystem::path filename) {
    const auto extension = filename.extension().string();

    if (extension_map.contains(extension)) {
        return extension_map.at(extension);
    }

    return GL_FALSE;
}

static std::string LoadFile(std::filesystem::path filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename.string());
    }
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return contents;
}

static GLuint CompileShader(const std::string &shader_code, GLenum type) {
    const auto shader = glCreateShader(type);
    const GLchar *shader_source = shader_code.c_str();
    glShaderSource(shader, 1, &shader_source, 0);
    glCompileShader(shader);

    return shader;
}

Program::Program() {
}

Program::Program(std::vector<std::filesystem::path> filenames) {
    Compile(filenames);
}

Program::~Program() {
    Release();
}

Program::Program(Program &&other) noexcept {
    _ID = other._ID;
    _uniforms = other._uniforms;
    _filenames = other._filenames;

    other._ID = 0;
    other._uniforms = std::map<std::string, Uniform>{};
    other._filenames = std::vector<std::filesystem::path>{};
}

Program &Program::operator=(Program &&other) noexcept {
    if (this != &other) {
        Release();

        std::swap(_ID, other._ID);
        std::swap(_filenames, other._filenames);
        std::swap(_uniforms, other._uniforms);
    }

    return *this;
}

void Program::Compile(std::vector<std::filesystem::path> filenames) {
    Release();

    _filenames = filenames;
    std::vector<GLuint> compiled_shaders;

    for (size_t i = 0; i < filenames.size(); i++) {
        const auto shader_code = LoadFile(filenames[i]);
        const auto shader_type = GetShaderType(filenames[i]);

        if (!shader_type) {
            throw std::runtime_error("Wrong file extensions provided");
        }

        const auto shader = CompileShader(shader_code, shader_type);
        compiled_shaders.push_back(shader);
    }

    _ID = glCreateProgram();
    for (size_t i = 0; i < compiled_shaders.size(); i++) {
        glAttachShader(_ID, compiled_shaders[i]);
    }

    glLinkProgram(_ID);
    GLint is_linked = 0;
    glGetProgramiv(_ID, GL_LINK_STATUS, (int *)&is_linked);
    if (is_linked == GL_FALSE) {
        GLint max_length = 0;
        glGetProgramiv(_ID, GL_INFO_LOG_LENGTH, &max_length);

        // The max_length includes the NULL character
        std::string info_log("", max_length);
        glGetProgramInfoLog(_ID, max_length, &max_length, &info_log[0]);
        throw std::runtime_error(info_log);
    }
    for (size_t i = 0; i < compiled_shaders.size(); i++) {
        glDeleteShader(compiled_shaders[i]);
    }

    GLint uniform_count{0};
    glGetProgramiv(_ID, GL_ACTIVE_UNIFORMS, &uniform_count);
    if (uniform_count) {
        GLint max_name_len;
        GLint count;
        GLenum type;
        GLint length;

        glGetProgramiv(_ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);
        auto uniform_name = std::make_unique<char[]>(max_name_len);

        for (GLint i = 0; i < uniform_count; i++) {
            glGetActiveUniform(_ID, i, max_name_len, &length, &count, &type, uniform_name.get());
            Uniform uniform{};
            uniform.location = glGetUniformLocation(_ID, uniform_name.get());
            uniform.length = length;
            uniform.count = count;
            uniform.type = type;

            _uniforms.emplace(std::string(uniform_name.get(), uniform.length), uniform);
        }
    }
}

void Program::Release() noexcept {
    glDeleteProgram(_ID);
    _ID = 0;
}

void Program::Bind() const {
    glUseProgram(_ID);
}
