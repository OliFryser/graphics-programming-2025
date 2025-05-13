// Uniforms
uniform vec3 SphereColor;
uniform vec3 SphereCenter;
uniform float SphereRadius;

uniform vec3 BoxColor;
uniform mat4 BoxMatrix;
uniform vec3 BoxSize;

uniform float Smoothness;

uniform sampler2D NoiseTexture;
uniform float Time;

float Noise(vec3 x ) {
  vec3 p = floor(x);
  vec3 f = fract(x);
  f = f*f*(3.0-2.0*f);

  vec2 uv = (p.xy+vec2(37.0,239.0)*p.z) + f.xy;
  vec2 tex = textureLod(NoiseTexture, (uv+0.5)/256.0, 0.0).yx;

  return mix( tex.x, tex.y, f.z ) * 2.0 - 1.0;
}

float FBM(vec3 p) {
  vec3 q = p + Time * 0.5 * vec3(1.0, -0.2, -1.0);
  float g = Noise(q);

  float f = 0.0;
  float scale = 0.5;
  float factor = 2.02;

  for (int i = 0; i < 6; i++) {
      f += scale * Noise(q);
      q *= factor;
      factor += 0.21;
      scale *= 0.5;
  }

  return f;
}

// Signed distance function
float GetDistance(vec3 p)
{
	// Sphere in position "SphereCenter" and size "SphereRadius"
	float dSphere = SphereSDF(TransformToLocalPoint(p, SphereCenter), SphereRadius);

	// Box with worldView transform "BoxMatrix" and dimensions "BoxSize"
	float dBox = BoxSDF(TransformToLocalPoint(p, BoxMatrix), BoxSize);

	float debugSphere = SphereSDF(TransformToLocalPoint(p, SphereCenter), 1.0);

	float fog = SmoothSubtraction(dSphere, dBox, Smoothness);

	float d = SmoothUnion(debugSphere, fog, Smoothness);

	return d;
}

float SampleDensity(vec3 p)
{
	//float f = FBM(p);
	return -GetDistance(p);
}

