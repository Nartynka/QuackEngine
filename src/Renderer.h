#pragma once

#include <vector>
#include <glm.hpp>

namespace Quack
{
	class VertexArray;
	class IndexBuffer;
	class Shader;
	class Model;
	class Texture;

	class Renderer
	{
	public:		
		static void Init();

		static Shader* linesShader;

		static void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader);
		static void DrawNotIndexed(const VertexArray& vao, unsigned int count, const Shader& shader);
		static void DrawMesh(const VertexArray& vao, const IndexBuffer& ibo, const std::vector<Texture>& textures, Shader& shader);
		
		// Debug
		static void DrawOutline(const VertexArray& vao, const IndexBuffer& ibo, glm::mat4 model, glm::vec3 color = glm::vec3(0.f, 0.5f, 1.f));
		static void DrawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color = glm::vec3(0.f, 1.f, 0.f));
		//static void DrawDebug();
		static void DrawPoint(glm::vec3 point, glm::vec3 color = glm::vec3(0.f, 1.f, 1.f)); 
	};
}
