#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>


class TerrainApplication : public Application
{
public:
    TerrainApplication();
    TerrainApplication(unsigned int gridX, unsigned int gridY);

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void BuildShaders();
    void UpdateOutputMode();

private:
    unsigned int m_gridX, m_gridY;

    unsigned int m_shaderProgram;

    VertexBufferObject m_vbo;
    VertexArrayObject m_vao;

    // (todo) 01.5: Declare an EBO

};
