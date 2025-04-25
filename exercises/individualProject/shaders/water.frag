in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform vec2 ColorTextureScale;
uniform sampler2D ColorTexture;

uniform float Time;

uniform vec3 CameraPosition;

void main()
{
	vec2 samplingSpot = TexCoord * ColorTextureScale + vec2(sin(Time * .5), 0);
	vec4 textureColor = texture(ColorTexture, samplingSpot);

	SurfaceData data;
	data.normal = normalize(WorldNormal);
	data.reflectionColor = Color.rgb * textureColor.rgb;
	vec3 arm = vec3(1.0, 1.0, 1.0);
	data.ambientReflectance = arm.x;
	data.diffuseReflectance = 1.0;
	data.specularReflectance = pow(1.0 - arm.y, 4);
	data.specularExponent = 2.0 / pow(arm.y, 2) - 2.0;

	vec3 position = WorldPosition;
	vec3 viewDir = GetDirection(position, CameraPosition);
	vec3 color = ComputeLighting(position, data, viewDir, true);

	FragColor = vec4(color, Color.a);
}
