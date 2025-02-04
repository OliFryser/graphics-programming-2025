#include "TerrainApplication.h"

// (todo) 01.1: Include the libraries you need

#include <cmath>
#include <iostream>
#include <vector>

#include "ituGL/core/Data.h"
#include "ituGL/geometry/VertexAttribute.h"

// Helper structures. Declared here only for this exercise
struct Vector2
{
    Vector2() : Vector2(0.f, 0.f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct Vector3
{
    Vector3() : Vector3(0.f,0.f,0.f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;

    Vector3 Normalize() const
    {
        float length = std::sqrt(1 + x * x + y * y);
        return Vector3(x / length, y / length, z / length);
    }
};

// (todo) 01.8: Declare an struct with the vertex format



TerrainApplication::TerrainApplication()
    : Application(800, 800, "Terrain demo"), m_gridX(16), m_gridY(16), m_shaderProgram(0)
{
}

TerrainApplication::TerrainApplication(unsigned int gridX, unsigned int gridY)
    : Application(800, 800, "Terrain demo"), m_gridX(gridX), m_gridY(gridY), m_shaderProgram(0)
{
}

void TerrainApplication::Initialize()
{
    Application::Initialize();

    // Build shaders and store in m_shaderProgram
    BuildShaders();

    std::vector<Vector2> uvs;

    std::vector<Vector3> vertices;
    float scaleX = 1.0f / float(m_gridX);
    float scaleY = 1.0f / float(m_gridY);

    const Vector2 bottomLeftUV(0.0f, 0.0f);
    const Vector2 bottomRightUV(1.0f, 0.0f);
    const Vector2 topLeftUV(0.0f, 1.0f);
    const Vector2 topRightUV(1.0f, 1.0f);
    
    for (int y = 0; y <= m_gridY - 1; y++)
    {
        for (int x = 0; x <= m_gridX - 1; x++)
        {
            Vector3 bottomLeft(
                x * scaleX - .5f,
                y * scaleY - .5f,
                0.0f);
            Vector3 bottomRight(
                (x + 1.0f) * scaleX - .5f,
                y * scaleY - .5f,
                0.0f);
            Vector3 topLeft(
                x * scaleX - .5f,
                (y + 1.0f) * scaleY - .5f,
                0.0f);
            Vector3 topRight(
                (x + 1.0f) * scaleX - .5f,
                (y + 1.0f) * scaleY - .5f,
                0.0f);

            // first triangle
            vertices.push_back(bottomLeft);
            uvs.push_back(bottomLeftUV);
            vertices.push_back(topLeft);
            uvs.push_back(topLeftUV);
            vertices.push_back(topRight);
            uvs.push_back(topRightUV);
            // second triangle
            vertices.push_back(topRight);
            uvs.push_back(topRightUV);
            vertices.push_back(bottomRight);
            uvs.push_back(bottomRightUV);
            vertices.push_back(bottomLeft);
            uvs.push_back(bottomLeftUV);
        }
    }

    // (todo) 01.1: Initialize VAO, and VBO
    m_vao.Bind();
    
    m_vbo.Bind();
    m_vbo.AllocateData(vertices.size() * sizeof(Vector3) + uvs.size() * sizeof(Vector2));
    
    m_vbo.UpdateData<const Vector3>(vertices, 0);
    m_vbo.UpdateData<const Vector2>(uvs, vertices.size() * sizeof(Vector3));

    std::cout << "Sizeof Vector2: " << sizeof(Vector2) << "\nSizeof Vector3: " << sizeof(Vector3) << std::endl;

    VertexAttribute position(Data::Type::Float, 3);
    m_vao.SetAttribute(0, position, 0);
    VertexAttribute uv(Data::Type::Float, 2);
    m_vao.SetAttribute(1, uv, vertices.size() * sizeof(Vector3));
    
    // (todo) 01.5: Initialize EBO


    // (todo) 01.1: Unbind VAO, and VBO
    m_vao.Unbind();
    m_vbo.Unbind();

    // (todo) 01.5: Unbind EBO

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void TerrainApplication::Update()
{
    Application::Update();

    UpdateOutputMode();
}

void TerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Set shader to be used
    glUseProgram(m_shaderProgram);

    // (todo) 01.1: Draw the grid
    m_vao.Bind();
    
    glDrawArrays(GL_TRIANGLES, 0, m_gridX * m_gridY * 6);
    
    m_vao.Unbind();
}

void TerrainApplication::Cleanup()
{
    Application::Cleanup();
}


void TerrainApplication::BuildShaders()
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "layout (location = 2) in vec3 aColor;\n"
        "layout (location = 3) in vec3 aNormal;\n"
        "uniform mat4 Matrix = mat4(1);\n"
        "out vec2 texCoord;\n"
        "out vec3 color;\n"
        "out vec3 normal;\n"
        "void main()\n"
        "{\n"
        "   texCoord = aTexCoord;\n"
        "   color = aColor;\n"
        "   normal = aNormal;\n"
        "   gl_Position = Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "uniform uint Mode = 0u;\n"
        "in vec2 texCoord;\n"
        "in vec3 color;\n"
        "in vec3 normal;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   switch (Mode)\n"
        "   {\n"
        "   default:\n"
        "   case 0u:\n"
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "       break;\n"
        "   case 1u:\n"
        "       FragColor = vec4(fract(texCoord), 0.0f, 1.0f);\n"
        "       break;\n"
        "   case 2u:\n"
        "       FragColor = vec4(color, 1.0f);\n"
        "       break;\n"
        "   case 3u:\n"
        "       FragColor = vec4(normalize(normal), 1.0f);\n"
        "       break;\n"
        "   case 4u:\n"
        "       FragColor = vec4(color * max(dot(normalize(normal), normalize(vec3(1,0,1))), 0.2f), 1.0f);\n"
        "       break;\n"
        "   }\n"
        "}\n\0";

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    m_shaderProgram = shaderProgram;
}

void TerrainApplication::UpdateOutputMode()
{
    for (int i = 0; i <= 4; ++i)
    {
        if (GetMainWindow().IsKeyPressed(GLFW_KEY_0 + i))
        {
            int modeLocation = glGetUniformLocation(m_shaderProgram, "Mode");
            glUseProgram(m_shaderProgram);
            glUniform1ui(modeLocation, i);
            break;
        }
    }
    if (GetMainWindow().IsKeyPressed(GLFW_KEY_TAB))
    {
        const float projMatrix[16] = { 0, -1.294f, -0.721f, -0.707f, 1.83f, 0, 0, 0, 0, 1.294f, -0.721f, -0.707f, 0, 0, 1.24f, 1.414f };
        int matrixLocation = glGetUniformLocation(m_shaderProgram, "Matrix");
        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(matrixLocation, 1, false, projMatrix);
    }
}
