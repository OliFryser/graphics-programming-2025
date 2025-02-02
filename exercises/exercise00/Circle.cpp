#include "Circle.h"

#include <iostream>

#include "Utils.h"

Circle::Circle(float radius, int detail) : m_Radius(radius)
{
	float angle = 0.0f;
	float anglePerVertex = 360.0f / detail;

	AddVertex(0.0f, 0.0f, 0.0f);
	AddVertexAtAngle(angle);
	
	for (int i = 0; i < detail; i++)
	{
		angle += anglePerVertex;
		AddVertexAtAngle(angle);
		AddTriangleIndices(0, i + 1, i + 2);
	}
}

void Circle::TranslateCircle(float xTranslation, float yTranslation)
{
	for (size_t i = 0; i < vertices.size(); i += 3)
	{
		vertices[i] += xTranslation;
		vertices[i + 1] += yTranslation;
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

void Circle::AddVertexAtAngle(float angle)
{
	float angleInRadians = ConvertDegreesToRadians(angle);
	float x = sin(angleInRadians) * m_Radius;
	float y = cos(angleInRadians) * m_Radius;
	float z = 0.0f;
	AddVertex(x, y, z);
}
