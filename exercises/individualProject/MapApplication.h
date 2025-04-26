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
class TextureCubemapObject;

class MapApplication : public Application
{
public:
    MapApplication();

protected:
    void Initialize() override;
    void InitializeLights();
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

    template <typename T>
    void UpdateMaterialsUniform(const char* uniformName, T value)
    {
        MapApplication::UpdateTerrainMaterialsUniform(uniformName, value);
        MapApplication::UpdateWaterMaterialUniform(uniformName, value);
    }

    template <typename T>
    void UpdateTerrainMaterialsUniform(const char* uniformName, T value)
    {
        for (std::shared_ptr<Material> terrainUniform : m_terrainMaterials)
        {
            ShaderProgram::Location location = terrainUniform->GetUniformLocation(uniformName);
            if (location >= 0)
                terrainUniform->SetUniformValue(location, value);
        }
        ShaderProgram::Location location = m_waterMaterial->GetUniformLocation(uniformName);
        if (location >= 0)
            m_waterMaterial->SetUniformValue(location, value);
    }

    template <typename T>
    void UpdateWaterMaterialUniform(const char* uniformName, T value)
    {
        for (std::shared_ptr<Material> terrainUniform : m_terrainMaterials)
        {
            ShaderProgram::Location location = terrainUniform->GetUniformLocation(uniformName);
            if (location >= 0)
                terrainUniform->SetUniformValue(location, value);
        }
        ShaderProgram::Location location = m_waterMaterial->GetUniformLocation(uniformName);
        if (location >= 0)
            m_waterMaterial->SetUniformValue(location, value);
    }

    void CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords);
    std::shared_ptr<Texture2DObject> CreateDefaultTexture();
    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);

    void CreateTerrainMesh(unsigned int gridX, unsigned int gridY);

private:
    const int TERRAIN_MESH_COUNT = 4;

    unsigned int m_gridX, m_gridY, m_gridWidth, m_gridHeight;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    Renderer m_renderer;
    Scene m_scene;

    // Needed for transparent water
    Scene m_waterScene;

    DearImGui m_imGui;

    // Camera controller
    CameraController m_cameraController;

    glm::vec3 m_ambientColor;

    float m_heightScale, m_smoothingAmount;
    float m_waterLevel;
    int m_levels;
    bool m_quantizeTerrain;

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
    std::vector<std::shared_ptr<Texture2DObject>> m_normalMaps;

    std::shared_ptr<TextureCubemapObject> m_skyboxTexture;
};