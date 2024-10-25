#include "Renderer.h"
#include "Framework.h"
#include "Log.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <unordered_map>

Program::Program(Program &&other) : _filenames(other._filenames), _uniforms(other._uniforms), _ID(other._ID) {
    std::swap(*this, other);

    other._filenames.clear();
    other._uniforms.clear();
    other._ID = 0;
}

Program &Program::operator=(Program &&other) {
    if (this != &other) {
        Release();
        std::swap(*this, other);
    }

    return *this;
}

Program::Program() : _ID(0) {
}

Program::~Program() {
    Release();
}

std::unique_ptr<Program> Program::Create(const tstring &name) {
    auto program = std::make_unique<Program>();
    return program;
}

void Program::AddShader(std::filesystem::path filename, GLenum type) {
    if (!_filenames.contains(type)) {
        _filenames.emplace(type, filename);
    } else {
        _filenames[type] = filename;
    }
}

void Program::ClearShaderFilename() {
    _filenames.clear();
}

void Program::Bind() {
    glUseProgram(_ID);
}

void Program::Unbind() {
    glUseProgram(0);
}

GLuint Program::ID() {
    return _ID;
}

void Program::SetFloat(const std::string &name, float value) {
    if (_uniforms.contains(name)) {
        glUniform1f(_uniforms[name].location, value);
    }
}

void Program::SetInt(const std::string &name, std::int32_t value) {
    if (_uniforms.contains(name)) {
        glUniform1i(_uniforms[name].location, value);
    }
}

void Program::SetUint(const std::string &name, std::uint32_t value) {
    if (_uniforms.contains(name)) {
        glUniform1ui(_uniforms[name].location, value);
    }
}

void Program::SetVec2(const std::string &name, glm::vec2 value) {
    if (_uniforms.contains(name)) {
        glUniform2fv(_uniforms[name].location, 1, glm::value_ptr(value));
    }
}

void Program::SetVec3(const std::string &name, glm::vec3 value) {
    if (_uniforms.contains(name)) {
        glUniform3fv(_uniforms[name].location, 1, glm::value_ptr(value));
    }
}

void Program::SetVec4(const std::string &name, glm::vec4 value) {
    if (_uniforms.contains(name)) {
        glUniform4fv(_uniforms[name].location, 1, glm::value_ptr(value));
    }
}

void Program::SetMat4(const std::string &name, glm::mat4 &value) {
    if (_uniforms.contains(name)) {
        glUniformMatrix4fv(_uniforms[name].location, 1, GL_FALSE, glm::value_ptr(value));
    }
}

void Program::Compile() {
    _ID = glCreateProgram();

    std::vector<GLuint> shaders;
    for (const auto &[type, filename] : _filenames) {
        // Read file
        GLint filesize = std::filesystem::file_size(filename);
        char *shader_source = new char[filesize];
        std::ifstream file(filename, std::ios::binary);
        file.read(shader_source, filesize);
        if (!file) {
            delete[] shader_source;
            throw std::runtime_error(std::format("Failed to read file {}", filename.string()));
        }
        file.close();

        // Create shader
        const GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &shader_source, &filesize);
        glCompileShader(shader);

        delete[] shader_source;

        // Save shader
        glAttachShader(_ID, shader);
        shaders.push_back(shader);
    }

    // Create program
    glLinkProgram(_ID);
    GLint success{};
    glGetProgramiv(_ID, GL_LINK_STATUS, &success);
    if (!success) {
        GLint maxLength = 0;
        glGetProgramiv(_ID, GL_INFO_LOG_LENGTH, &maxLength);

        std::string infoLog(maxLength, ' ');
        glGetProgramInfoLog(_ID, maxLength, &maxLength, infoLog.data());

        glDeleteProgram(_ID);
        _ID = 0;

        Log::Info(ConvertString(infoLog));
        throw std::runtime_error(infoLog);
    }
    // Clean shaders
    for (const auto &shader : shaders) {
        glDeleteShader(shader);
    }

    // Fetch uniform
    GLint uniform_count = 0;
    glGetProgramiv(_ID, GL_ACTIVE_UNIFORMS, &uniform_count);

    if (uniform_count != 0) {
        GLint max_name_len = 0;
        GLsizei length = 0;
        GLsizei count = 0;
        GLenum type = GL_NONE;
        glGetProgramiv(_ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);

        auto uniform_name = std::make_unique<char[]>(max_name_len);

        for (GLint i = 0; i < uniform_count; ++i) {
            glGetActiveUniform(_ID, i, max_name_len, &length, &count, &type, uniform_name.get());

            UniformInfo uniform_info = {};
            uniform_info.location = glGetUniformLocation(_ID, uniform_name.get());

            _uniforms.emplace(std::make_pair(std::string(uniform_name.get(), length), uniform_info));
        }
    }
}

void Program::Release() {
    glDeleteTextures(1, &_ID);
    _ID = 0;
}

std::unique_ptr<Mesh> Mesh::Create(const tstring &name) {
    auto mesh = std::make_unique<Mesh>();
    return mesh;
}

void Mesh::Render(GLenum mode, GLsizei count) {
    glBindVertexArray(_vao);
    if (_ebo) {
        glDrawElements(mode, _count, GL_UNSIGNED_BYTE, nullptr);
    } else {
        glDrawArrays(mode, 0, count);
    }
}

void Mesh::Release() {
    glDeleteVertexArrays(1, &_vao);
    _vao = 0;

    if (_vbo) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_ebo) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }
}

Mesh::Mesh(Mesh &&other) : _vao(other._vao), _vbo(other._vbo), _ebo(other._ebo), _count(other._count) {
    std::swap(*this, other);

    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
    other._count = 0;
}

Mesh &Mesh::operator=(Mesh &&other) {
    if (this != &other) {
        Release();
        std::swap(*this, other);
    }

    return *this;
}

Mesh::Mesh() {
    _count = 0;
    _ebo = 0;
    _vbo = 0;
    glCreateVertexArrays(1, &_vao);
}

Mesh::~Mesh() {
    Release();
}

std::unordered_map<std::string, GLuint> Uniformbuffer::_bindings = std::unordered_map<std::string, GLuint>();

Uniformbuffer::Uniformbuffer(Uniformbuffer &&other) : _name(other._name), _binding(other._binding), _ubo(other._ubo) {
    other._name.clear();
    other._binding = 0;
    other._ubo = 0;
}

Uniformbuffer &Uniformbuffer::operator=(Uniformbuffer &&other) {
    if (this != &other) {
        Release();
        std::swap(*this, other);
    }

    return *this;
}

Uniformbuffer::Uniformbuffer(const tstring &name, GLuint binding) {
    _name = RestoreStringA(name);
    _binding = binding;
    _ubo = 0;
}

Uniformbuffer::~Uniformbuffer() {
    Release();
}

// Todo is this really good ??
std::unique_ptr<Uniformbuffer> Uniformbuffer::Create(const tstring &name, GLuint binding, GLsizei size, GLvoid *data) {
    auto buffer = std::make_unique<Uniformbuffer>(name, binding);

    glGenBuffers(1, &buffer->_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->_ubo);
    glObjectLabel(GL_BUFFER, buffer->_ubo, buffer->_name.size(), buffer->_name.c_str());
    glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, buffer->_binding, buffer->_ubo, 0, size);

    _bindings.emplace(RestoreStringA(name), binding);

    return buffer;
}

void Uniformbuffer::SetData(GLintptr offset, GLsizei size, GLvoid *data) {
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

GLuint Uniformbuffer::GetBinding() const {
    return _binding;
}

GLuint Uniformbuffer::GetBinding(const tstring &name) {
    return _bindings[RestoreStringA(name)];
}

void Uniformbuffer::Release() {
    glDeleteBuffers(1, &_ubo);
}

Texture::Texture(Texture &&other) : _name(other._name), _ID(other._ID), _width(other._width), _height(other._height) {
    other._name.clear();
    other._ID = 0;
    other._width = 0;
    other._height = 0;
}

Texture &Texture::operator=(Texture &&other) {
    if (this != &other) {
        Release();
        std::swap(*this, other);
    }

    return *this;
}

Texture::Texture(const tstring &name, GLsizei width, GLsizei height) {
    _name = RestoreStringA(name);
    _width = width;
    _height = height;

    glGenTextures(1, &_ID);
    glBindTexture(GL_TEXTURE_2D, _ID);
    glObjectLabel(GL_TEXTURE, _ID, _name.size(), _name.c_str());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, _width, _height);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    Release();
}

std::unique_ptr<Texture> Texture::Create(const tstring &name, int width, int height) {
    auto texture = std::make_unique<Texture>(name, width, height);
    return texture;
}

void Texture::GenerateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, _ID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind(GLenum unit) const noexcept {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _ID);
}

GLuint Texture::ID() const noexcept {
    return _ID;
}

GLsizei Texture::Width() const noexcept {
    return _width;
}

GLsizei Texture::Height() const noexcept {
    return _height;
}

std::vector<uint32_t> Texture::ReadPixels() const {
    auto pixels = std::vector<std::uint32_t>(_width * _height);
    glBindTexture(GL_TEXTURE_2D, _ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    return pixels;
}

void Texture::SetPixels(std::span<uint32_t> pixels) {
    if (pixels.size() != _width * _height) {
        throw std::runtime_error("The supplied pixels are of the wrong size");
    }

    glBindTexture(GL_TEXTURE_2D, _ID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Release() {
    glDeleteTextures(1, &_ID);
    _ID = 0;
    _width = 0;
    _height = 0;
}

std::unique_ptr<Mesh> Framebuffer::_mesh;
std::unique_ptr<Program> Framebuffer::_program;

Framebuffer::Framebuffer(Framebuffer &&other)
    : _ID(other._ID), _width(other._width), _height(other._height), _name(other._name) {
    other._ID = 0;
    other._width = 0;
    other._height = 0;
    other._name.clear();
    _texture = std::move(other._texture);
    other._texture = nullptr;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other) {
    if (this != &other) {
        Release();
        std::swap(*this, other);
    }

    return *this;
}

void Framebuffer::Init() {
    _mesh = Mesh::Create(TEXT("Framebuffer Mesh"));
    _program = Program::Create(TEXT("Framebuffer Program"));
    _program->AddShader("data/screen.vert", GL_VERTEX_SHADER);
    _program->AddShader("data/screen.frag", GL_FRAGMENT_SHADER);
    _program->Compile();
}

std::unique_ptr<Framebuffer> Framebuffer::Create(const tstring &name, int width, int height) {
    auto framebuffer = std::make_unique<Framebuffer>();

    framebuffer->_name = RestoreStringA(name);
    framebuffer->_width = width;
    framebuffer->_height = height;

    glGenFramebuffers(1, &framebuffer->_ID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->_ID);
    glObjectLabel(GL_FRAMEBUFFER, framebuffer->_ID, framebuffer->_name.size(), framebuffer->_name.c_str());

    framebuffer->_texture = Texture::Create(TEXT("Framebuffer Texture"), width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->_texture->ID(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Failed to create framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return framebuffer;
}

void Framebuffer::Resize(int width, int height) {
    _texture.release();

    glBindFramebuffer(GL_FRAMEBUFFER, _ID);
    _texture = Texture::Create(TEXT("Framebuffer Texture"), width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->ID(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Failed to create framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, _ID);
}

void Framebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Render() {
    _program->Bind();
    _texture->Bind(0);
    _mesh->Render(GL_TRIANGLES, 3);
}

Framebuffer::Framebuffer() {
    _width  = 0;
    _height = 0;
    _name = "";
    _ID = 0;
    _texture = nullptr;
}

Framebuffer::~Framebuffer() {
    Release();
}

void Framebuffer::Release() {
    _texture.release();
    glDeleteFramebuffers(1, &_ID);
    _ID = 0;
    _width = 0;
    _height = 0;
}
