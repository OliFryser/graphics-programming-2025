#version 330 core

layout (location = 0) in vec2 ParticlePosition;
layout (location = 1) in float ParticleSize;
layout (location = 2) in float ParticleBirth;
layout (location = 3) in float ParticleDuration;
layout (location = 4) in vec4 ParticleColor;
layout (location = 5) in vec2 ParticleVelocity;

out vec4 Color;

uniform float CurrentTime;
uniform float Gravity;

vec2 GetNewPosition(float age)
{
	vec2 gravityAcceleration = vec2(0.0, Gravity);
	return ParticlePosition + ParticleVelocity * age + 0.5 * gravityAcceleration * age * age;
}

void main()
{
	Color = ParticleColor;

	float age = CurrentTime - ParticleBirth;
	gl_PointSize = age <= ParticleDuration ? ParticleSize : 0.0;

	gl_Position = vec4(GetNewPosition(age), 0.0, 1.0);
}

