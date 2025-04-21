#pragma once

#include <ituGL/renderer/Renderer.h>

#include <ituGL/application/Application.h>
#include <ituGL/scene/Scene.h>
#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/Mesh.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>

#include <glm/mat4x4.hpp>
#include <vector>

class Texture2DObject;

class MapApplication : public Application
{
public:
    MapApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeTextures();
    void InitializeMaterials();
    void InitializeMeshes();
    void InitializeModels();
    void InitializeCamera();
    void InitializeRenderer();

    void RenderGui();

    std::shared_ptr<Texture2DObject> CreateDefaultTexture();
    std::shared_ptr<Texture2DObject> CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords);
    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);

    void CreateTerrainMesh(unsigned int gridX, unsigned int gridY);

private:
    const int TERRAIN_MESH_COUNT = 4;

    unsigned int m_gridX, m_gridY, m_gridWidth, m_gridHeight;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    Renderer m_renderer;
    Scene m_scene;

    DearImGui m_imGui;

    // Camera controller
    CameraController m_cameraController;

    std::shared_ptr<Mesh> m_terrainPatch;
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