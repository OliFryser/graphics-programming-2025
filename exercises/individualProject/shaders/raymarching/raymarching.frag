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
uniform float DepthBias;

uniform uint MaxSteps;
uniform float MarchSize;

// Configure ray marcher
void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float marchSize, out float surfaceDistance)
{
    maxSteps = MaxSteps;
    maxDistance = ProjMatrix[3][2] / (ProjMatrix[2][2] + 1.0f); // Far plane
	marchSize = MarchSize;
    surfaceDistance = 0.001f;
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

	vec3 lightDir = normalize((ViewMatrix * vec4(LightDirection, 0.0)).xyz);
	
	Output o;
	// Use Volumetric Raymarching to sample the color
	VolumetricRaymarch(origin, dir, lightDir, LightColor, o);
	// With the output value, get the final color
	FragColor = o.color;

	// Use regular raymarching to get the distance for the depth buffer
	distance += RayMarch(origin, dir);

	// Hit point in view space is given by the direction from the camera and the distance
	float depth = o.distance - DepthBias;
	vec3 point = dir * distance;
	vec4 clip = ProjMatrix * vec4(point, 1.0);
	gl_FragDepth = clip.z / clip.w * 0.5 + 0.5;
}
