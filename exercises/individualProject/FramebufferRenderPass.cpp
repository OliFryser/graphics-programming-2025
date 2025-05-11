#include "FramebufferRenderPass.h"

#include <itugl/texture/FramebufferObject.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <ituGL/geometry/VertexArrayObject.h>
#include <ituGL/renderer/Renderer.h>
#include <array>

FramebufferRenderPass::FramebufferRenderPass(int width, int height, int drawcallCollectionIndex)
	: ForwardRenderPass(drawcallCollectionIndex)
{
    InitializeTextures(width, height);
    InitializeFramebuffer();
}


void FramebufferRenderPass::InitializeTextures(int width, int height)
{
    // Depth: Set the min and magfilter as nearest
    m_depthTexture = std::make_shared<Texture2DObject>();
    m_depthTexture->Bind();
    m_depthTexture->SetImage(0, width, height, TextureObject::FormatDepth, TextureObject::InternalFormatDepth);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_depthTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);

    // Color: Bind the newly created texture, set the image, and the min and magfilter as nearest
    m_colorTexture = std::make_shared<Texture2DObject>();
    m_colorTexture->Bind();
    m_colorTexture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA);
    m_colorTexture->SetParameter(TextureObject::ParameterEnum::MinFilter, GL_NEAREST);
    m_colorTexture->SetParameter(TextureObject::ParameterEnum::MagFilter, GL_NEAREST);
    
    Texture2DObject::Unbind();
}

void FramebufferRenderPass::InitializeFramebuffer()
{
    std::shared_ptr<FramebufferObject> targetFramebuffer = std::make_shared<FramebufferObject>();

    targetFramebuffer->Bind();

    targetFramebuffer->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Depth, *m_depthTexture);

    // Set the color texture as color attachment 0
    targetFramebuffer->SetTexture(FramebufferObject::Target::Draw, FramebufferObject::Attachment::Color0, *m_colorTexture);

    // Set the draw buffers used by the framebuffer (all attachments except depth)
    targetFramebuffer->SetDrawBuffers(std::array<FramebufferObject::Attachment, 1>(
        {
            FramebufferObject::Attachment::Color0,
        }));

    m_targetFramebuffer = targetFramebuffer;

    FramebufferObject::Unbind();
}

void FramebufferRenderPass::UpdateTextures(int width, int height)
{
    m_depthTexture->Bind();
    m_depthTexture->SetImage(0, width, height, TextureObject::FormatDepth, TextureObject::InternalFormatDepth);

    m_colorTexture->Bind();
    m_colorTexture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA);

    Texture2DObject::Unbind();
}

void FramebufferRenderPass::Render()
{
    Renderer& renderer = GetRenderer();
    renderer.GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);
    
    ForwardRenderPass::Render();
}

void FramebufferRenderPass::UpdateFramebuffers(int width, int height)
{
    UpdateTextures(width, height);
}
