#pragma once
#include "AABB.h"
#include "Brush.h"
#include "File.h"

class Viewport;

class Canvas {
  public:
    static void Init();

    Canvas(const Canvas &) = delete;
    Canvas(Canvas &&) = delete;
    Canvas &operator=(const Canvas &) = delete;
    Canvas &operator=(Canvas &&) = delete;
    Canvas();
    ~Canvas();

    /* Canvas */

    static std::unique_ptr<Canvas> New();
    void Load(glm::ivec2 coord, File *file);
    static std::unique_ptr<Canvas> Open(File *file);

    bool IsSaved() const;
    void Save(File *file);
    void LazyLoad(glm::vec2 cursor, File *file);
    void LazySave(glm::vec2 cursor, File *file);

    void SaveTile(size_t i, File *file);

    void Refresh();
    void Paint(Brush *brush);

    void Render(Viewport *viewport);

    /* Layers */
    // std::wstring AddLayer(const std::wstring& name, int order, int mode);
    // void DeleteLayer(const std::wstring& id);
    // void SetLayerName(const std::wstring& id, const std::wstring& name);
    // std::wstring GetLayerName(const std::wstring& id);
    // void SelectLayer(const std::wstring& id);
    // void DeselectLayer(const std::wstring& id);
    // void MergeLayers(std::span<std::wstring> ids);
    // void SetSelectedLayers(std::span<std::wstring> ids);
    // std::vector<std::wstring> GetSelectedLayers();
    // void SetLayerMode(const std::wstring& id, int mode);
    // int GetLayerMode(const std::wstring& id);
    // void SetLayerOrder(const std::wstring& id, int order);
    // int GetLayerOrder(const std::wstring& id);
    // void SetLayerVisibility(const std::wstring& id, bool visible);
    // bool GetLayerVisibility(const std::wstring& id);

    /* Tiles */

    struct Tile {
        glm::ivec2 coord;
        std::uint32_t layer;
        std::uint32_t size;
    };

  private:
    void CreateTile(glm::ivec2 coord);
    void DeleteTile(glm::ivec2 coord);
    void ReloadTile(glm::ivec2 coord);
    void RenderTiles();
    void CullTiles(Viewport *viewport);

    std::map<std::pair<int, int>, size_t> _coord_tile;
    std::vector<Tile> _tiles_data;
    std::vector<AABB> _tiles_aabb;
    std::vector<bool> _tiles_visibility;
    std::vector<bool> _tiles_processing;
    std::vector<bool> _tiles_saved;
    std::vector<Texture> _tiles_textures;

    bool _saved;

    static std::unique_ptr<Uniformbuffer> _tile_ubo;
    static std::unique_ptr<Program> _program;
    static std::unique_ptr<Mesh> _mesh;
    static std::vector<uint32_t> _pixels;
};
