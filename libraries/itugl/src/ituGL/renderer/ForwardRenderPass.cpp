#include <ituGL/renderer/ForwardRenderPass.h>

#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/renderer/Renderer.h>

ForwardRenderPass::ForwardRenderPass(const std::shared_ptr<FramebufferObject> targetFrameBuffer)
    : ForwardRenderPass(0, targetFrameBuffer)
{
}

ForwardRenderPass::ForwardRenderPass(int drawcallCollectionIndex, const std::shared_ptr<FramebufferObject> targetFrameBuffer)
    : RenderPass(targetFrameBuffer)
{
    m_drawcallCollectionIndex = drawcallCollectionIndex;
}

void ForwardRenderPass::Render()
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
            // Set the renderstates
            renderer.SetLightingRenderStates(first);

            // Draw
            drawcallInfo.GetDrawcall().Draw();

            first = false;
        }
    }
}
