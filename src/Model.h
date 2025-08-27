#pragma once
#include <vector>
#include "Mesh.h"

#include <assimp/scene.h>

// A model class that contains multiple meshes
namespace Quack
{
	class Shader;

	class Model
	{
	public:
		Model(const char* path);
		~Model();

		void Draw(Shader& shader) const;
		std::vector<Mesh> meshes;
	private:
		std::string directory; // for loading texture

		void LoadModel(std::string path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName);
	};
}