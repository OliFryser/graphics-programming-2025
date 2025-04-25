#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;
out float Height;

uniform sampler2D Heightmap;
uniform sampler2D NormalMap;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

uniform int TerrainWidth;
uniform float SmoothingAmount;
uniform float QuantizeStep;
uniform float HeightScale;

void main()
{
	vec4 height = texture(Heightmap, (VertexPosition.xz * (TerrainWidth - 1) + 0.5) / TerrainWidth) * HeightScale;
	vec4 normal = texture(NormalMap, (VertexPosition.xz * 127 + 0.5)/128);
	//vec4 normal = texture(NormalMap, VertexPosition.xz);

	WorldPosition = (WorldMatrix * vec4(VertexPosition.x, height.x, VertexPosition.z, 1.0)).xyz;
	WorldNormal = (WorldMatrix * vec4(normal.xyz, 0.0)).xyz;
	//WorldNormal = (WorldMatrix * vec4(VertexNormal, 0.0)).xyz;
	TexCoord = VertexTexCoord;
	Height = height.x;
	
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}