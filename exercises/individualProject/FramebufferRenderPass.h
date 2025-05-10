#pragma once

#include <itugl/renderer/ForwardRenderPass.h>
#include <itugl/texture/Texture2DObject.h>

class FramebufferRenderPass : public ForwardRenderPass
{
public:
	FramebufferRenderPass(int width, int height, int drawcallCollectionIndex = 0);

	void Render() override;
	void UpdateFramebuffers(int width, int height) override;

	std::shared_ptr<Texture2DObject> GetDepthTexture() { return m_depthTexture; }
	std::shared_ptr<Texture2DObject> GetColorTexture() { return m_colorTexture; }

private:
	void InitializeTextures(int width, int height);
	void InitializeFramebuffer();

	void UpdateTextures(int width, int height);

	std::shared_ptr<Texture2DObject> m_depthTexture, m_colorTexture;
};