#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in float Height;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D ColorTexture0;
uniform sampler2D ColorTexture1;
uniform sampler2D ColorTexture2;
uniform sampler2D ColorTexture3;
uniform vec2 ColorTextureRange01;
uniform vec2 ColorTextureRange12;
uniform vec2 ColorTextureRange23;
uniform vec2 ColorTextureScale;

uniform sampler2D DirtTexture;
uniform sampler2D GrassTexture;
uniform sampler2D RockTexture;
uniform sampler2D SnowTexture;

uniform vec2 GrassRange;
uniform vec2 RockRange;
uniform vec2 SnowRange;

float mixSample(vec2 range, float height)
{
	return clamp((height - range.x) / (range.y - range.x), 0.0, 1.0);
}

void main()
{
	vec4 DirtSample = texture(DirtTexture, TexCoord * ColorTextureScale);
	vec4 GrassSample = texture(GrassTexture, TexCoord * ColorTextureScale);
	vec4 RockSample = texture(RockTexture, TexCoord * ColorTextureScale);
	vec4 SnowSample = texture(SnowTexture, TexCoord * ColorTextureScale);

	float mixed1 = mixSample(GrassRange, Height);
	float mixed2 = mixSample(RockRange, Height);
	float mixed3 = mixSample(SnowRange, Height);

	vec4 mix1 = mix(DirtSample, GrassSample, mixed1);
	vec4 mix2 = mix(mix1, RockSample, mixed2);
	vec4 mix3 = mix(mix2, SnowSample, mixed3);

	FragColor = Color * mix3;

	// Multiply texture color by vertex Color
	FragColor = Color * color;
}


