#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "ModelAnimation.h"

// -----------------------------------------------------------------------
// Animator
//   - Avanza el tiempo de la animación en cada frame (Update)
//   - Recorre el árbol de huesos (CalculateBoneTransform)
//   - Expone el array m_FinalBoneMatrices para enviarlo al shader
// -----------------------------------------------------------------------
class Animator
{
public:
    Animator(ModelAnimation* animation)
    {
        m_CurrentTime   = 0.0f;
        m_CurrentAnim   = animation;
        // Inicializar con identidad — máximo 100 huesos (igual que el shader)
        m_FinalBoneMatrices.reserve(100);
        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    // Llamar UNA VEZ por frame, antes de dibujar el modelo
    void UpdateAnimation(float dt)
    {
        if (!m_CurrentAnim) return;

        m_CurrentTime += m_CurrentAnim->GetTicksPerSecond() * dt;
        // Loop automático
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnim->GetDuration());

        CalculateBoneTransform(&m_CurrentAnim->GetRootNode(), glm::mat4(1.0f));
    }

    // Cambiar animación en tiempo real (para múltiples clips)
    void PlayAnimation(ModelAnimation* animation)
    {
        m_CurrentAnim = animation;
        m_CurrentTime = 0.0f;
    }

    // El array que se sube al uniform del shader
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const
    {
        return m_FinalBoneMatrices;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    ModelAnimation*             m_CurrentAnim;
    float                  m_CurrentTime;

    // Recorre el árbol recursivamente, acumulando la transformación global
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName  = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_CurrentAnim->FindBone(nodeName);
        if (bone) {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransform = parentTransform * nodeTransform;

        auto& boneInfoMap = m_CurrentAnim->GetBoneIDMap();
        auto  it = boneInfoMap.find(nodeName);
        if (it != boneInfoMap.end()) {
            int   index        = it->second.id;
            glm::mat4 offset   = it->second.offset;
            if (index < 100)
                m_FinalBoneMatrices[index] = globalTransform * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransform);
    }
};
