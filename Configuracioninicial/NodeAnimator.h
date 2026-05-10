#pragma once
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include "NodeAnimation.h"

class NodeAnimator
{
public:
    NodeAnimator(NodeAnimation* animation) {
        m_CurrentTime = 0.0f;
        m_Animation = animation;
    }

    void UpdateAnimation(float dt) {
        if (m_Animation) {
            m_CurrentTime += m_Animation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_Animation->GetDuration());
            CalculateFinalTransforms(&m_Animation->GetRootNode(), glm::mat4(1.0f));
        }
    }
    void SetTime(float time) {
        m_CurrentTime = time;
        CalculateFinalTransforms(&m_Animation->GetRootNode(), glm::mat4(1.0f));
    }
    std::map<std::string, glm::mat4> GetFinalTransforms() { return m_FinalTransforms; }

private:
    void CalculateFinalTransforms(const NodeData* node, glm::mat4 parentTransform) {
        glm::mat4 nodeTransform = node->transformation;

        Bone* bone = m_Animation->FindBone(node->name);
        if (bone) {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransform = parentTransform * nodeTransform;
        m_FinalTransforms[node->name] = globalTransform;

        for (const auto& child : node->children) {
            CalculateFinalTransforms(&child, globalTransform);
        }
    }

    NodeAnimation* m_Animation;
    float m_CurrentTime;
    std::map<std::string, glm::mat4> m_FinalTransforms;
};