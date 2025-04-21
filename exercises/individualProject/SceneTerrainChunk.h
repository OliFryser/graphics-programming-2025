#pragma once

#include <itugl/scene/SceneNode.h>

class SceneTerrainChunk : public SceneNode
{
public:
	SceneTerrainChunk(const std::string& name, std::shared_ptr<Transform> transform);

};