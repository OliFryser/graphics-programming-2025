//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform mat4 ProjMatrix;
uniform mat4 ViewMatrix;
uniform mat4 InvProjMatrix;
uniform vec3 LightDirection;
uniform vec3 LightColor;
uniform vec3 CloudColor;
uniform float DepthBias;

uniform uint MaxSteps;
uniform float MaxSafeStep;
uniform float MarchSize;
// If the depth buffer doesn't find anything use this as a fallback
uniform float MaxRenderDistance;

uniform sampler2D DepthTexture;
uniform sampler2D BlueNoiseTexture;
uniform int Frame;

// Configure ray marcher
void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float surfaceDistance)
{
    maxSteps = MaxSteps;
    maxDistance = ProjMatrix[3][2] / (ProjMatrix[2][2] + 1.0); // Far plane
    surfaceDistance = 0.001;
}

void GetVolumetricMarcherConfig(out uint maxSteps, out float marchSize, out float maxSafeStep, out vec3 lightColor, out vec3 volumeColor)
{
	maxSteps = MaxSteps;
	marchSize = MarchSize;
	maxSafeStep = MaxSafeStep;
	lightColor = LightColor;
	volumeColor = CloudColor;
}

float GetSceneDepthInViewSpace(float sceneDepth)
{
	// remap from [0;1] to [-1;1] (ndc)
	vec4 ndc = vec4(TexCoord * 2.0 - 1.0, sceneDepth * 2.0 - 1.0, 1.0);
	// get coord in viewspace
	vec4 viewPos = InvProjMatrix * ndc;
	viewPos /= viewPos.w;
	return length(viewPos.xyz);
}

void main()
{
	// Start from transformed position
	vec4 viewPos = InvProjMatrix * vec4(TexCoord.xy * 2.0 - 1.0, 0.0, 1.0);
	vec3 origin = viewPos.xyz / viewPos.w;

	// Initial distance to camera
	float distance = length(origin);
	
	// Normalize to get view direction
	vec3 dir = origin / distance;

	// Get the light direction in viewspace (this only works with one light)
	vec3 lightDir = normalize((ViewMatrix * vec4(LightDirection, 0.0)).xyz);

	// Get the depth from the depth buffer from the forward renderpass
	float sceneDepth = texture(DepthTexture, TexCoord).r;
	sceneDepth = GetSceneDepthInViewSpace(sceneDepth);
	sceneDepth = min(MaxRenderDistance, sceneDepth);

	// Sample blue noise and use offset
	float blueNoise = texture(BlueNoiseTexture, gl_FragCoord.xy / 512.0).r;
	float offset = fract(blueNoise + float(Frame%32) / sqrt(0.5));

	// Use Volumetric Raymarching to sample the color
	Output o;
	VolumetricRaymarch(origin, dir, lightDir, sceneDepth, offset, o);
	
	FragColor = o.color;
}
