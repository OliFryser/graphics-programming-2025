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

uniform vec3 CameraPosition;


float inverseMix(vec2 range, float value)
{
	return clamp((value - range.x) / (range.y - range.x), 0.0, 1.0);
}

void main()
{
	// Sample all the color textures
	vec4 color0 = texture(ColorTexture0, TexCoord * ColorTextureScale);
	vec4 color1 = texture(ColorTexture1, TexCoord * ColorTextureScale);
	vec4 color2 = texture(ColorTexture2, TexCoord * ColorTextureScale);
	vec4 color3 = texture(ColorTexture3, TexCoord * ColorTextureScale);

	// I could check if this frag is under waterlevel and darken it by multiplying by a darker color
	// Mix between them according to height ranges
	vec4 tex = color0;
	tex = mix(tex, color1, inverseMix(ColorTextureRange01, Height));
	tex = mix(tex, color2, inverseMix(ColorTextureRange12, Height));
	tex = mix(tex, color3, inverseMix(ColorTextureRange23, Height));

	SurfaceData data;
	data.normal = normalize(WorldNormal);
	data.reflectionColor = Color.rgb * tex.rgb;
	vec3 arm = vec3(1.0, 1.0, 1.0);
	data.ambientReflectance = arm.x;
	data.diffuseReflectance = 1.0;
	data.specularReflectance = pow(1.0 - arm.y, 4);
	data.specularExponent = 2.0 / pow(arm.y, 2) - 2.0;

	vec3 position = WorldPosition;
	vec3 viewDir = GetDirection(position, CameraPosition);
	vec3 color = ComputeLighting(position, data, viewDir, true);
	
	FragColor = vec4(data.normal, 1.0);
}