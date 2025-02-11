#version 330 core

out vec4 FragColor;

// (todo) 02.5: Add Color input variable here


void main()
{
	float color = 1.0 - length(gl_PointCoord * 2 - 1);
	FragColor = vec4(1, 1, 1, color);
}
