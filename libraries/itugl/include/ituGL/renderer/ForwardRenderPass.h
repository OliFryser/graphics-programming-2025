#pragma once

#include <ituGL/renderer/RenderPass.h>

class ForwardRenderPass : public RenderPass
{
public:
    ForwardRenderPass(const std::shared_ptr<FramebufferObject> targetFrameBuffer = nullptr);
    ForwardRenderPass(int drawcallCollectionIndex, const std::shared_ptr<FramebufferObject> targetFrameBuffer = nullptr);

    void Render() override;

protected:
    int m_drawcallCollectionIndex;
};
