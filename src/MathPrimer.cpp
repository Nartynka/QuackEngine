#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/epsilon.hpp>

#include <memory>
#include "GLFW/glfw3.h"
#include "Scene.h"
#include "Renderer.h"

namespace Quack 
{
	void ClosestPointPlane(const std::shared_ptr<Scene> scene);
	void ClosestPointLine(const std::shared_ptr<Scene> scene);
	void PolygonClipping(const std::shared_ptr<Scene> scene);
	void SutherlandHodgmanClipping(glm::vec3* shape, glm::vec3* shapeNormals, int shapeSize, glm::vec3* shapeToBeClipped, int shapeToBeClippedSize);
	void ClosestPointsOfTwoLines(const std::shared_ptr<Scene> scene);

	void MathPrimer(const std::shared_ptr<Scene> scene)
	{
		//ClosestPointPlane(scene);
		//ClosestPointLine(scene);
		PolygonClipping(scene);

		//ClosestPointsOfTwoLines(scene);
	}


	void ClosestPointsOfTwoLines(const std::shared_ptr<Scene> scene)
	{
		float currentTime = glfwGetTime();

		// Line 1
		glm::vec3 A = { -1.4f, 0.f, 0.f };
		glm::vec3 B = { 0.6f, 0.f, 0.f };
		
		// Line 2
		glm::vec3 C = { -0.7f, -1.f, 0.5f };
		glm::vec3 D = { 0.5f, 1.5f, 0.5f };

		if (!scene->isPaused || scene->shouldDoOneStep)
		{
			A.x += glm::sin(currentTime) * 0.5f;
			A.y += glm::cos(currentTime) * 0.4f;
			A.z += glm::sin(currentTime) * 0.2f;

			B.x += glm::sin(currentTime) * 0.5f;
			B.y += glm::cos(currentTime) * 0.4f;
			B.z += glm::sin(currentTime) * 0.2f;

			C.x -= glm::sin(currentTime) * 0.1f;
			C.y -= glm::cos(currentTime) * 0.2f;
			C.z -= glm::sin(currentTime) * 0.2f;

			D.x -= glm::sin(currentTime) * 0.1f;
			D.y -= glm::cos(currentTime) * 0.2f;
			D.z -= glm::sin(currentTime) * 0.2f;
		}

		Renderer::DrawLine(A, B, glm::vec3(1.f, 0.f, 0.f));
		Renderer::DrawLine(C, D, glm::vec3(0.f, 1.f, 0.f));

		glm::vec3 ab = B - A;
		glm::vec3 cd = D - C;

		// Point on a line: 
		// L1(s) = A + s*ab
		// L2(t) = C + t*cd
		//
		// We have to find s & t so that vector between these points (L1-L2) will be perpendicular to both lines
		// dot(L1(s) - L2(t), ab) = 0  &  dot(L1(s) - L2(t), cd) = 0

		glm::vec3 r = A - C;

		float a = glm::dot(ab, ab);
		float b = glm::dot(cd, ab);
		float c = glm::dot(r, ab);
		float e = glm::dot(cd, cd);
		float f = glm::dot(r, cd);

		float det = a * e - b * b;

		float s = (b * f - c * e) / det;
		float t = (a * f - b*c) / det;

		// !! Clamping breaks the perpendicularity to both lines !!
		//s = glm::clamp(s, 0.f, 1.f);
		//t = glm::clamp(t, 0.f, 1.f);
		// But with two edges of a cube the point will always be on both lines so no need for clamping 
		// or projecting the second point if clamping were needed

		glm::vec3 L1 = A + s * ab;
		glm::vec3 L2 = C + t * cd;

		glm::vec3 v = L1 - L2;

		//float x = glm::dot(v, ab);
		//float y = glm::dot(v, cd);
		
		//if (glm::epsilonNotEqual(x, 0.f, 0.000001f) || glm::epsilonNotEqual(y, 0.f, 0.000001f))
		//	QUACK_LOG("{}, {}", x, y);


		Renderer::DrawLine(L1, L2, glm::vec3(1.f, 0.f, 1.f));
		Renderer::DrawPoint(L1, glm::vec3(1.f, 0.f, 1.f));
		Renderer::DrawPoint(L2, glm::vec3(0.f, 1.f, 1.f));
	}


	glm::vec3 generateNormal(glm::vec3 v1, glm::vec3 v2, bool isCCW = true, bool isY = true)
	{
		if (isY)
		{
			if (isCCW)
				return glm::normalize(glm::vec3(v2.y - v1.y, -(v2.x - v1.x), 0.f));

			return glm::normalize(glm::vec3(-(v2.y - v1.y), v2.x - v1.x, 0.f));
		}

		if (isCCW)
			return glm::normalize(glm::vec3(v2.z - v1.z, 0.f, -(v2.x - v1.x)));

		return glm::normalize(glm::vec3(-(v2.z - v1.z), 0.f, v2.x - v1.x));
	}

	glm::vec3 rotateZ(glm::vec3 point, float angle = 30.f)
	{
		glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
		return rotationMat * glm::vec4(point, 1.0f);
	}

	glm::vec3 quad[] = {
		{-1.f, 0.f, 1.f},
		{1.f, 0.f, 1.f},
		{1.f, 0.f, -1.f},
		{-1.f, 0.f, -1.f},
	};

	glm::vec3 quadNormals[] = {
		generateNormal(quad[0], quad[1], false, false),
		generateNormal(quad[1], quad[2], false, false),
		generateNormal(quad[2], quad[3], false, false),
		generateNormal(quad[3], quad[0], false, false),
	};

	/*
	glm::vec3 quad2[] = {
		{-1.f, 1.f, 0.f},
		{1.f, 1.f, 0.f},
		{1.f, -1.f, 0.f},
		{-1.f, -1.f, 0.f}
	};

	glm::vec3 pentagon[] = {
		{-0.7f, -1.f, 0.f},
		{0.7f, -1.f, 0.f },
		{1.1f, 0.2f, 0.f},
		{0.f,  1.f, 0.f},
		{-1.1f, 0.2f, 0.f}
	};

	glm::vec3 pentNormals[] = {
		generateNormal(pentagon[0], pentagon[1]),
		generateNormal(pentagon[1], pentagon[2]),
		generateNormal(pentagon[2], pentagon[3]),
		generateNormal(pentagon[3], pentagon[4]),
		generateNormal(pentagon[4], pentagon[0]),
	};

	glm::vec3 pentagon2[] = {
		{-0.2f, -1.2f, 0.f},
		{1.2f, -1.2f, 0.f },
		{1.6f, 0.f, 0.f},
		{0.5f,  0.8f, 0.f},
		{-0.6f, 0.f, 0.f}
	};

	glm::vec3 duck[] = {
		{-1.0f, -1.2f, 0.f},
		{ 0.6f, -1.2f, 0.f},
		{ 1.4f, -0.6f, 0.f},
		{ 1.0f,  0.2f, 0.f},
		{ 1.0f,  1.0f, 0.f},
		{ 1.6f,  1.0f, 0.f},
		{ 0.8f,  1.6f, 0.f},
		{ 0.2f,  1.0f, 0.f},
		{ 0.2f,  0.2f, 0.f},
		{-0.6f,  0.0f, 0.f},
		{-1.4f, -0.4f, 0.f}
	};
	*/

	glm::vec3 cube[] = {
		rotateZ(glm::vec3(-0.5f, -0.5f, -0.5f)),
		rotateZ(glm::vec3(0.5f, -0.5f, -0.5f)),
		rotateZ(glm::vec3(0.5f, -0.5f,  0.5f)),
		rotateZ(glm::vec3(-0.5f, -0.5f,  0.5f)),
		rotateZ(glm::vec3(-0.5f, -0.5f, -0.5f)),

		rotateZ(glm::vec3(-0.5f,  0.5f, -0.5f)),

		rotateZ(glm::vec3(0.5f,  0.5f, -0.5f)),
		rotateZ(glm::vec3(0.5f,  0.5f,  0.5f)),
		rotateZ(glm::vec3(-0.5f,  0.5f,  0.5f)),
		rotateZ(glm::vec3(-0.5f,  0.5f, -0.5f)),

		rotateZ(glm::vec3(0.5f,  0.5f, -0.5f)),
		rotateZ(glm::vec3(0.5f, -0.5f, -0.5f)),
		rotateZ(glm::vec3(0.5f, -0.5f,  0.5f)),
		rotateZ(glm::vec3(0.5f,  0.5f,  0.5f)),
		rotateZ(glm::vec3(-0.5f,  0.5f,  0.5f)),
		rotateZ(glm::vec3(-0.5f, -0.5f,  0.5f)),
	};

	void PolygonClipping(const std::shared_ptr<Scene> scene)
	{
		float currentTime = glfwGetTime();

		if (!scene->isPaused || scene->shouldDoOneStep)
		{
			for (glm::vec3& l : cube)
			{
				l.x += glm::cos(currentTime) * 0.02f;
				l.y += glm::sin(currentTime) * 0.02f;
			}
		}
		
		SutherlandHodgmanClipping(quad, quadNormals, 4, cube, 4);

		Renderer::DrawLine(quad[0], quad[0]+quadNormals[0], glm::vec3(1.f, 0.f, 0.f));
		Renderer::DrawLine(quad[1], quad[1]+quadNormals[1], glm::vec3(0.f, 1.f, 0.f));
		Renderer::DrawLine(quad[2], quad[2]+quadNormals[2], glm::vec3(0.f, 0.f, 1.f));
		Renderer::DrawLine(quad[3], quad[3]+quadNormals[3], glm::vec3(1.f, 1.f, 1.f));
		
		Renderer::DrawPolygon(16, cube, glm::vec3(1.f, 0.f, 0.f));
	}

	// Shape & shapeNormals should be the same size
	void SutherlandHodgmanClipping(glm::vec3* shape, glm::vec3* shapeNormals, int shapeSize, glm::vec3* shapeToBeClipped, int shapeToBeClippedSize)
	{
		// You can treat pointers as iterators, first element + size
		std::vector<glm::vec3> polygonToBeClipped(shapeToBeClipped, shapeToBeClipped + shapeToBeClippedSize);

		for (int i = 0; i < shapeSize; i++)
		{
			glm::vec3 normal = normalize(shapeNormals[i]);
			glm::vec3 point = shape[i];
			glm::vec3 v1;
			glm::vec3 v2;

			std::vector<glm::vec3> clippedPolygon;

			for (int j = 0; j < polygonToBeClipped.size(); j++)
			{
				v1 = polygonToBeClipped[j]; // A
				v2 = polygonToBeClipped[(j + 1) % polygonToBeClipped.size()]; // B

				if (glm::dot(normal, v1 - point) > 0.f)
				{
					// Both outside (keep nothing)
					if (glm::dot(normal, v2 - point) > 0.f) {}

					// Outside inside (keep intersection & v2)
					else
					{
						glm::vec3 ab = v2 - v1;

						float d = dot(normal, point);
						float t = (d - dot(normal, v1)) / dot(normal, ab);

						glm::vec3 intersection = v1 + t * ab;

						clippedPolygon.push_back(intersection);
						clippedPolygon.push_back(v2);
					}
				}
				else
				{
					// inside outside (keep only intersection)
					if (glm::dot(normal, v2 - point) > 0.f)
					{
						glm::vec3 ab = v2 - v1;

						float d = dot(normal, point);
						float t = (d - dot(normal, v1)) / dot(normal, ab);

						glm::vec3 intersection = v1 + t * ab;

						clippedPolygon.push_back(intersection);
					}
					// inside inside (keep v2)
					else
					{
						clippedPolygon.push_back(v2);
					}
				}
			}

			polygonToBeClipped = clippedPolygon;
		}


		glm::vec3 clippingNormal = { 0.f, -1.f, 0.f };
		std::vector<glm::vec3> clipped;

		for (auto p : polygonToBeClipped)
		{
			if(dot(p - shape[0], clippingNormal) > 0)
			{ 
				clipped.push_back(p);
			}
		}


		Renderer::DrawPolygon(clipped.size(), clipped.data(), glm::vec3(1.f, 1.f, 1.f));

		Renderer::DrawPolygon(polygonToBeClipped.size(), polygonToBeClipped.data(), glm::vec3(0.f, 0.f, 1.f));
		Renderer::DrawPolygon(shapeSize, shape);
	}



	void ClosestPointPlane(const std::shared_ptr<Scene> scene)
	{
		float currentTime = glfwGetTime();

		glm::vec3 point = { -2.f, 0.5f, -4.f };
		glm::vec4 pointColor = { 1.f, 0.f, 0.f, 1.f };

		glm::vec3 plainPoint = glm::vec3(0.f, 0.f, -5.f);
		glm::vec3 plainNormal = glm::vec3(0.f, 1.f, 0.f);

		if (!scene->isPaused || scene->shouldDoOneStep)
		{
			point.y += glm::cos(currentTime) * 0.4f;
			point.x += glm::sin(currentTime) * 0.4f;
			point.z += glm::sin(currentTime) * 0.2f;
		}

		glm::vec3 R = point - glm::dot(plainNormal, point - plainPoint) * plainNormal;

		if (glm::dot(plainNormal, (point - plainPoint)) > 0.f)
		{
			pointColor = { 0.f, 1.f, 0.f, 1.f };
		}
		else
			pointColor = { 1.f, 0.f, 0.f, 1.f };

		Renderer::DrawPoint(point, pointColor);
		Renderer::DrawPoint(R, glm::vec4(1.f));
		Renderer::DrawLine(plainPoint, plainPoint + plainNormal);
	}

	void ClosestPointLine(const std::shared_ptr<Scene> scene)
	{
		float currentTime = glfwGetTime();

		glm::vec3 point = { -1.f, 3.f, -3.f };
		glm::vec4 pointColor = { 1.f, 0.f, 0.f, 1.f };
		// Point on line AB: A + t(B-A)

		if (!scene->isPaused || scene->shouldDoOneStep)
		{
			point.x += glm::sin(currentTime) * 0.3f;
			point.y += glm::cos(currentTime) * 0.3f;
		}

		glm::vec3 A = { 0.f, 0.f, -3.f }; // lineStart
		glm::vec3 B = { 0.f, 3.f, -3.f }; // lineEnd

		glm::vec3 lineVec = B - A;
		glm::vec3 lineDir = glm::normalize(lineVec);

		float t = glm::dot(point - A, lineDir) / glm::length(lineVec);

		if (t > 1.f)
			t = 1.f;
		if (t < 0.f)
			t = 0.f;

		glm::vec3 P = A + t * lineVec;
		
		Renderer::DrawPoint(point, pointColor);
		Renderer::DrawPoint(P);
		Renderer::DrawLine(A, B, glm::vec4(1.f));
	}
}