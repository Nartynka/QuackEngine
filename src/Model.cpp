#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Log.h"

namespace Quack
{
	Model::Model(const char* path)
	{
		loadModel(path);
	}

	Model::~Model()
	{
	}

	void Model::Draw(Shader& shader)
	{
		for (auto& mesh : meshes)
			mesh.Draw(shader);
	}

	void Model::loadModel(std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			QUACK_ERROR("ERROR ASSIMP: {}", importer.GetErrorString());
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		
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

		// process vertex positions, normals and texture coordinates
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position = { mesh->mVertices[i].x,  mesh->mVertices[i].y ,  mesh->mVertices[i].z };
			//vertex.normal = { mesh->mNormals[i].x,  mesh->mNormals[i].y ,  mesh->mNormals[i].z };
			
			//if (mesh->mTextureCoords[0])
			//	vertex.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			//else
			//	vertex.texCoords = { 0.f, 0.f };

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
			// for now only the first material
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return { vertices, indices, textures };
	}

	std::vector<Texture> Model::loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString path;
			material->GetTexture(type, i, &path);

			Texture texture;
			texture.id = i; // @TODO: load texture here form file. Concatenate the path and the directory
			texture.type = typeName;
			texture.path = path.C_Str();
			textures.push_back(texture);
		}

		return textures;
	}

}