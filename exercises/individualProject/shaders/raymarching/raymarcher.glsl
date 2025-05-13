
// Forward declare distance function
float GetDistance(vec3 p);

// Forward declare sample density function
float SampleDensity(vec3 p);

// Forward declare config function
void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float surfaceDistance);
void GetVolumetricMarcherConfig(out uint maxSteps, out float marchSize, out vec3 lightColor, out vec3 volumeColor);

// Ray marching algorithm
float RayMarch(vec3 origin, vec3 dir)
{
    float distance = 0.0f;

    // Get configuration specific to this shader pass
    uint maxSteps;
    float maxDistance, surfaceDistance;
    GetRayMarcherConfig(maxSteps, maxDistance, surfaceDistance);

    // Iterate until maxSteps is reached or we find a point
    for(uint i = 0u; i < maxSteps; ++i)
    {
        // Get distance to the current point
        vec3 p = origin + dir * distance;
        float d = GetDistance(p);
        distance += d;
        // If inside cloud, just return 0 since we are inside the volume
        if (d < 0)
            return 0;

        // If distance is too big, discard the fragment
        if (distance > maxDistance)
            discard;

        // If this step increment was very small, we found a hit
        if (d < surfaceDistance)
            break;
    }

    return distance;
}

struct Output {
    vec4 color;
};

// Volumetric raymarching inspired by https://blog.maximeheckel.com/posts/real-time-cloudscapes-with-volumetric-raymarching/
void VolumetricRaymarch(vec3 origin, vec3 dir, vec3 lightDir, float maxDistance, inout Output o)
{
    float depth = 0.0;
    float delta = 0.3;

    vec4 res = vec4(0.0);
    
    // We need to know when we first hit the volume for the depth buffer
    float distance = -1.0;

    // Get configuration specific to this shader pass
    uint maxSteps;
    float marchSize;
    vec3 lightColor, volumeColor;
    GetVolumetricMarcherConfig(maxSteps, marchSize, lightColor, volumeColor);

    // Iterate until maxSteps is reached or we find a point
    for (uint i = 0u; i < maxSteps; ++i)
    {
        vec3 p = origin + dir * depth;

        if (depth > maxDistance)
            break;

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
        }

        depth += marchSize;
    }
    o.color = res;
}

uniform int RaymarchHack;
// Calculate numerical normals using the tetrahedron technique with specific differential
// Implementation here because GetDistance needs to be defined
vec3 CalculateNormal(vec3 p, float h)
{
    vec3 normal = vec3(0.0f);

    #define ZERO (min(RaymarchHack, 0)) // hack to prevent inlining
    for(int i = ZERO; i < 4; i++)
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        normal += e * GetDistance(p + e * h);
    }

    return normalize(normal);
}

vec3 CalculateNormal(vec3 p)
{
    return CalculateNormal(p, 0.0001f);
}
