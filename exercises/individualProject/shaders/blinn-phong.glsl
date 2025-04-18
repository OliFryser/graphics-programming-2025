#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D ColorTexture;
uniform vec4 Color;
uniform vec3 AmbientColor;
uniform float AmbientReflection;

uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform float DiffuseReflection;

uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 CameraPosition;

vec3 GetAmbientReflection(vec4 color)
{
	// R_ambient = I_a * K_a * color
	return AmbientColor * AmbientReflection* color.rgb;
}

vec3 GetDiffuseReflection(vec4 color, vec3 lightVector, vec3 worldNormal)
{
	return LightColor * DiffuseReflection * color.rgb * clamp(dot(worldNormal, lightVector), 0, 1);
}

vec3 GetSpecularReflection(vec3 worldNormal, vec3 halfVector)
{
	return LightColor * SpecularReflection * pow(clamp(dot(worldNormal, halfVector), 0, 1), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec4 color, vec3 lightVector, vec3 worldNormal, vec3 halfVector)
{
	return GetAmbientReflection(color) 
		+ GetDiffuseReflection(color, lightVector, worldNormal) 
		+ GetSpecularReflection(worldNormal, halfVector);
}
