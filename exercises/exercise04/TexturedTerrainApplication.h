#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/Mesh.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <glm/mat4x4.hpp>
#include <vector>

class Texture2DObject;

class TexturedTerrainApplication : public Application
{
public:
    TexturedTerrainApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;

private:
    void InitializeTextures();
    void InitializeMeshes();
    void InitializeMaterials();

    void DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix);

    std::shared_ptr<Texture2DObject> CreateDefaultTexture();
    std::shared_ptr<Texture2DObject> CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords);
    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);

    void CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY);

private:
    const int TERRAIN_MESH_COUNT = 4;

    unsigned int m_gridX, m_gridY;

    Camera m_camera;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    Mesh m_terrainPatch;
    std::shared_ptr<Material> m_defaultMaterial;
    std::vector<std::shared_ptr<Material>> m_terrainMaterials;
    std::shared_ptr<Material> m_waterMaterial;

    std::shared_ptr<Texture2DObject> m_defaultTexture;
    std::shared_ptr<Texture2DObject> m_dirtTexture;
    std::shared_ptr<Texture2DObject> m_grassTexture;
    std::shared_ptr<Texture2DObject> m_rockTexture;
    std::shared_ptr<Texture2DObject> m_snowTexture;
    std::shared_ptr<Texture2DObject> m_waterTexture;

    std::vector<std::shared_ptr<Texture2DObject>> m_heightMaps;
};