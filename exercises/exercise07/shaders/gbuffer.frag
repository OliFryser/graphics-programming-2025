//Inputs
in vec3 ViewNormal;
in vec2 TexCoord;

//Outputs
out vec4 FragAlbedo;
out vec2 FragNormal;
out vec4 FragOthers;

//Uniforms
uniform vec3 Color;
uniform sampler2D ColorTexture;

uniform float AmbientReflectance;
uniform float DiffuseReflectance;
uniform float SpecularReflectance;
uniform float SpecularExponent;

void main()
{
	FragAlbedo = texture(ColorTexture, TexCoord) * vec4(Color, 1.0);
	FragNormal = ViewNormal.xy;
	float w = 1 / (1 + SpecularExponent);
	// w * (1 + SpecularExponent) = 1
	// 1 + SpecularExponent = (1 / w)
	// SpecularExponent = (1 / w) - 1
	
	FragOthers = vec4(AmbientReflectance, DiffuseReflectance, SpecularReflectance, w);
}
