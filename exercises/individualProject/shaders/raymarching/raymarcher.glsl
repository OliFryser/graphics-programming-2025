
// Forward declare distance function
float GetDistance(vec3 p);

// Forward declare sample density function
float SampleDensity(vec3 p);

// Forward declare config function
void GetVolumetricMarcherConfig(out uint maxSteps, out float marchSize, out float maxSafeStep, out vec3 lightColor, out vec3 volumeColor);

struct Output {
    vec4 color;
};

// Volumetric raymarching inspired by https://blog.maximeheckel.com/posts/real-time-cloudscapes-with-volumetric-raymarching/
void VolumetricRaymarch(vec3 origin, vec3 dir, vec3 lightDir, float maxDistance, float offset, inout Output o)
{
    float delta = 0.3;

    vec4 res = vec4(0.0);
    
    // We need to know when we first hit the volume for the depth buffer
    float distance = -1.0;

    // Get configuration specific to this shader pass
    uint maxSteps;
    float marchSize, maxSafeStep;
    vec3 lightColor, volumeColor;
    GetVolumetricMarcherConfig(maxSteps, marchSize, maxSafeStep, lightColor, volumeColor);

    float depth = marchSize * offset;
    // Iterate until maxSteps is reached or we find a point
    for (uint i = 0u; i < maxSteps; ++i)
    {
        if (depth > maxDistance)
            break;

        vec3 p = origin + dir * depth;

        float density = SampleDensity(p);

        // we only draw the density if it's greater than 0;
        if (density > 0.0) {
            // directional derivative
            float diffuse = clamp((SampleDensity(p) - SampleDensity(p + delta * -lightDir)) / delta, 0.0, 1.0);

            vec3 lin = volumeColor * 1.1 + 0.8 * lightColor * diffuse;
            vec4 color = vec4(mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), density), density);
            color.rgb *= lin;

            color.rgb *= color.a;
            res += color * (1.0 - res.a);

            // If volume is almost opaque, no reason to keep sampling
            if (res.a > 0.99)
                break;
        }

        depth += marchSize;
    }
    o.color = res;
}