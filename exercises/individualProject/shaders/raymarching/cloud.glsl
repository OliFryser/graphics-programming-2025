// Uniforms
uniform vec3 SphereColor;
uniform vec3 SphereCenter;
uniform float SphereRadius;

uniform vec3 BoxColor;
uniform mat4 BoxMatrix;
uniform vec3 BoxSize;

uniform float Smoothness;

uniform sampler3D NoiseTexture;
uniform float Time;
uniform mat4 InvViewMatrix;
uniform float NoiseStrength;
uniform float NoiseScale;
uniform float CloudDensity;

float Noise(vec3 worldP) {
	return texture(NoiseTexture, worldP).r * 2.0 - 1.0;
}

float FBM(vec3 worldP)
{
    float total = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;

    float lacunarity = 2.0;
    float gain = 0.5;
    
    for (int i = 0; i < 4; ++i)
    {
        total += Noise(worldP * frequency) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return total;
}

// Signed distance function
float GetDistance(vec3 p)
{
	// Sphere in position "SphereCenter" and size "SphereRadius"
	float dSphere = SphereSDF(TransformToLocalPoint(p, SphereCenter), SphereRadius);

	// Box with worldView transform "BoxMatrix" and dimensions "BoxSize"
	float dBox = BoxSDF(TransformToLocalPoint(p, BoxMatrix), BoxSize);

	float d = SmoothSubtraction(dSphere, dBox, Smoothness);

	return d;
}

float SampleDensity(vec3 p)
{
    vec3 worldP = (InvViewMatrix * vec4(p, 1.0)).xyz;

    float sdf = GetDistance(p);
    float fbm = FBM(worldP * NoiseScale);
    float noise = (fbm * 0.5 + 0.5) * NoiseStrength;

    return (-sdf + noise) * CloudDensity;
}