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

uniform int Levels;
uniform float SmoothingAmount;
uniform float QuantizeStep;
uniform float HeightScale;

void main()
{
	float height = (texture(Heightmap, (VertexPosition.xz * (TerrainWidth - 1) + 0.5) / TerrainWidth)).x * HeightScale;
	
	float level = ceil(height * Levels);
	float base = level / Levels;
	float above = (level + 1.0) / Levels;

	// How close are we to the next plateau?
	float t = height * Levels - (level - 1.0);

	// Smooth transition
	float smoothT = smoothstep(0.0, SmoothingAmount, t); // SmoothingAmount is a small value like 0.05
	height = mix(base, above, smoothT);
	
	vec4 normal = texture(NormalMap, (VertexPosition.xz * 127 + 0.5)/128);
	WorldPosition = (WorldMatrix * vec4(VertexPosition.x, height, VertexPosition.z, 1.0)).xyz;

	WorldNormal = (WorldMatrix * vec4(normal.xyz, 0.0)).xyz;
	TexCoord = VertexTexCoord;
	Height = height;
	
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}