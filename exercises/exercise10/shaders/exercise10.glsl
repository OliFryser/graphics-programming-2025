
// Uniforms
// TODO 10.1 : Replace constants with uniforms with the same name
uniform vec3 SphereColor;
uniform vec3 SphereCenter;
uniform float SphereRadius;

uniform vec3 BoxColor;
uniform mat4 BoxMatrix;
uniform vec3 BoxSize;

// Output structure
struct Output
{
	// color of the closest figure
	vec3 color;
};


// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
	// Sphere in position "SphereCenter" and size "SphereRadius"
	float dSphere = SphereSDF(TransformToLocalPoint(p, SphereCenter), SphereRadius);

	// Box with worldView transform "BoxMatrix" and dimensions "BoxSize"
	float dBox = BoxSDF(TransformToLocalPoint(p, BoxMatrix), BoxSize);

	// TODO 10.2 : Replace Union with SmoothUnion and try different small values of smoothness
	float d = Union(dSphere, dBox);

	// TODO 10.2 : Replace this with a mix, using the blend factor from SmoothUnion
	o.color = d == dSphere ? SphereColor : BoxColor;

	return d;
}

// Default value for o
void InitOutput(out Output o)
{
	o.color = vec3(0.0f);
}

// Output function: Just a dot with the normal and view vectors
vec4 GetOutputColor(vec3 p, float distance, Output o)
{
	vec3 normal = CalculateNormal(p);
	vec3 viewDir = normalize(-p);
	float dotNV = dot(normalize(-p), normal);
	return vec4(dotNV * o.color, 1.0f);
}
