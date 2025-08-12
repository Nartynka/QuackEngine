#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Log.h"
#include "Texture.h"

namespace Quack
{
	Model::Model(const char* path)
	{
		loadModel(path);
	}

	Model::~Model() = default;

	void Model::Draw(Shader& shader) const
	{
		for (auto& mesh : meshes)
			mesh.Draw(shader);
	}

	void Model::loadModel(std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			QUACK_ERROR("ERROR ASSIMP: {}", importer.GetErrorString());
			return;
		}
		directory = path.substr(0, path.find_last_of('/')+1);
		
		processNode(scene->mRootNode, scene);
	}

	void Model::processNode(aiNode* node, const aiScene* scene)
	{
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// retrieve the mesh from scene by id on the child
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		std::vector <Texture> textures;

		// process vertex positions and texture coordinates
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

			if (mesh->HasTextureCoords(0))
			{
				//QUACK_GOOD("Texture coord found! u: {}, v: {}", mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				vertex.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}
			else
			{
				QUACK_WARN("No texture coord found");
				vertex.texCoords = { 0.0f, 0.0f };
			}

			if (mesh->HasNormals())
			{
				vertex.normal = { mesh->mNormals->x, mesh->mNormals->y, mesh->mNormals->z };
			}
			else
			{
				QUACK_WARN("No normals found");
				vertex.normal = { 1.0f, 0.0f, 0.0f };
			}


			vertices.push_back(vertex);
		}

		// process indices
		// each mesh contains an array of faces (triangles here) that contains the indices of the vertices we need
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// @TODO: Other textures, normals etc.
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		}

		return { vertices, indices, textures };
	}

	std::vector<Texture> Model::loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString fileName;
			material->GetTexture(type, i, &fileName);

			Texture texture;
			std::string path = directory + fileName.C_Str();
			texture.id = Texture::LoadFromFile(path.c_str());
			texture.type = typeName;
			texture.path = path;
			textures.push_back(texture);
		}

		return textures;
	}

}
