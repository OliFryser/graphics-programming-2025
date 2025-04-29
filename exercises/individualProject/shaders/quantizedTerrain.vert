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
uniform int ChunkRows;
uniform int ChunkColumns;
uniform bool QuantizeTerrain;

uniform int Levels;
uniform float SmoothingAmount;
uniform float HeightScale;

float QuantizeHeight(float height)
{
	float level = ceil(height * Levels);
	float base = level / Levels;
	float above = (level + 1.0) / Levels;

	// get the fractional part (how close we are to the next plateau)
	float t = height * Levels - (level - 1.0);

	// Smooth transition
	float smoothT = smoothstep(0.0, SmoothingAmount, t);
	return mix(base, above, smoothT);
}

float SampleHeightMap(vec2 samplePoint)
{
	// wrap around texture
	return (texture(Heightmap, (samplePoint * (TerrainWidth - 1) + 0.5) / TerrainWidth)).x * HeightScale;
}

vec3 CalculateNormalFromNeighbors()
{
	vec2 posL = VertexPosition.xz + vec2(-1, 0) / vec2(TerrainWidth - 1);
	float heightL = SampleHeightMap(posL);
	vec2 posR = VertexPosition.xz + vec2(1, 0) / vec2(TerrainWidth - 1);
	float heightR = SampleHeightMap(posR);
	vec2 posD = VertexPosition.xz + vec2(0, -1) / vec2(TerrainWidth - 1);
	float heightD = SampleHeightMap(posD);
	vec2 posU = VertexPosition.xz + vec2(0, 1) / vec2(TerrainWidth - 1);
	float heightU = SampleHeightMap(posU);
	if (QuantizeTerrain)
	{
		heightL = QuantizeHeight(heightL);
		heightR = QuantizeHeight(heightR);
		heightD = QuantizeHeight(heightD);
		heightU = QuantizeHeight(heightU);
	}

	vec3 dx = normalize(vec3(posL.x, heightL, posL.y) - vec3(posR.x, heightR, posR.y));
	vec3 dz = normalize(vec3(posD.x, heightD, posD.y) - vec3(posU.x, heightU, posU.y));

    return normalize(cross(dz, dx));
}

void main()
{
	float height = SampleHeightMap(VertexPosition.xz);
	
	if (QuantizeTerrain)
		height = QuantizeHeight(height);
	
	//vec4 normal = texture(NormalMap, (VertexPosition.xz * 127 + 0.5)/128);
	vec3 normal = CalculateNormalFromNeighbors();

	WorldPosition = (WorldMatrix * vec4(VertexPosition.x, height, VertexPosition.z, 1.0)).xyz;
	WorldNormal = (WorldMatrix * vec4(normal.xyz, 0.0)).xyz;
	TexCoord = VertexTexCoord;
	Height = height;
	
	gl_Position = ViewProjMatrix * vec4(WorldPosition, 1.0);
}