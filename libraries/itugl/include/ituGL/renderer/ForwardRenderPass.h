#pragma once

#include <ituGL/renderer/RenderPass.h>

class ForwardRenderPass : public RenderPass
{
public:
    ForwardRenderPass();
    ForwardRenderPass(int drawcallCollectionIndex);

    void Render() override;

protected:
    int m_drawcallCollectionIndex;
};
