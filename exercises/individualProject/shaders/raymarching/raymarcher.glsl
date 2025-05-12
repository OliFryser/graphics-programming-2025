
// Forward declare distance function
float GetDistance(vec3 p);

// Forward declare sample density function
float SampleDensity(vec3 p);

// Forward declare config function
void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float marchSize, out float surfaceDistance);

// Ray marching algorithm
float RayMarch(vec3 origin, vec3 dir)
{
    float distance = 0.0f;

    // Get configuration specific to this shader pass
    uint maxSteps;
    float maxDistance, marchSize, surfaceDistance;
    GetRayMarcherConfig(maxSteps, maxDistance, marchSize, surfaceDistance);

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
    float distance;
};

// Volumetric raymarching inspired by https://blog.maximeheckel.com/posts/real-time-cloudscapes-with-volumetric-raymarching/
void VolumetricRaymarch(vec3 origin, vec3 dir, vec3 lightDir, vec3 lightColor, inout Output o)
{
    float depth = 0.0;
    vec3 p = origin + dir * depth;
    float delta = 0.3;

    vec4 res = vec4(0.0);
    
    // We need to know when we first hit the volume for the depth buffer
    float distance = -1.0;

    // Get configuration specific to this shader pass
    uint maxSteps;
    float maxDistance, marchSize, surfaceDistance;
    GetRayMarcherConfig(maxSteps, maxDistance, marchSize, surfaceDistance);

    // Iterate until maxSteps is reached or we find a point
    for (uint i = 0u; i < maxSteps; ++i)
    {
        float density = SampleDensity(p);

        // we only draw the density if it's greater than 0;
        if (density > 0.0) {
            // directional derivative
            float diffuse = clamp((SampleDensity(p) - SampleDensity(p + delta * -lightDir)) / delta, 0.0, 1.0);

            vec3 lin = vec3(0.60,0.60,0.75) * 1.1 + 0.8 * lightColor * diffuse;
            vec4 color = vec4(mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), density), density);
            color.rgb *= lin;

            color.rgb *= color.a;
            res += color * (1.0 - res.a);

            if (distance < 0.0 && res.a > 0.01) {
                distance = depth; // First visible impact
            }
        }

        depth += marchSize;
        p = origin + dir * depth;
    }
    o.color = res;
    o.distance = distance;
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
