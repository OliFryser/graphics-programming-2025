#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D ColorTexture;
uniform vec4 Color;

void main()
{
	FragColor = texture(ColorTexture, TexCoord) * Color;
}
