#include <ituGL/renderer/PostFXRenderPass.h>

#include <ituGL/renderer/Renderer.h>
#include <ituGL/shader/Material.h>
#include <ituGL/texture/FramebufferObject.h>

PostFXRenderPass::PostFXRenderPass(std::shared_ptr<Material> material, std::shared_ptr<const FramebufferObject> framebuffer)
    : RenderPass(framebuffer), m_material(material)
{
}

void PostFXRenderPass::Render()
{
    Renderer& renderer = GetRenderer();

    assert(m_material);
    m_material->Use();
    const Mesh* mesh = &renderer.GetFullscreenMesh();
    
    bool wasSRGB = renderer.GetDevice().IsFeatureEnabled(GL_FRAMEBUFFER_SRGB);
    renderer.GetDevice().SetFeatureEnabled(GL_FRAMEBUFFER_SRGB, false);
    bool wasDepthTesting = renderer.GetDevice().IsFeatureEnabled(GL_DEPTH_TEST);
    renderer.GetDevice().SetFeatureEnabled(GL_DEPTH_TEST, false);

    mesh->DrawSubmesh(0);

    renderer.GetDevice().SetFeatureEnabled(GL_DEPTH_TEST, wasDepthTesting);
    renderer.GetDevice().SetFeatureEnabled(GL_FRAMEBUFFER_SRGB, wasSRGB);
}
