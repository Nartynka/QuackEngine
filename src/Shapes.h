#pragma once

#include <memory>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Quack
{
	class Shape
	{
	public:
		Shape() = default;

		// this is probably bad but it's temporal, will fix later @TODO
		void Initialize()
		{
			GenerateGeometry();
			SetupBuffers();
		}

		void SetupBuffers()
		{
			// Create OpenGL objects
			vao = std::make_unique<VertexArray>();
			vao->Bind();
			vbo = std::make_unique<VertexBuffer>(vertices.data(), (unsigned int)(vertices.size() * sizeof(float)));
			ibo = std::make_unique<IndexBuffer>(indices.data(), (unsigned int)indices.size());

			VertexBufferLayout layout;
			layout.AddElement(3); // Position

			vao->AddBuffer(*vbo, layout);
		}

		std::unique_ptr<VertexArray> vao;
		std::unique_ptr<VertexBuffer> vbo;
		std::unique_ptr<IndexBuffer> ibo;

	protected:

		virtual void GenerateGeometry() = 0;
		std::vector<float> vertices;
		std::vector<int> indices;
	};


	class Cube : public Shape
	{
	public:
		Cube()
		{
			Initialize();
		}

		void GenerateGeometry() override
		{
			vertices = {
				// Front face
			   -0.5f, -0.5f,  0.5f,  // 0: bottom-left-front
				0.5f, -0.5f,  0.5f,  // 1: bottom-right-front
				0.5f,  0.5f,  0.5f,  // 2: top-right-front
			   -0.5f,  0.5f,  0.5f,  // 3: top-left-front
			   // Back face
			  -0.5f, -0.5f, -0.5f,  // 4: bottom-left-back
			   0.5f, -0.5f, -0.5f,  // 5: bottom-right-back
			   0.5f,  0.5f, -0.5f,  // 6: top-right-back
			  -0.5f,  0.5f, -0.5f   // 7: top-left-back
			};

			indices = {
				0, 1, 2,  2, 3, 0, // Front face
				4, 6, 5,  6, 4, 7, // Back face
				4, 0, 3,  3, 7, 4, // Left face
				1, 5, 6,  6, 2, 1, // Right face
				4, 5, 1,  1, 0, 4, // Bottom face
				3, 2, 6,  6, 7, 3  // Top face
			};
		}
	};

}
