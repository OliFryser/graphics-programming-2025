#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/geometry/VertexBufferObject.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/geometry/ElementBufferObject.h>


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
    Color GetColorFromHeight(float height);

private:
    unsigned int m_gridX, m_gridY;

    unsigned int m_shaderProgram;

    VertexBufferObject m_vbo;
    VertexArrayObject m_vao;
    ElementBufferObject m_ebo;

};