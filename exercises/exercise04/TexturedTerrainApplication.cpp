#include "TexturedTerrainApplication.h"

#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/texture/Texture2DObject.h>

#include <glm/gtx/transform.hpp>  // for matrix transformations

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <iostream>
#include <numbers>  // for PI constant

TexturedTerrainApplication::TexturedTerrainApplication()
    : Application(1024, 1024, "Textures demo")
    , m_gridX(128), m_gridY(128)
    , m_vertexShaderLoader(Shader::Type::VertexShader)
    , m_fragmentShaderLoader(Shader::Type::FragmentShader)
{
}

void TexturedTerrainApplication::Initialize()
{
    Application::Initialize();

    // Build textures and keep them in a list
    InitializeTextures();

    // Build materials and keep them in a list
    InitializeMaterials();

    // Build meshes and keep them in a list
    InitializeMeshes();

    //Enable depth test
    GetDevice().EnableFeature(GL_DEPTH_TEST);

    //Enable wireframe
    //GetDevice().SetWireframeEnabled(true);
}

void TexturedTerrainApplication::Update()
{
    Application::Update();

    const Window& window = GetMainWindow();

    glm::vec2 mousePosition = window.GetMousePosition(true);
    m_camera.SetViewMatrix(glm::vec3(0.0f, 15.0f, 15.0f), glm::vec3(mousePosition, 0.0f));

    int width, height;
    window.GetDimensions(width, height);
    float aspectRatio = static_cast<float>(width) / height;
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 100.0f);

    m_waterMaterial->SetUniformValue("Time", GetCurrentTime());
}

void TexturedTerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    glm::mat4 q1Translate = glm::translate(glm::vec3(0, 0, -1.0f));
    glm::mat4 q2Translate = glm::translate(glm::vec3(-1.0f, 0, -1.0f));
    glm::mat4 q3Translate = glm::translate(glm::vec3(-1.0f, 0, 0));
    glm::mat4 q4Translate = glm::translate(glm::vec3(0, 0, 0));

    glm::mat4 pushDownTranslate = glm::translate(glm::vec3(0.0f, -.5f, 0.0f));

    // Terrain patches
    DrawObject(m_terrainPatch, *m_terrainMaterials[1], glm::scale(glm::vec3(10.0f)) * q1Translate * pushDownTranslate);
    DrawObject(m_terrainPatch, *m_terrainMaterials[3], glm::scale(glm::vec3(10.0f)) * q2Translate * pushDownTranslate);
    DrawObject(m_terrainPatch, *m_terrainMaterials[2], glm::scale(glm::vec3(10.0f)) * q3Translate * pushDownTranslate);
    DrawObject(m_terrainPatch, *m_terrainMaterials[0], glm::scale(glm::vec3(10.0f)) * q4Translate * pushDownTranslate);
    
    glm::mat4 waterLevel = glm::translate(glm::vec3(0.0f, -0.15f, 0.0f));

    // Water patches
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::scale(glm::vec3(10.0f)) * q1Translate * waterLevel);
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::scale(glm::vec3(10.0f)) * q2Translate * waterLevel);
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::scale(glm::vec3(10.0f)) * q3Translate * waterLevel);
    DrawObject(m_terrainPatch, *m_waterMaterial, glm::scale(glm::vec3(10.0f)) * q4Translate * waterLevel);

}

void TexturedTerrainApplication::InitializeTextures()
{
    m_defaultTexture = CreateDefaultTexture();
    m_heightMaps.push_back(CreateHeightMap(m_gridX, m_gridY, glm::ivec2(0, 0)));
    m_heightMaps.push_back(CreateHeightMap(m_gridX, m_gridY, glm::ivec2(0, -1)));
    m_heightMaps.push_back(CreateHeightMap(m_gridX, m_gridY, glm::ivec2(-1, 0)));
    m_heightMaps.push_back(CreateHeightMap(m_gridX, m_gridY, glm::ivec2(-1, -1)));

    m_dirtTexture = LoadTexture("textures/dirt.png");
    m_grassTexture = LoadTexture("textures/grass.jpg");
    m_rockTexture = LoadTexture("textures/rock.jpg");
    m_snowTexture = LoadTexture("textures/snow.jpg");
    m_waterTexture = LoadTexture("textures/water.png");
}

void TexturedTerrainApplication::InitializeMaterials()
{
    // Default shader program
    Shader defaultVS = m_vertexShaderLoader.Load("shaders/default.vert");
    Shader defaultFS = m_fragmentShaderLoader.Load("shaders/default.frag");
    std::shared_ptr<ShaderProgram> defaultShaderProgram = std::make_shared<ShaderProgram>();
    defaultShaderProgram->Build(defaultVS, defaultFS);

    // Default material
    m_defaultMaterial = std::make_shared<Material>(defaultShaderProgram);
    m_defaultMaterial->SetUniformValue("Color", glm::vec4(1.0f));

    // terrain material
    Shader terrainVS = m_vertexShaderLoader.Load("shaders/terrain.vert");
    Shader terrainFS = m_fragmentShaderLoader.Load("shaders/terrain.frag");
    std::shared_ptr<ShaderProgram> terrainShaderProgram = std::make_shared<ShaderProgram>();
    terrainShaderProgram->Build(terrainVS, terrainFS);

    m_terrainMaterials.push_back(std::make_shared<Material>(terrainShaderProgram));
    m_terrainMaterials[0]->SetUniformValue("DirtTexture", m_dirtTexture);
    m_terrainMaterials[0]->SetUniformValue("GrassTexture", m_grassTexture);
    m_terrainMaterials[0]->SetUniformValue("RockTexture", m_rockTexture);
    m_terrainMaterials[0]->SetUniformValue("SnowTexture", m_snowTexture);
    m_terrainMaterials[0]->SetUniformValue("GrassRange", glm::vec2(.3f, .5f));
    m_terrainMaterials[0]->SetUniformValue("RockRange", glm::vec2(.5f, .7f));
    m_terrainMaterials[0]->SetUniformValue("SnowRange", glm::vec2(.7f, .8f));
    m_terrainMaterials[0]->SetUniformValue("ColorTextureScale", glm::vec2(0.05f));
    m_terrainMaterials[0]->SetUniformValue("Color", glm::vec4(1.0f));

    for (int i = 1; i < 4; i++)
    {
        std::shared_ptr<Material> material = std::make_shared<Material>(*m_terrainMaterials[0].get());
        m_terrainMaterials.push_back(material);
        m_terrainMaterials[i]->SetUniformValue("Heightmap", m_heightMaps[i]);
    }

    m_terrainMaterials[0]->SetUniformValue("Heightmap", m_heightMaps[0]);

    // water material
    Shader waterVS = m_vertexShaderLoader.Load("shaders/water.vert");
    Shader waterFS = m_fragmentShaderLoader.Load("shaders/water.frag");
    std::shared_ptr<ShaderProgram> waterShaderProgram = std::make_shared<ShaderProgram>();
    waterShaderProgram->Build(waterVS, waterFS);

    m_waterMaterial = std::make_shared<Material>(waterShaderProgram);
    m_waterMaterial->SetUniformValue("ColorTexture", m_waterTexture);
    m_waterMaterial->SetUniformValue("ColorTextureScale", glm::vec2(0.05f));
    m_waterMaterial->SetUniformValue("Color", glm::vec4(1.0f, 1.0f, 1.0f, .5f));
    m_waterMaterial->SetBlendEquation(Material::BlendEquation::Add);
    m_waterMaterial->SetBlendParams(Material::BlendParam::SourceAlpha, Material::BlendParam::OneMinusSourceAlpha);
}

void TexturedTerrainApplication::InitializeMeshes()
{
    CreateTerrainMesh(m_terrainPatch, m_gridX, m_gridY);
}

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::CreateDefaultTexture()
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

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::LoadTexture(const char* path)
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

std::shared_ptr<Texture2DObject> TexturedTerrainApplication::CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords)
{
    std::shared_ptr<Texture2DObject> heightmap = std::make_shared<Texture2DObject>();
    float maxHeight = 0;
    float minHeight = 10000.0f;

    std::vector<float> pixels;
    for (unsigned int j = 0; j < height; ++j)
    {
        for (unsigned int i = 0; i < width; ++i)
        {
            float x = i / static_cast<float>(width - 1);
            float y = j / static_cast<float>(height - 1);
            
            float height = (stb_perlin_fbm_noise3(x + coords.x, y + coords.y, 0.0f, 2.0f, .5f, 6) + 1.0f) * .5f;
            
            if (height > maxHeight)
                maxHeight = height;
            if (height < minHeight)
                minHeight = height;

            pixels.push_back(height);
        }
    }

    std::cout << "Min Height: " << minHeight << ", Max Height: " << maxHeight << std::endl;

    heightmap->Bind();
    heightmap->SetImage<float>(0, width, height, TextureObject::FormatR, TextureObject::InternalFormatR16F, pixels);
    heightmap->GenerateMipmap();

    return heightmap;
}

void TexturedTerrainApplication::DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix)
{
    material.Use();

    ShaderProgram& shaderProgram = *material.GetShaderProgram();
    ShaderProgram::Location locationWorldMatrix = shaderProgram.GetUniformLocation("WorldMatrix");
    material.GetShaderProgram()->SetUniform(locationWorldMatrix, worldMatrix);
    ShaderProgram::Location locationViewProjMatrix = shaderProgram.GetUniformLocation("ViewProjMatrix");
    material.GetShaderProgram()->SetUniform(locationViewProjMatrix, m_camera.GetViewProjectionMatrix());

    mesh.DrawSubmesh(0);
}

void TexturedTerrainApplication::CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY)
{
    // Define the vertex structure
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2 texCoord)
            : position(position), normal(normal), texCoord(texCoord) {}
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
                indices.push_back(bottom_left);
                indices.push_back(bottom_right);
                indices.push_back(top_left);

                //Triangle 2
                indices.push_back(bottom_right);
                indices.push_back(top_left);
                indices.push_back(top_right);
            }
        }
    }

    mesh.AddSubmesh<Vertex, unsigned int, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
}
