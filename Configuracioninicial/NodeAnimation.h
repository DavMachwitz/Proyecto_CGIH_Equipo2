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

// Estructura simplificada para los nodos del stand
struct NodeData
{
    glm::mat4 transformation;
    std::string name;
    std::vector<NodeData> children;
};

class NodeAnimation
{
public:
    NodeAnimation(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GlobalScale);

        if (!scene || !scene->mAnimations) return;

        auto animation = scene->mAnimations[0];
        m_Duration = (float)animation->mDuration;
        m_TicksPerSecond = (float)animation->mTicksPerSecond;

        // Leer la jerarquía pura
        ReadHierarchyData(m_RootNode, scene->mRootNode);
        
        // Cargar los canales de animación (movimientos de cada pieza)
        for (unsigned int i = 0; i < animation->mNumChannels; i++) {
            auto channel = animation->mChannels[i];
            m_Bones.emplace_back(channel->mNodeName.data, i, channel);
        }
    }

    Bone* FindBone(const std::string& name) {
        for (auto& bone : m_Bones) {
            if (bone.GetBoneName() == name) return &bone;
        }
        return nullptr;
    }

    float GetTicksPerSecond() { return m_TicksPerSecond; }
    float GetDuration() { return m_Duration; }
    const NodeData& GetRootNode() { return m_RootNode; }

private:
    void ReadHierarchyData(NodeData& dest, const aiNode* src) {
        dest.name = src->mName.data;
        dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
        for (unsigned int i = 0; i < src->mNumChildren; i++) {
            NodeData child;
            ReadHierarchyData(child, src->mChildren[i]);
            dest.children.push_back(child);
        }
    }

    float m_Duration;
    float m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    NodeData m_RootNode;
};