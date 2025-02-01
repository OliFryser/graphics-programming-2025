#include "Circle.h"

#include <iostream>

#include "Utils.h"

Circle::Circle(float radius, int detail) : _radius(radius)
{
	float angle = 0.0f;
	float anglePerVertex = 360.0f / detail;

	AddVertex(0.0f, 0.0f, 0.0f);
	AddVertexForTriangle(angle);

	int index = 1; 
	
	for (int i = 0; i < detail; i++)
	{
		angle += anglePerVertex;
		AddVertexForTriangle(angle);
		AddTriangleIndices(0, index, index + 1);
		index++;
	}
}

void Circle::AddVertex(float x, float y, float z)
{
	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);
}

void Circle::AddTriangleIndices(unsigned int first, unsigned int second, unsigned int third)
{
	indices.push_back(first);
	indices.push_back(second);
	indices.push_back(third);
}

void Circle::AddVertexForTriangle(float angle)
{
	float angleInRadians = ConvertDegreesToRadians(angle);
	float x = sin(angleInRadians) * _radius;
	float y = cos(angleInRadians) * _radius;
	float z = 0.0f;
	AddVertex(x, y, z);
}
