#include "TransparentForwardRenderPass.h"

#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/renderer/Renderer.h>

TransparentForwardRenderPass::TransparentForwardRenderPass()
{
}

void TransparentForwardRenderPass::Render()
{
    Renderer& renderer = GetRenderer();

    const Camera& camera = renderer.GetCurrentCamera();
    const auto& lights = renderer.GetLights();
    const auto& drawcallCollection = renderer.GetDrawcalls(m_drawcallCollectionIndex);

    // for all drawcalls
    for (const Renderer::DrawcallInfo& drawcallInfo : drawcallCollection)
    {
        // Prepare drawcall states
        renderer.PrepareDrawcall(drawcallInfo);

        std::shared_ptr<const ShaderProgram> shaderProgram = drawcallInfo.GetMaterial().GetShaderProgram();

        //for all lights
        bool first = true;
        unsigned int lightIndex = 0;
        while (renderer.UpdateLights(shaderProgram, lights, lightIndex))
        {
            glDepthFunc(first ? GL_LESS : GL_EQUAL);

            // Draw
            drawcallInfo.GetDrawcall().Draw();

            first = false;
        }
    }
}
