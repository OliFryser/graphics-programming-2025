#include "MapApplication.h"

#include <ituGL/asset/TextureCubemapLoader.h>
#include <ituGL/texture/Texture2DObject.h>
#include <ituGL/texture/FramebufferObject.h>

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

#include <stb_image.h>

#include <imgui.h>
#include <format>
#include <ituGL/renderer/PostFXRenderPass.h>

MapApplication::MapApplication()
    : Application(1024, 1024, "Individual Project")
    , m_gridX(128), m_gridY(128)
    , m_gridWidth(2), m_gridHeight(2)
    , m_vertexShaderLoader(Shader::Type::VertexShader)
    , m_fragmentShaderLoader(Shader::Type::FragmentShader)
    , m_renderer(GetDevice())
    , m_ambientColor(.25f)
    , m_heightScale(.5f)
    , m_smoothingAmount(0.35f)
    , m_waterLevel(2.5f)
    , m_levels(10)
    , m_quantizeTerrain(true)
    , m_sceneFramebuffer(std::make_shared<FramebufferObject>())
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

    //Enable wireframe
    //GetDevice().SetWireframeEnabled(true);
}

void MapApplication::InitializeLights()
{
    // Create a directional light and add it to the scene
    std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>();
    directionalLight->SetDirection(glm::vec3(-0.3f, -1.0f, -0.3f)); // It will be normalized inside the function
    directionalLight->SetIntensity(1.0f);
    auto sceneLight = std::make_shared<SceneLight>("directional light", directionalLight);
    m_scene.AddSceneNode(sceneLight);
}

void MapApplication::Update()
{
    Application::Update();
    const Camera& camera = *m_cameraController.GetCamera()->GetCamera();

    const Window& window = GetMainWindow();
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    m_waterMaterial->SetUniformValue("Time", GetCurrentTime());
    UpdateMaterialsUniform("AmbientColor", m_ambientColor);
    UpdateTerrainMaterialsUniform("HeightScale", m_heightScale);
    UpdateTerrainMaterialsUniform("SmoothingAmount", m_smoothingAmount);
    UpdateTerrainMaterialsUniform("Levels", m_levels);
    UpdateTerrainMaterialsUniform("QuantizeTerrain", static_cast<int>(m_quantizeTerrain));
    

    for (int i = 0; i < m_gridWidth * m_gridHeight; i++)
    {
        auto waterChunk = m_waterScene.GetSceneNode(std::format("Water chunk {}", i));
        auto currentTranslation = waterChunk->GetTransform()->GetTranslation();
        waterChunk->GetTransform()->SetTranslation(glm::vec3(currentTranslation.x, m_waterLevel, currentTranslation.z));
    }

    // Add the scene nodes to the renderer
    RendererSceneVisitor rendererSceneVisitor(m_renderer);
    m_scene.AcceptVisitor(rendererSceneVisitor);
    //m_waterScene.AcceptVisitor(rendererSceneVisitor);
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
    
    if (auto window = m_imGui.UseWindow("Lighting Controls"))
    {
        ImGui::ColorEdit3("Ambient Color", &m_ambientColor[0]);
    }

    if (auto window = m_imGui.UseWindow("Terrain Options"))
    {
        ImGui::DragInt("Levels", &m_levels, 1.0f);
        ImGui::DragFloat("Height Scale", &m_heightScale, .05f, 0.0f);
        ImGui::DragFloat("Smoothing Amount", &m_smoothingAmount, .005f, 0.0f, 10.0f);

        ImGui::DragFloat("Water Level", &m_waterLevel, .05f, 0.0f);
        ImGui::Checkbox("Quantize Terrain", &m_quantizeTerrain);
        
    }

    // Draw GUI for scene nodes, using the visitor pattern
    ImGuiSceneVisitor imGuiVisitor(m_imGui, "Scene");
    m_scene.AcceptVisitor(imGuiVisitor);
    m_waterScene.AcceptVisitor(imGuiVisitor);
    
    // Draw GUI for camera controller
    m_cameraController.DrawGUI(m_imGui);

    //DrawRaymarchGui();

    m_imGui.EndFrame();
}

void MapApplication::DrawRaymarchGui()
{
    if (auto window = m_imGui.UseWindow("Raymarch parameters"))
    {
        const auto& viewTransform = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

        if (ImGui::TreeNodeEx("Sphere", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 sphereCenter(glm::vec3(-2.0f, 0.0f, -10.0f));

            ImGui::ColorEdit3("Sphere Color", m_cloudsMaterial->GetDataUniformPointer<float>("SphereColor"));
            ImGui::DragFloat3("Sphere Center", &sphereCenter.x, .1f);
            auto transformedPosition = (viewTransform * glm::vec4(sphereCenter, 1.0));
            m_cloudsMaterial->SetUniformValue("SphereCenter", glm::vec3(transformedPosition.x, transformedPosition.y, transformedPosition.z));

            ImGui::DragFloat("Sphere Radius", m_cloudsMaterial->GetDataUniformPointer<float>("SphereRadius"), .1f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Box", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 translation(2, 0, -10);
            static glm::vec3 rotation(0.0f);

            ImGui::ColorEdit3("Box Color", m_cloudsMaterial->GetDataUniformPointer<float>("BoxColor"));
            ImGui::DragFloat3("Box Translation", &translation.x, .1f);
            ImGui::DragFloat3("Box rotation", &rotation.x, .1f);

            auto boxTransform = glm::translate(translation);
            boxTransform = glm::rotate(boxTransform, rotation.x, glm::vec3(1.0f, .0f, .0f));
            boxTransform = glm::rotate(boxTransform, rotation.y, glm::vec3(.0f, 1.0f, .0f));
            boxTransform = glm::rotate(boxTransform, rotation.z, glm::vec3(.0f, .0f, 1.0f));

            m_cloudsMaterial->SetUniformValue("BoxMatrix", viewTransform * boxTransform);

            ImGui::DragFloat("Box Size", m_cloudsMaterial->GetDataUniformPointer<float>("BoxSize"), .1f);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Ray Marching", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat("Smoothness", m_cloudsMaterial->GetDataUniformPointer<float>("Smoothness"), .01f);
            ImGui::TreePop();
        }
    }
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

    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/BlueSkyCubeMapLQ.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);

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
        m_terrainMaterials[0]->SetUniformValue("ChunkRows", static_cast<int>(m_gridHeight));
        m_terrainMaterials[0]->SetUniformValue("ChunkColumns", static_cast<int>(m_gridWidth));
        
        m_terrainMaterials[0]->SetUniformValue("AmbientColor", glm::vec3(0.25f));
        m_terrainMaterials[0]->SetUniformValue("Levels", m_levels);

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

        m_waterMaterial->SetUniformValue("AmbientColor", glm::vec3(0.25f));

        m_waterMaterial->SetUniformValue("EnvironmentMaxLod", maxLod);
        m_waterMaterial->SetUniformValue("EnvironmentTexture", m_skyboxTexture);
        
        m_waterMaterial->SetUniformValue("TerrainWidth", static_cast<int>(m_gridX));
        m_waterMaterial->SetUniformValue("ColorTexture", m_waterTexture);
        m_waterMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.05f));
        m_waterMaterial->SetUniformValue("Color", glm::vec4(1.0f, 1.0f, 1.0f, .6f));
        m_waterMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);
        m_waterMaterial->SetBlendEquation(Material::BlendEquation::Add);

    }
    m_cloudsMaterial = CreateRaymarchingMaterial("shaders/raymarching/cloud.glsl", m_sceneTexture);

    m_cloudsMaterial->SetUniformValue("SphereColor", glm::vec3(0.0f, 0.0f, 1.0f));
    m_cloudsMaterial->SetUniformValue("SphereCenter", glm::vec3(-2.0f, 0.0f, -10.0f));
    m_cloudsMaterial->SetUniformValue("SphereRadius", 1.25f);
    m_cloudsMaterial->SetUniformValue("BoxColor", glm::vec3(1.0f, 0.0f, .0f));
    m_cloudsMaterial->SetUniformValue("BoxMatrix", glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 2, 0, -10, 1));
    m_cloudsMaterial->SetUniformValue("BoxSize", glm::vec3(1.0f, 1.0f, 1.0f));
    m_cloudsMaterial->SetUniformValue("Smoothness", .5f);
}

void MapApplication::InitializeMeshes()
{
    m_terrainPatch = std::make_shared<Mesh>();
    CreateTerrainMesh(m_gridX, m_gridY);
}

void MapApplication::InitializeModels()
{
    glm::vec3 scale = glm::vec3(10.0f);

    std::vector<glm::vec3> gridPositionTranslations;
    for (int z = 0; z < m_gridHeight; z++)
    {
        for (int x = 0; x < m_gridWidth; x++)
        {
            gridPositionTranslations.push_back(glm::vec3(-x, 0.0f, -z));
        }
    }

    auto terrainPatchPtr = std::make_shared<Mesh>();

    auto waterModelPointer = std::make_shared<Model>(m_terrainPatch);
    waterModelPointer->AddMaterial(m_waterMaterial);

    for (int i = 0; i < m_gridWidth * m_gridHeight; i++)
    {
        auto terrainModelPointer = std::make_shared<Model>(m_terrainPatch);
        std::shared_ptr<Transform> terrainTransform = std::make_shared<Transform>();
        terrainTransform->SetScale(scale);
        terrainTransform->SetTranslation(scale * gridPositionTranslations[i]);
        terrainModelPointer->AddMaterial(m_terrainMaterials[i]);
        const std::string& terrainChunkName = std::format("Terrain chunk {}", i);
        auto terrainChunkNode = std::make_shared<SceneModel>(terrainChunkName, terrainModelPointer, terrainTransform);
        m_scene.AddSceneNode(terrainChunkNode);

        auto waterTransform = std::make_shared<Transform>();
        waterTransform->SetScale(scale);
        waterTransform->SetTranslation(scale * (gridPositionTranslations[i]));
        const std::string& waterChunkName = std::format("Water chunk {}", i);
        m_waterScene.AddSceneNode(
            std::make_shared<SceneModel>(
                waterChunkName, waterModelPointer, waterTransform));
    }
}

void MapApplication::InitializeRenderer()
{
    InitializeFramebuffers();
    m_renderer.AddRenderPass(std::make_unique<ForwardRenderPass>(m_sceneFramebuffer));
    m_renderer.AddRenderPass(std::make_unique<SkyboxRenderPass>(m_skyboxTexture));
    m_renderer.AddRenderPass(std::make_unique<PostFXRenderPass>(m_cloudsMaterial, m_renderer.GetDefaultFramebuffer()));
}

void MapApplication::InitializeFramebuffers()
{
    int width, height;
    GetMainWindow().GetDimensions(width, height);

    // Scene Texture
    m_sceneTexture = std::make_shared<Texture2DObject>();
    m_sceneTexture->Bind();
    m_sceneTexture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormat::InternalFormatRGBA16F);
    m_sceneTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_LINEAR);
    m_sceneTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_LINEAR);
    Texture2DObject::Unbind();

    // Depth texture
    m_depthTexture = std::make_shared<Texture2DObject>();
    m_depthTexture->Bind();
    m_depthTexture->SetImage(0, width, height, TextureObject::FormatDepth, TextureObject::InternalFormat::InternalFormatDepth);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_LINEAR);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_LINEAR);
    Texture2DObject::Unbind();

    // Scene framebuffer
    m_sceneFramebuffer->Bind();
    m_sceneFramebuffer->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_sceneTexture);
    //m_sceneFramebuffer->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Depth, *m_depthTexture);
    m_sceneFramebuffer->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>
        ({ 
            FramebufferObject::Attachment::Color0
        })
    );
    FramebufferObject::Unbind();
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

            float height = stb_perlin_fbm_noise3(x + coords.x, y + coords.y, 0.0f, 2.0f, .5f, 6) * .5f + .5f;

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
            auto dz = glm::vec3(0, 1, (heightU - heightD) / 2.0);

            auto normal = glm::normalize(glm::cross(dz, dx));
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
    heightmap->SetParameter(Texture2DObject::ParameterEnum::WrapS, GL_MIRRORED_REPEAT);
    heightmap->SetParameter(Texture2DObject::ParameterEnum::WrapT, GL_MIRRORED_REPEAT);
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

std::shared_ptr<Material> MapApplication::CreateRaymarchingMaterial(const char* fragmentShaderPath, std::shared_ptr<Texture2DObject> sourceTexture)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back("shaders/raymarching/sdflibrary.glsl");
    fragmentShaderPaths.push_back("shaders/raymarching/raymarcher.glsl");
    fragmentShaderPaths.push_back("shaders/raymarching/copy.frag");
    //fragmentShaderPaths.push_back(fragmentShaderPath);
    //fragmentShaderPaths.push_back("shaders/raymarching/raymarching.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    ShaderProgram::Location projMatrixLocation = shaderProgramPtr->GetUniformLocation("ProjMatrix");
    ShaderProgram::Location invProjMatrixLocation = shaderProgramPtr->GetUniformLocation("InvProjMatrix");
    m_renderer.RegisterShaderProgram(shaderProgramPtr,
        [=](const ShaderProgram& shaderProgram, const glm::mat4& worldMatrix, const Camera& camera, bool cameraChanged)
        {
            if (cameraChanged)
            {
                shaderProgram.SetUniform(projMatrixLocation, camera.GetProjectionMatrix());
                shaderProgram.SetUniform(invProjMatrixLocation, glm::inverse(camera.GetProjectionMatrix()));
            }
        }, m_renderer.GetDefaultUpdateLightsFunction(*shaderProgramPtr));

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);
    material->SetUniformValue("SourceTexture", sourceTexture);

    return material;
}