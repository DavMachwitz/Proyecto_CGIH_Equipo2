#pragma once
#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Bone.h"
#include "AssimpGLMHelper.h"

// -----------------------------------------------------------------------
// BoneInfo: lo que Model.h necesita guardar por cada hueso
// -----------------------------------------------------------------------
struct BoneInfo
{
    int id;             // índice en el array finalBonesMatrices del shader
    glm::mat4 offset;   // aiMesh->mBones[i]->mOffsetMatrix convertida
};

// -----------------------------------------------------------------------
// Nodo del árbol de jerarquía de huesos
// -----------------------------------------------------------------------
struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

// -----------------------------------------------------------------------
// Animation: representa UNA animación leída de un aiAnimation
// -----------------------------------------------------------------------
class ModelAnimation
{
public:
    ModelAnimation() = default;

    ModelAnimation(const std::string& animationPath, 
              std::map<std::string, BoneInfo>& boneInfoMap,
              int& boneCount)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath,
            aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || !scene->mRootNode) {
            std::cout << "ERROR::Animation: no se pudo cargar " << animationPath << std::endl;
            return;
        }
        if (scene->mNumAnimations == 0) {
            std::cout << "ERROR::Animation: el archivo no contiene animaciones." << std::endl;
            return;
        }

        // Tomar la primera animación (index 0)
        aiAnimation* animation = scene->mAnimations[0];
        m_Duration    = (float)animation->mDuration;
        m_TicksPerSec = (int)(animation->mTicksPerSecond != 0
                              ? animation->mTicksPerSecond : 25.0);

        ReadHierarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, boneInfoMap, boneCount);
    }

    ~ModelAnimation() {}

    Bone* FindBone(const std::string& name)
    {
        for (auto& b : m_Bones)
            if (b.GetBoneName() == name)
                return &b;
        return nullptr;
    }

    float                  GetTicksPerSecond()  const { return (float)m_TicksPerSec; }
    float                  GetDuration()        const { return m_Duration; }
    const AssimpNodeData&  GetRootNode()              { return m_RootNode; }
    const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
    float    m_Duration    = 0.0f;
    int      m_TicksPerSec = 25;
    std::vector<Bone> m_Bones;
    AssimpNodeData    m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;

    // Lee la jerarquía del nodo raíz de la escena
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        dest.name           = src->mName.data;
        dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount  = src->mNumChildren;
        for (unsigned int i = 0; i < src->mNumChildren; i++) {
            AssimpNodeData child;
            ReadHierarchyData(child, src->mChildren[i]);
            dest.children.push_back(child);
        }
    }

    // Crea objetos Bone para los canales que no estaban en el Model
    void ReadMissingBones(const aiAnimation* animation,
                          std::map<std::string, BoneInfo>& boneInfoMap,
                          int& boneCount)
    {
        int size = animation->mNumChannels;
        for (int i = 0; i < size; i++) {
            aiNodeAnim* channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                BoneInfo bi;
                bi.id = boneCount;
                bi.offset = glm::mat4(1.0f);
                boneInfoMap[boneName] = bi;
                boneCount++;
            }
            m_Bones.emplace_back(boneName, boneInfoMap[boneName].id, channel);
        }
        m_BoneInfoMap = boneInfoMap;
    }
};
