#pragma once

#include <vector>

/// <summary>
/// A circle is made out of many triangles
/// radius is a float between 0 and 1.
/// detail is how many triangles
/// </summary>
class Circle {
public:
	Circle(float radius, int detail);

	void TranslateCircle(float xTranslation, float yTranslation);

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

private:
	float m_Radius;

	void AddVertex(float x, float y, float z);
	void AddTriangleIndices(unsigned int first, unsigned int second, unsigned int third);
	void AddVertexForTriangle(float angle);
};