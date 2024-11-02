#include "pch.h"

#include "App.h"
#include "File.h"
#include "Log.h"
#include "Preferences.h"

struct Context {
    std::span<uint8_t> ptr;
    size_t offset;
};

static void _png_read_from_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto ctx = reinterpret_cast<Context *>(png_get_io_ptr(png_ptr));
    memcpy_s(data, length, ctx->ptr.data() + ctx->offset, length);
    ctx->offset += length;
}

static void _png_write_to_memory(png_structp png_ptr, png_bytep data, png_size_t length) {
    auto *vec = reinterpret_cast<std::vector<uint8_t> *>(png_get_io_ptr(png_ptr));
    vec->insert(vec->end(), data, data + length);
}

static std::vector<uint32_t> Read(std::span<uint8_t> data) {
    std::vector<uint32_t> pixels;

    uint8_t *ptr = data.data();
    size_t len = data.size();

    Context ctx{};
    ctx.ptr = data;
    ctx.offset = 0;

    constexpr int signature_len = 8;

    auto is_png = !png_sig_cmp(ptr, 0, signature_len);
    if (!is_png) {
        return pixels;
    }

    png_structp png_ptr{};
    png_infop info_ptr{}, end_info{};

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr)
        return pixels;

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return pixels;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return pixels;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return pixels;
    }

    png_set_read_fn(png_ptr, reinterpret_cast<void *>(&ctx), _png_read_from_memory);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    size_t width = png_get_image_height(png_ptr, info_ptr);
    size_t height = png_get_image_width(png_ptr, info_ptr);

    pixels.resize(width * height);

    const auto rows_ptr = png_get_rows(png_ptr, info_ptr);
    uint8_t *p_ptr = reinterpret_cast<uint8_t *>(pixels.data());
    for (size_t r = 0; r < height; r++) {
        memcpy_s(p_ptr + sizeof(uint32_t) * width * r, sizeof(uint32_t) * width, rows_ptr[r], sizeof(4) * width);
    }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    return pixels;
}

static std::vector<uint8_t> Write(int compression, int width, int height, std::vector<uint32_t> pixels) {
    std::vector<uint8_t> data;

    uint8_t *ptr = reinterpret_cast<uint8_t *>(pixels.data());
    size_t len = pixels.size() * sizeof(uint32_t);

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

    std::vector<uint8_t *> rows(height);
    for (size_t h = 0; h < height; h++) {
        rows[h] = ptr + sizeof(uint32_t) * width * h;
    }

    png_set_compression_level(png_ptr, compression);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_rows(png_ptr, info_ptr, rows.data());

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return data;
}

File::File() : _textures_indexes(), _pngs() {
    const auto tile_resolution = Preferences::Get()->_tile_resolution;

    _saved = false;
    _new = true;

    strcpy_s(_info._type, "msh");
    _info._version[0] = 0;
    _info._version[1] = 0;
    _info._version[2] = 2;
    _info._version[3] = 0;
    _info._resolution = tile_resolution;

    _info._size = 0;
    _info._header_count = 0;
    _filename = std::filesystem::path();
}

File::~File() {
}

std::unique_ptr<File> File::New(std::filesystem::path filename) {
    auto file = std::make_unique<File>();

    file->Rename(filename);
    file->_saved = true;
    file->_new = true;

    return file;
}

void File::Rename(std::filesystem::path filename) {
    _filename = filename;
}

std::filesystem::path File::GetFilename() const {
    return _filename;
}

bool File::IsSaved() const {
    return _saved;
}

bool File::IsNew() const {
    return _new;
}

std::unique_ptr<File> File::Open(std::filesystem::path filename) {
    auto file = std::make_unique<File>();

    file->Rename(filename);
    file->_saved = true;
    file->_new = false;

    // Load byte stream
    std::filebuf fp;
    fp.open(filename, std::ios_base::in | std::ios_base::binary);
    if (!fp.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::streampos pos{};
    pos = fp.sgetn(reinterpret_cast<char *>(&file->_info), sizeof(Info));

    // make sure the file type is correct (_type == "msh")
    if (strcmp(file->_info._type, "msh") != 0) {
        LOG_INFO(TEXT("Wrong file format"));
        throw std::runtime_error("Wrong file format");
    }

    // make sure the version is compatible

    // uncompress the header
    std::vector<TileHeader> _headers(file->_info._header_count);
    file->_pngs.resize(file->_info._header_count);

    // for every entry in the header load the tile as compressed from the data offset and length
    pos = fp.sgetn(reinterpret_cast<char *>(_headers.data()), sizeof(TileHeader) * _headers.size());

    for (size_t i = 0; i < _headers.size(); i++) {
        if ((size_t)pos != _headers[i].start) {
            pos = fp.pubseekpos(_headers[i].start);
        }
        file->_pngs[i].resize(_headers[i].len, 0);
        pos = fp.sgetn(reinterpret_cast<char *>(file->_pngs[i].data()), file->_pngs[i].size());
        file->_textures_indexes.emplace(std::pair<int, int>(_headers[i].coord[0], _headers[i].coord[1]), i);
    }

    fp.close();

    return file;
}

void File::Save(std::filesystem::path filename) {
    std::filebuf file;
    file.open(filename, std::ios_base::out | std::ios_base::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::streampos pos{};

    std::vector<TileHeader> tile_headers(_pngs.size());
    _info._header_count = _pngs.size();

    std::streampos header_pos = pos;
    // reserve sizeof(TileHeader) * _info._header_count to save the header later on
    pos = file.pubseekoff(sizeof(Info) + (sizeof(TileHeader) * _info._header_count), std::ios::beg);

    // write the data and saved their coords in the tile_headers
    for (const auto &[coord, index] : _textures_indexes) {
        tile_headers[index].coord[0] = coord.first;
        tile_headers[index].coord[1] = coord.second;
        tile_headers[index].start = file.pubseekoff(0, std::ios::cur);
        tile_headers[index].len = _pngs[index].size();
        pos = file.sputn(reinterpret_cast<char *>(_pngs[index].data()), _pngs[index].size());
        file.pubsync();
    }

    _info._size = 0;

    // resume infos
    pos = file.pubseekpos(0);
    pos = file.sputn(reinterpret_cast<char *>(&_info), sizeof(_info));
    pos = file.sputn(reinterpret_cast<char *>(tile_headers.data()), sizeof(TileHeader) * tile_headers.size());

    file.close();

    _saved = true;
    _new = false;
}

std::wstring File::GetDisplayName() {
    if (!_saved || !App::Get()->_canvas->IsSaved()) {
        return std::format(TEXT("*{}"), _filename.filename().wstring());
    }
    // FIXME: This and all the other call using .wstring() will break ansi compatibility
    return _filename.filename().wstring();
}

std::vector<std::pair<int, int>> File::GetSavedTileLocation() const {
    std::vector<std::pair<int, int>> tiles_saved;
    tiles_saved.reserve(_textures_indexes.size());

    for (const auto &[key, val] : _textures_indexes) {
        tiles_saved.push_back(key);
    }

    return tiles_saved;
}

int File::GetTileResolution() const {
    return _info._resolution;
}

bool File::HasTile(int x, int y) const {
    return _textures_indexes.contains({x, y});
}

std::vector<uint32_t> File::ReadTileTexture(int x, int y) {
    if (!_textures_indexes.contains({x, y})) {
        throw std::runtime_error("This file does not have this tile texture");
    }

    size_t png_index = _textures_indexes[{x, y}];
    std::vector<uint32_t> texture = Read(_pngs[png_index]);

    if (texture.size() <= 0) {
        LOG_INFO(std::format(TEXT("Failed to get saved texture at coord {},{}"), x, y));
        throw std::runtime_error(std::format("Failed to get saved texture at coord {},{}", x, y));
    }

    return texture;
}

void File::WriteTileTexture(int x, int y, std::vector<uint32_t> pixels, int compression) {
    size_t png_index;
    if (!_textures_indexes.contains({x, y})) {
        png_index = _pngs.size();
        _pngs.push_back({});
        _textures_indexes.emplace(std::pair<int, int>(x, y), png_index);
        LOG_INFO(std::format(TEXT("[FILE]: Added new Tile_{}_{}"), x, y));
    } else {
        png_index = _textures_indexes[{x, y}];
    }

    _pngs[png_index] = Write(compression, _info._resolution, _info._resolution, pixels);
    LOG_INFO(std::format(TEXT("[FILE]: Saved Tile_{}_{}: {}/{}b"), x, y, _pngs[png_index].size(),
                         pixels.size() * sizeof(uint32_t)));
}