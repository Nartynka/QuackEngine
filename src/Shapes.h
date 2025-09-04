#pragma once

#include <memory>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#define M_PI 3.14159265358979323846f

namespace Quack
{
	// @TODO: Shapes probably should follow the same pattern as models
	class Shape
	{
	public:
		Shape() = default;

		void SetupBuffers(bool hasNormals = true)
		{
			// Create OpenGL objects
			vao = std::make_unique<VertexArray>();
			vao->Bind();
			vbo = std::make_unique<VertexBuffer>(vertices.data(), (unsigned int)(vertices.size() * sizeof(float)));
			ibo = std::make_unique<IndexBuffer>(indices.data(), (unsigned int)indices.size());

			VertexBufferLayout layout;
			layout.AddElement(3); // Position

			if(hasNormals)
				layout.AddElement(3); // Normals

			vao->AddBuffer(*vbo, layout);
		}

		unsigned int GetVerticesCount()
		{
			return (unsigned int)vertices.size();
		}

		std::unique_ptr<VertexArray> vao;
		std::unique_ptr<VertexBuffer> vbo;
		std::unique_ptr<IndexBuffer> ibo;

	protected:
		std::vector<float> vertices;
		std::vector<int> indices;
	};


	class Cube : public Shape
	{
	public:
		Cube(glm::vec3 halfSize = glm::vec3(0.5f))
		{
			vertices = {
			   // Front face position				  // Normals
			   -halfSize.x, -halfSize.y,  halfSize.z, 0.5f, 0.5f, 1.0f, // 0: bottom-left-front
				halfSize.x, -halfSize.y,  halfSize.z, 0.5f, 0.5f, 1.0f, // 1: bottom-right-front
				halfSize.x,  halfSize.y,  halfSize.z, 0.5f, 0.5f, 1.0f, // 2: top-right-front
			   -halfSize.x,  halfSize.y,  halfSize.z, 0.5f, 0.5f, 1.0f, // 3: top-left-front
			   // Back face position
			   -halfSize.x, -halfSize.y, -halfSize.z, 0.5f, 0.5f, 1.0f, // 4: bottom-left-back
				halfSize.x, -halfSize.y, -halfSize.z, 0.5f, 0.5f, 1.0f, // 5: bottom-right-back
				halfSize.x,  halfSize.y, -halfSize.z, 0.5f, 0.5f, 1.0f, // 6: top-right-back
			   -halfSize.x,  halfSize.y, -halfSize.z, 0.5f, 0.5f, 1.0f, // 7: top-left-back
			};


			indices = {
				0, 1, 2,  2, 3, 0,  // Front face
				4, 6, 5,  6, 4, 7,  // Back face
				4, 0, 3,  3, 7, 4,  // Left face
				1, 5, 6,  6, 2, 1,  // Right face
				4, 5, 1,  1, 0, 4,  // Bottom face
				3, 2, 6,  6, 7, 3,  // Top face
			};

			SetupBuffers();
		}
	};
	using Rectangle = Cube;


	class NormalCube : public Shape
	{
	public:
		NormalCube(glm::vec3 halfSize = glm::vec3(0.5f))
		{
			vertices = {
				// position						  // normals
				-halfSize.x, -halfSize.y, -halfSize.z,  0.0f, 0.0f, -1.0f,
				 halfSize.x, -halfSize.y, -halfSize.z,  0.0f, 0.0f, -1.0f,
				 halfSize.x,  halfSize.y, -halfSize.z,  0.0f, 0.0f, -1.0f,
				 halfSize.x,  halfSize.y, -halfSize.z,  0.0f, 0.0f, -1.0f,
				-halfSize.x,  halfSize.y, -halfSize.z,  0.0f, 0.0f, -1.0f,
				-halfSize.x, -halfSize.y, -halfSize.z,  0.0f, 0.0f, -1.0f,

				-halfSize.x, -halfSize.y,  halfSize.z,  0.0f, 0.0f, 1.0f,
				 halfSize.x, -halfSize.y,  halfSize.z,  0.0f, 0.0f, 1.0f,
				 halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 0.0f, 1.0f,
				 halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 0.0f, 1.0f,
				-halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 0.0f, 1.0f,
				-halfSize.x, -halfSize.y,  halfSize.z,  0.0f, 0.0f, 1.0f,

				-halfSize.x,  halfSize.y,  halfSize.z,  -1.0f, 0.0f, 0.0f,
				-halfSize.x,  halfSize.y, -halfSize.z,  -1.0f, 0.0f, 0.0f,
				-halfSize.x, -halfSize.y, -halfSize.z,  -1.0f, 0.0f, 0.0f,
				-halfSize.x, -halfSize.y, -halfSize.z,  -1.0f, 0.0f, 0.0f,
				-halfSize.x, -halfSize.y,  halfSize.z,  -1.0f, 0.0f, 0.0f,
				-halfSize.x,  halfSize.y,  halfSize.z,  -1.0f, 0.0f, 0.0f,

				 halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 0.0f, 0.0f,
				 halfSize.x,  halfSize.y, -halfSize.z,  1.0f, 0.0f, 0.0f,
				 halfSize.x, -halfSize.y, -halfSize.z,  1.0f, 0.0f, 0.0f,
				 halfSize.x, -halfSize.y, -halfSize.z,  1.0f, 0.0f, 0.0f,
				 halfSize.x, -halfSize.y,  halfSize.z,  1.0f, 0.0f, 0.0f,
				 halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 0.0f, 0.0f,

				-halfSize.x, -halfSize.y, -halfSize.z,  0.0f, -1.0f, 0.0f,
				 halfSize.x, -halfSize.y, -halfSize.z,  0.0f, -1.0f, 0.0f,
				 halfSize.x, -halfSize.y,  halfSize.z,  0.0f, -1.0f, 0.0f,
				 halfSize.x, -halfSize.y,  halfSize.z,  0.0f, -1.0f, 0.0f,
				-halfSize.x, -halfSize.y,  halfSize.z,  0.0f, -1.0f, 0.0f,
				-halfSize.x, -halfSize.y, -halfSize.z,  0.0f, -1.0f, 0.0f,

				-halfSize.x,  halfSize.y, -halfSize.z,  0.0f, 1.0f, 0.0f,
				 halfSize.x,  halfSize.y, -halfSize.z,  0.0f, 1.0f, 0.0f,
				 halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 1.0f, 0.0f,
				 halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 1.0f, 0.0f,
				-halfSize.x,  halfSize.y,  halfSize.z,  0.0f, 1.0f, 0.0f,
				-halfSize.x,  halfSize.y, -halfSize.z,  0.0f, 1.0f, 0.0f,
			};

			// Create OpenGL objects
			vao = std::make_unique<VertexArray>();
			vao->Bind();
			vbo = std::make_unique<VertexBuffer>(vertices.data(), (unsigned int)(vertices.size() * sizeof(float)));

			VertexBufferLayout layout;
			layout.AddElement(3); // Position
			layout.AddElement(3); // Normals

			vao->AddBuffer(*vbo, layout);
		}
	};


	// Debug Shapes

	class DebugCube : public Shape
	{
	public:
		DebugCube(glm::vec3 halfSize = glm::vec3(0.5f))
		{
			vertices = {
				// Front face position
				-halfSize.x, -halfSize.y,  halfSize.z, // 0: bottom-left-front
				 halfSize.x, -halfSize.y,  halfSize.z, // 1: bottom-right-front
				 halfSize.x,  halfSize.y,  halfSize.z, // 2: top-right-front
				-halfSize.x,  halfSize.y,  halfSize.z, // 3: top-left-front
				// Back face position
				-halfSize.x, -halfSize.y, -halfSize.z, // 4: bottom-left-back
				 halfSize.x, -halfSize.y, -halfSize.z, // 5: bottom-right-back
				 halfSize.x,  halfSize.y, -halfSize.z, // 6: top-right-back
				-halfSize.x,  halfSize.y, -halfSize.z, // 7: top-left-back
			};

			indices = {
				0, 1,  1, 2,  2, 3,  3, 0, // Front Face
				4, 5,  5, 6,  6, 7,  7, 4, // Back Face
				0, 4,  1, 5,  2, 6,  3, 7  // Side connecting lines
			};

			SetupBuffers(false);
		}
	};


	class DebugSphere : public Shape
	{
	public:
		DebugSphere(float radius, int segments = 32)
		{
			// @TODO: This could be a function that is called three times here
			// XY circle
			for (int i = 0; i <= segments; i++)
			{
				float theta = (float)i / segments * 2.0f * M_PI;
				float x = radius * cos(theta);
				float y = radius * sin(theta);
				float z = 0.0f;

				vertices.insert(vertices.end(), { x, y, z });
			}

			// XZ circle
			for (int i = 0; i <= segments; i++)
			{
				float theta = (float)i / segments * 2.0f * M_PI;
				float x = radius * cos(theta);
				float y = 0.0f;
				float z = radius * sin(theta);

				vertices.insert(vertices.end(), { x, y, z });
			}

			// YZ circle
			for (int i = 0; i <= segments; i++)
			{
				float theta = (float)i / segments * 2.0f * M_PI;
				float x = 0.0f;
				float y = radius * cos(theta);
				float z = radius * sin(theta);

				vertices.insert(vertices.end(), { x, y, z });
			}

			// Generate indices
			int circleVertexCount = segments + 1; // how many vertices per circle
			int circleCount = 3;

			for (int circle = 0; circle < circleCount; circle++)
			{
				int startIndex = circle * circleVertexCount;

				for (int i = 0; i < segments; i++)
				{
					indices.push_back(startIndex + i);
					indices.push_back(startIndex + i + 1);
				}
			}

			SetupBuffers(false);
		}
	};

}
