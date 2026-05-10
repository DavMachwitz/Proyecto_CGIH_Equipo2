#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SOIL2/SOIL2.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assert.h>

#include "Mesh.h"
#include "Shader.h"
#include "AssimpGLMHelper.h"  
#include "ModelAnimation.h"

using namespace std;

GLint TextureFromFile(const char *path, string directory);

class Model
{
public:
	vector<Mesh>& getMeshes() { return meshes; }
	 map<string, BoneInfo>& GetBoneInfoMap()  { return m_BoneInfoMap; }
    int&                   GetBoneCount()    { return m_BoneCounter; }
	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	Model(GLchar *path)
	{
		this->loadModel(path);
	}

	// Draws the model, and thus all its meshes
	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
		{
			this->meshes[i].Draw(shader);
		}
	}
	// En Model.h -> sección public:
	void DrawNodeAnimation(Shader shader, std::map<std::string, glm::mat4> nodeTransforms, glm::mat4 baseModel)
	{
	    for (unsigned int i = 0; i < meshes.size(); i++)
	    {
	        glm::mat4 nodeMatrix = glm::mat4(1.0f);
	        
	        // Si el nombre de esta malla existe en las animaciones, usamos esa matriz
	        if (nodeTransforms.count(meshes[i].name)) {
	            nodeMatrix = nodeTransforms[meshes[i].name];
	        }

	        // Combinamos la posición del mundo con la de la animación
	        glm::mat4 finalModel = baseModel * nodeMatrix;
	        
	        // Pasamos la matriz al shader
	        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));
	        
	        // Dibujamos la malla individualmente
	        meshes[i].Draw(shader);
	    }
	}

private:
	/*  Model Data  */
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

										/*  Functions   */
										// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	map<string, BoneInfo> m_BoneInfoMap;
    int                   m_BoneCounter = 0;
	void loadModel(string path)
	{
		// Read file via ASSIMP
		Assimp::Importer importer;
		//const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		// En Model.h -> loadModel
		//const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights);
		//const aiScene* scene = importer.ReadFile(path,
		//	aiProcess_Triangulate |
		//	aiProcess_FlipUVs |
		//	aiProcess_GlobalScale);   // ← agregar
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights
		| aiProcess_FlipUVs);
		// Check for errors
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/'));

		// Process ASSIMP's root node recursively
		this->processNode(scene->mRootNode, scene);
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene.
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			this->meshes.push_back(this->processMesh(mesh, scene, node->mName.C_Str()));
		}

		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}
	// Dentro de processMesh, antes de vertices.push_back(vertex):
	void SetVertexBoneDataToDefault(Vertex& vertex) {
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}
	void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (vertex.m_BoneIDs[i] < 0) {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }
	void ExtractBoneWeightForVertices(vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) {
		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			int boneID = -1;
	        string boneName = mesh->mBones[boneIndex]->mName.C_Str();		        
		     if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id     = m_BoneCounter;
                newBoneInfo.offset = ConvertMatrixToGLMFormat(
                    mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
 
            assert(boneID != -1);
 
            auto weights    = mesh->mBones[boneIndex]->mWeights;
            int  numWeights = mesh->mBones[boneIndex]->mNumWeights;
 
            for (int weightIndex = 0; weightIndex < numWeights; weightIndex++)
            {
                int   vertexId = weights[weightIndex].mVertexId;
                float weight   = weights[weightIndex].mWeight;
                assert(vertexId < (int)vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }


	// Asegúrate de que diga: string nodeName al final de los paréntesis
	Mesh processMesh(aiMesh *mesh, const aiScene *scene, string nodeName)
	{
	    vector<Vertex> vertices;
	    vector<GLuint> indices;
	    vector<Texture> textures;

	    for (GLuint i = 0; i < mesh->mNumVertices; i++)
	    {
	        Vertex vertex;
	        glm::vec3 vector; 
	        SetVertexBoneDataToDefault(vertex);

	        vector.x = mesh->mVertices[i].x;
	        vector.y = mesh->mVertices[i].y;
	        vector.z = mesh->mVertices[i].z;
	        vertex.Position = vector;

	        vector.x = mesh->mNormals[i].x;
	        vector.y = mesh->mNormals[i].y;
	        vector.z = mesh->mNormals[i].z;
	        vertex.Normal = vector;

	        if (mesh->mTextureCoords[0])
	        {
	            glm::vec2 vec;
	            vec.x = mesh->mTextureCoords[0][i].x;
	            vec.y = mesh->mTextureCoords[0][i].y;
	            vertex.TexCoords = vec;
	        }
	        else
	        {
	            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
	        }
	        vertices.push_back(vertex);
	    }

	    if(mesh->mNumBones > 0)
	        ExtractBoneWeightForVertices(vertices, mesh, scene);

	    for (GLuint i = 0; i < mesh->mNumFaces; i++)
	    {
	        aiFace face = mesh->mFaces[i];
	        for (GLuint j = 0; j < face.mNumIndices; j++)
	        {
	            indices.push_back(face.mIndices[j]);
	        }
	    }

	    if (mesh->mMaterialIndex >= 0)
	    {
	        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	        vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	        vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	    }

	    // Ahora nodeName ya existe aquí
	    return Mesh(vertices, indices, textures, nodeName);
	}
	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;

		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			GLboolean skip = false;

			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)

					break;
				}
			}

			if (!skip)
			{   // If texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);

				this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}

		return textures;
	}
};

GLint TextureFromFile(const char *path, string directory)
{
	//Generate texture ID and load texture data
	string filename = string(path);
	filename = directory + '/' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;

	unsigned char *image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureID;
}