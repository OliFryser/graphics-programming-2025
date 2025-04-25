#include "MapApplication.h"

#include <ituGL/asset/TextureCubemapLoader.h>
#include <ituGL/texture/Texture2DObject.h>

#include <ituGL/renderer/ForwardRenderPass.h>
#include <ituGL/renderer/SkyboxRenderPass.h>

#include <ituGL/scene/Transform.h>
#include <ituGL/scene/SceneCamera.h>
#include <ituGL/scene/SceneModel.h>
#include <ituGL/scene/SceneLight.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <ituGL/scene/ImGuiSceneVisitor.h>

#include <ituGL/lighting/DirectionalLight.h>

#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/geometry/Model.h>

#include <glm/gtx/transform.hpp>  // for matrix transformations

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <iostream>
#include <numbers>  // for PI constant
#include <imgui.h>
#include <format>

MapApplication::MapApplication()
    : Application(1024, 1024, "Individual Project")
    , m_gridX(128), m_gridY(128)
    , m_gridWidth(2), m_gridHeight(2)
    , m_vertexShaderLoader(Shader::Type::VertexShader)
    , m_fragmentShaderLoader(Shader::Type::FragmentShader)
    , m_renderer(GetDevice())
{
}

void MapApplication::Initialize()
{
    Application::Initialize();
    m_imGui.Initialize(GetMainWindow());

    // Build textures and keep them in a list
    InitializeTextures();

    // Build materials and keep them in a list
    InitializeMaterials();

    // Build meshes and keep them in a list
    InitializeMeshes();
    InitializeModels();
    
    InitializeLights();

    InitializeCamera();
    InitializeRenderer();
    
    //Enable depth test
    GetDevice().EnableFeature(GL_DEPTH_TEST);
    GetDevice().EnableFeature(GL_BLEND);
    //Enable wireframe
    //GetDevice().SetWireframeEnabled(true);
}

void MapApplication::InitializeLights()
{
    // Create a directional light and add it to the scene
    std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>();
    directionalLight->SetDirection(glm::vec3(-0.3f, -1.0f, -0.3f)); // It will be normalized inside the function
    directionalLight->SetIntensity(1.0f);
    m_scene.AddSceneNode(std::make_shared<SceneLight>("directional light", directionalLight));
}

void MapApplication::Update()
{
    Application::Update();
    const Camera& camera = *m_cameraController.GetCamera()->GetCamera();

    const Window& window = GetMainWindow();
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    m_waterMaterial->SetUniformValue("Time", GetCurrentTime());

    // Add the scene nodes to the renderer
    RendererSceneVisitor rendererSceneVisitor(m_renderer);
    m_scene.AcceptVisitor(rendererSceneVisitor);
}

void MapApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    m_renderer.Render();

    RenderGui();
}

void MapApplication::RenderGui()
{
    m_imGui.BeginFrame();
    
    // Draw GUI for scene nodes, using the visitor pattern
    ImGuiSceneVisitor imGuiVisitor(m_imGui, "Scene");
    m_scene.AcceptVisitor(imGuiVisitor);
    
    // Draw GUI for camera controller
    m_cameraController.DrawGUI(m_imGui);

    m_imGui.EndFrame();
}

void MapApplication::Cleanup()
{
    m_imGui.Cleanup();
}

void MapApplication::InitializeTextures()
{
    m_defaultTexture = CreateDefaultTexture();
    for (int z = 0; z < m_gridHeight; z++)
    {
        for (int x = 0; x < m_gridWidth; x++)
        {
            CreateHeightMap(m_gridX, m_gridY, glm::ivec2(-x, -z));
        }
    }

    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/BlueSkyCubeMap.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);

    m_dirtTexture = LoadTexture("textures/dirt.png");
    m_grassTexture = LoadTexture("textures/grass.jpg");
    m_rockTexture = LoadTexture("textures/rock.jpg");
    m_snowTexture = LoadTexture("textures/snow.jpg");
    m_waterTexture = LoadTexture("textures/water.png");
}

void MapApplication::InitializeMaterials()
{
    m_skyboxTexture->Bind();
    float maxLod;
    m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
    TextureCubemapObject::Unbind();
    {
        // terrain material
        std::vector<const char*> fragmentShaderPaths;
        fragmentShaderPaths.push_back("shaders/version330.glsl");
        fragmentShaderPaths.push_back("shaders/utils.glsl");
        fragmentShaderPaths.push_back("shaders/blinn-phong.glsl");
        fragmentShaderPaths.push_back("shaders/lighting.glsl");
        fragmentShaderPaths.push_back("shaders/quantizedTerrain.frag");
        Shader terrainVS = m_vertexShaderLoader.Load("shaders/quantizedTerrain.vert");
        Shader terrainFS = m_fragmentShaderLoader.Load(fragmentShaderPaths);
        std::shared_ptr<ShaderProgram> terrainShaderProgram = std::make_shared<ShaderProgram>();
        terrainShaderProgram->Build(terrainVS, terrainFS);

        ShaderProgram::Location viewProjMatrixLocation = terrainShaderProgram->GetUniformLocation("ViewProjMatrix");
        ShaderProgram::Location cameraPositionLocation = terrainShaderProgram->GetUniformLocation("CameraPosition");
        ShaderProgram::Location locationWorldMatrix = terrainShaderProgram->GetUniformLocation("WorldMatrix");
        // Register shader with renderer
        m_renderer.RegisterShaderProgram(terrainShaderProgram,
            [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
            {
                if (cameraChanged)
                {
                    shaderProgram.SetUniform(cameraPositionLocation, camera.ExtractTranslation());
                    shaderProgram.SetUniform(viewProjMatrixLocation, camera.GetViewProjectionMatrix());
                }
                shaderProgram.SetUniform(locationWorldMatrix, worldMatrix);
            },
            m_renderer.GetDefaultUpdateLightsFunction(*terrainShaderProgram)
        );

        m_terrainMaterials.push_back(std::make_shared<Material>(terrainShaderProgram));
        m_terrainMaterials[0]->SetUniformValue("ColorTexture0", m_dirtTexture);
        m_terrainMaterials[0]->SetUniformValue("ColorTexture1", m_grassTexture);
        m_terrainMaterials[0]->SetUniformValue("ColorTexture2", m_rockTexture);
        m_terrainMaterials[0]->SetUniformValue("ColorTexture3", m_snowTexture);
        m_terrainMaterials[0]->SetUniformValue("ColorTextureRange01", glm::vec2(.3f, .5f));
        m_terrainMaterials[0]->SetUniformValue("ColorTextureRange12", glm::vec2(.5f, .7f));
        m_terrainMaterials[0]->SetUniformValue("ColorTextureRange23", glm::vec2(.7f, .8f));
        m_terrainMaterials[0]->SetUniformValue("ColorTextureScale", glm::vec2(0.05f));
        m_terrainMaterials[0]->SetUniformValue("Color", glm::vec4(1.0f));
        m_terrainMaterials[0]->SetUniformValue("TerrainWidth", static_cast<int>(m_gridX));

        m_terrainMaterials[0]->SetUniformValue("AmbientColor", glm::vec3(0.25f));

        m_terrainMaterials[0]->SetUniformValue("EnvironmentTexture", m_skyboxTexture);
        m_terrainMaterials[0]->SetUniformValue("EnvironmentMaxLod", maxLod);


        for (int i = 1; i < m_gridWidth * m_gridHeight; i++)
        {
            std::shared_ptr<Material> material = std::make_shared<Material>(*m_terrainMaterials[0]);
            m_terrainMaterials.push_back(material);
            m_terrainMaterials[i]->SetUniformValue("Heightmap", m_heightMaps[i]);
            m_terrainMaterials[i]->SetUniformValue("NormalMap", m_normalMaps[i]);
        }

        m_terrainMaterials[0]->SetUniformValue("Heightmap", m_heightMaps[0]);
        m_terrainMaterials[0]->SetUniformValue("NormalMap", m_normalMaps[0]);
    }
    {
        // water material
        std::vector<const char*> waterFragmentShaderPaths;
        waterFragmentShaderPaths.push_back("shaders/version330.glsl");
        waterFragmentShaderPaths.push_back("shaders/utils.glsl");
        waterFragmentShaderPaths.push_back("shaders/blinn-phong.glsl");
        waterFragmentShaderPaths.push_back("shaders/lighting.glsl");
        waterFragmentShaderPaths.push_back("shaders/water.frag");
        Shader waterVS = m_vertexShaderLoader.Load("shaders/water.vert");
        Shader waterFS = m_fragmentShaderLoader.Load(waterFragmentShaderPaths);
        std::shared_ptr<ShaderProgram> waterShaderProgram = std::make_shared<ShaderProgram>();
        waterShaderProgram->Build(waterVS, waterFS);

        ShaderProgram::Location waterViewProjMatrixLocation = waterShaderProgram->GetUniformLocation("ViewProjMatrix");
        ShaderProgram::Location waterLocationWorldMatrix = waterShaderProgram->GetUniformLocation("WorldMatrix");
        // Register shader with renderer
        m_renderer.RegisterShaderProgram(waterShaderProgram,
            [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
            {
                if (cameraChanged)
                {
                    shaderProgram.SetUniform(waterViewProjMatrixLocation, camera.GetViewProjectionMatrix());
                }
                shaderProgram.SetUniform(waterLocationWorldMatrix, worldMatrix);
            },
            m_renderer.GetDefaultUpdateLightsFunction(*waterShaderProgram)
        );

        m_waterMaterial = std::make_shared<Material>(waterShaderProgram);

        m_terrainMaterials[0]->SetUniformValue("AmbientColor", glm::vec3(0.25f));

        m_terrainMaterials[0]->SetUniformValue("EnvironmentTexture", m_skyboxTexture);
        m_terrainMaterials[0]->SetUniformValue("EnvironmentMaxLod", maxLod);
        
        m_waterMaterial->SetUniformValue("TerrainWidth", static_cast<int>(m_gridX));
        m_waterMaterial->SetUniformValue("ColorTexture", m_waterTexture);
        m_waterMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.05f));
        m_waterMaterial->SetUniformValue("Color", glm::vec4(1.0f, 1.0f, 1.0f, .6f));
        m_waterMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);
        m_waterMaterial->SetBlendEquation(Material::BlendEquation::Add);
    }
}

void MapApplication::InitializeMeshes()
{
    m_terrainPatch = std::make_shared<Mesh>();
    CreateTerrainMesh(m_gridX, m_gridY);
}

void MapApplication::InitializeModels()
{
    std::vector<glm::vec3> gridPositionTranslations;
    for (int z = 0; z < m_gridHeight; z++)
    {
        for (int x = 0; x < m_gridWidth; x++)
        {
            gridPositionTranslations.push_back(glm::vec3(-x, 0.0f, -z));
        }
    }

    glm::vec3 pushDownTranslate = glm::vec3(0.0f, -.5f, 0.0f);
    glm::vec3 waterLevel = glm::vec3(0.0f, -0.15f, 0.0f);
    auto terrainPatchPtr = std::make_shared<Mesh>();

    auto waterModelPointer = std::make_shared<Model>(m_terrainPatch);
    waterModelPointer->AddMaterial(m_waterMaterial);

    for (int i = 0; i < m_gridWidth * m_gridHeight; i++)
    {
        auto terrainModelPointer = std::make_shared<Model>(m_terrainPatch);
        std::shared_ptr<Transform> terrainTransform = std::make_shared<Transform>();
        terrainTransform->SetScale(glm::vec3(10.0f));
        terrainTransform->SetTranslation(glm::vec3(10.0f) * (gridPositionTranslations[i] + pushDownTranslate));
        terrainModelPointer->AddMaterial(m_terrainMaterials[i]);
        const std::string& terrainChunkName = std::format("Terrain chunk {}", i);
        auto terrainChunkNode = std::make_shared<SceneModel>(terrainChunkName, terrainModelPointer, terrainTransform);
        m_scene.AddSceneNode(terrainChunkNode);
    }

    for (int i = 0; i < m_gridWidth * m_gridHeight; i++)
    {
        auto waterTransform = std::make_shared<Transform>();
        waterTransform->SetScale(glm::vec3(10.0f));
        waterTransform->SetTranslation(glm::vec3(10.0f) * (gridPositionTranslations[i] + waterLevel));
        const std::string& waterChunkName = std::format("Water chunk {}", i);
        m_scene.AddSceneNode(
            std::make_shared<SceneModel>(
                waterChunkName, waterModelPointer, waterTransform));
    }
}

void MapApplication::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>());
    m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
}

void MapApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
    m_scene.AddSceneNode(sceneCamera);
}

std::shared_ptr<Texture2DObject> MapApplication::CreateDefaultTexture()
{
    std::shared_ptr<Texture2DObject> texture = std::make_shared<Texture2DObject>();

    int width = 4;
    int height = 4;
    std::vector<float> pixels;
    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            pixels.push_back(1.0f);
            pixels.push_back(0.0f);
            pixels.push_back(1.0f);
            pixels.push_back(1.0f);
        }
    }

    texture->Bind();
    texture->SetImage<float>(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, pixels);
    texture->GenerateMipmap();

    return texture;
}

std::shared_ptr<Texture2DObject> MapApplication::LoadTexture(const char* path)
{
    std::shared_ptr<Texture2DObject> texture = std::make_shared<Texture2DObject>();

    int width = 0;
    int height = 0;
    int components = 0;

    unsigned char* data = stbi_load(path, &width, &height, &components, 4);

    texture->Bind();
    texture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, std::span<const unsigned char>(data, width * height * 4));

    texture->GenerateMipmap();

    stbi_image_free(data);

    return texture;
}

void MapApplication::CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords)
{
    std::shared_ptr<Texture2DObject> heightmap = std::make_shared<Texture2DObject>();

    std::vector<float> pixels;
    for (unsigned int j = 0; j < height; ++j)
    {
        for (unsigned int i = 0; i < width; ++i)
        {
            float x = i / static_cast<float>(width - 1);
            float y = j / static_cast<float>(height - 1);

            float height = (stb_perlin_fbm_noise3(x + coords.x, y + coords.y, 0.0f, 2.0f, .5f, 6) + 1.0f) * .5f;

            pixels.push_back(height);
        }
    }

    std::vector<float> normals;
    std::shared_ptr<Texture2DObject> normalMap = std::make_shared<Texture2DObject>();

    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            int columns = width;
            
            float heightL = x == 0 ? pixels[y * columns + x] : pixels[y * columns + x - 1];
            float heightR = x == width - 1? pixels[y * columns + x] : pixels[y * columns + x + 1];
            float heightD = y == 0 ? pixels[y * columns + x] : pixels[(y - 1) * columns + x];
            float heightU = y == height - 1 ? pixels[y * columns + x] : pixels[(y + 1) * columns + x];
            
            auto dx = glm::vec3(1, 0, (heightR - heightL) / 2.0);
            auto dy = glm::vec3(0, 1, (heightU - heightD) / 2.0);

            auto normal = glm::normalize(glm::cross(dy, dx));
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);
        }
    }

    normalMap->Bind();
    normalMap->SetImage<float>(0, width, height, TextureObject::FormatRGB, TextureObject::InternalFormatRGB8, normals);
    normalMap->GenerateMipmap();
    m_normalMaps.push_back(normalMap);
    Texture2DObject::Unbind();
    
    heightmap->Bind();
    heightmap->SetImage<float>(0, width, height, TextureObject::FormatR, TextureObject::InternalFormatR16F, pixels);
    heightmap->GenerateMipmap();
    m_heightMaps.push_back(heightmap);
    Texture2DObject::Unbind();
}

void MapApplication::CreateTerrainMesh(unsigned int gridX, unsigned int gridY)
{
    // Define the vertex structure
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2 texCoord)
            : position(position), normal(normal), texCoord(texCoord) {
        }
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // Define the vertex format (should match the vertex structure)
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(2);

    // List of vertices (VBO)
    std::vector<Vertex> vertices;

    // List of indices (EBO)
    std::vector<unsigned int> indices;

    // Grid scale to convert the entire grid to size 1x1
    glm::vec2 scale(1.0f / (gridX - 1), 1.0f / (gridY - 1));

    // Number of columns and rows
    unsigned int columnCount = gridX;
    unsigned int rowCount = gridY;

    // Iterate over each VERTEX
    for (unsigned int j = 0; j < rowCount; ++j)
    {
        for (unsigned int i = 0; i < columnCount; ++i)
        {
            // Vertex data for this vertex only
            glm::vec3 position(i * scale.x, 0.0f, j * scale.y);
            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            
            glm::vec2 texCoord(i, j);
            vertices.emplace_back(position, normal, texCoord);

            // Index data for quad formed by previous vertices and current
            if (i > 0 && j > 0)
            {
                unsigned int top_right = j * columnCount + i; // Current vertex
                unsigned int top_left = top_right - 1;
                unsigned int bottom_right = top_right - columnCount;
                unsigned int bottom_left = bottom_right - 1;

                //Triangle 1
                indices.push_back(bottom_right);
                indices.push_back(bottom_left);
                indices.push_back(top_left);

                //Triangle 2
                indices.push_back(bottom_right);
                indices.push_back(top_left);
                indices.push_back(top_right);
            }
        }
    }

    m_terrainPatch->AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
}