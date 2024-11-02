#pragma once

HGLRC InitRenderer(HDC hdc);

class Uniformbuffer {
  public:
    Uniformbuffer(const Uniformbuffer &) = delete;
    Uniformbuffer &operator=(const Uniformbuffer &) = delete;
    Uniformbuffer(Uniformbuffer &&other);
    Uniformbuffer &operator=(Uniformbuffer &&other);

    Uniformbuffer(const std::wstring &name, GLuint binding);
    ~Uniformbuffer();

    static std::unique_ptr<Uniformbuffer> Create(const std::wstring &name, GLuint binding, GLsizei size, GLvoid *data);

    void SetData(GLintptr offset, GLsizei size, GLvoid *data);
    GLuint GetBinding() const;

    static GLuint GetBinding(const std::wstring &name);

  private:
    void Release();

    std::string _name;
    GLuint _binding;
    GLuint _ubo; // is this necessary

    static std::unordered_map<std::string, GLuint> _bindings;
};

class Texture {
  public:
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&other);
    Texture &operator=(Texture &&other);

    Texture(const std::wstring &name, GLsizei width, GLsizei height);
    ~Texture();

    static std::unique_ptr<Texture> Create(const std::wstring &name, int width, int height);
    void GenerateMipmaps();

    std::vector<uint32_t> ReadPixels() const;
    void SetPixels(std::span<uint32_t> pixels);

    void Bind(GLenum unit) const noexcept;

    GLuint ID() const noexcept;
    GLsizei Width() const noexcept;
    GLsizei Height() const noexcept;

  private:
    void Release();

    std::string _name;
    GLuint _ID;
    GLsizei _width;
    GLsizei _height;
};

class Mesh {
  public:
    using Element = std::uint32_t;
    struct Vertex {
        glm::vec2 pos;
        glm::vec2 tex;
        glm::vec2 nrml;
        glm::vec4 color;
    };

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&other);
    Mesh &operator=(Mesh &&other);

    Mesh();
    ~Mesh();

    static std::unique_ptr<Mesh> Create(const std::wstring &name);

    void Render(GLenum mode, GLsizei count);

  private:
    void Release();

    GLsizei _count;
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
};

class Program {
  public:
    struct UniformInfo {
        GLuint location;
    };

    Program(const Program &) = delete;
    Program &operator=(const Program &) = delete;
    Program(Program &&other);
    Program &operator=(Program &&other);

    Program();
    ~Program();

    static std::unique_ptr<Program> Create(const std::wstring &name);

    void AddShader(std::filesystem::path filename, GLenum type);
    void ClearShaderFilename();
    void Compile();

    void Bind();
    void Unbind();

    GLuint ID();

    void SetFloat(const std::string &name, float value);
    void SetInt(const std::string &name, std::int32_t value);
    void SetUint(const std::string &name, std::uint32_t value);
    void SetVec2(const std::string &name, glm::vec2 value);
    void SetVec3(const std::string &name, glm::vec3 value);
    void SetVec4(const std::string &name, glm::vec4 value);
    void SetMat4(const std::string &name, glm::mat4 &value);

  private:
    void Release();

    std::unordered_map<GLenum, std::filesystem::path> _filenames;
    std::unordered_map<std::string, UniformInfo> _uniforms;

    GLuint _ID;
};

class Framebuffer {
  public:
    Framebuffer(const Framebuffer &) = delete;
    Framebuffer &operator=(const Framebuffer &) = delete;
    Framebuffer(Framebuffer &&other);
    Framebuffer &operator=(Framebuffer &&other);

    Framebuffer();
    ~Framebuffer();

    static void Init();
    static std::unique_ptr<Framebuffer> Create(const std::wstring &name, int width, int height);

    void Resize(int width, int height);
    void Bind();
    void Unbind();
    void Render();

  private:
    void Release();

    int _width;
    int _height;
    GLuint _ID;
    std::string _name;
    std::unique_ptr<Texture> _texture;

    static std::unique_ptr<Mesh> _mesh;
    static std::unique_ptr<Program> _program;
};
