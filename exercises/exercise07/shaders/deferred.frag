//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D DepthTexture;
uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D OthersTexture;
uniform mat4 InvViewMatrix;
uniform mat4 InvProjMatrix;

struct SurfaceData {
	vec3 normal;
	vec3 reflectionColor;
	float ambientReflectance;
	float diffuseReflectance;
	float specularReflectance;
	float specularExponent;
};

void main()
{
	SurfaceData data;
	data.normal = (InvViewMatrix * vec4(GetImplicitNormal(texture(NormalTexture, TexCoord).xy), 0.0)).xyz;
	data.reflectionColor = texture(AlbedoTexture, TexCoord).xyz;
	vec4 others = texture(OthersTexture, TexCoord);
	data.ambientReflectance = others.r;
	data.diffuseReflectance = others.g;
	data.specularReflectance = others.b;
	data.specularExponent = (1 / others.a) + 1;

	vec3 viewSpacePosition = ReconstructViewPosition(DepthTexture, TexCoord, InvProjMatrix);
	// camera is at 0,0,0 so we compute view vector easily
	vec3 viewVector = viewSpacePosition - vec3(0);
	vec3 viewVectorWorldSpace = (InvViewMatrix * vec4(viewVector, 0.0)).xyz;
	vec3 worldSpacePosition = (InvViewMatrix * vec4(viewSpacePosition, 1.0)).xyz;

	FragColor = vec4(ComputeLighting(worldSpacePosition, data, viewVectorWorldSpace, true), 1.0);
	//FragColor = vec4(others.xyz, 1.0);
}
