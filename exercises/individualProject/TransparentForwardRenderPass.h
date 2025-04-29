#pragma once

#include "itugl/renderer/ForwardRenderPass.h"

class TransparentForwardRenderPass : public ForwardRenderPass
{
public:
    TransparentForwardRenderPass();

    void Render() override;
};