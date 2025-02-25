#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform vec2 ColorTextureScale;
uniform sampler2D ColorTexture;

uniform float Time;

void main()
{
	vec2 samplingSpot = TexCoord * ColorTextureScale + vec2(sin(Time * .5), 0);
	FragColor = Color * texture(ColorTexture, samplingSpot);
}
