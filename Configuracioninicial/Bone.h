#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>
#include "AssimpGLMHelper.h"

// -----------------------------------------------------------------------
// Estructuras de keyframe
// -----------------------------------------------------------------------
struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

// -----------------------------------------------------------------------
// Bone: lee los keyframes de un aiNodeAnim y calcula la matriz local
// interpolada en cualquier instante de animación.
// -----------------------------------------------------------------------
class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;
        for (int i = 0; i < m_NumPositions; i++) {
            KeyPosition data;
            data.position  = GetGLMVec(channel->mPositionKeys[i].mValue);
            data.timeStamp = (float)channel->mPositionKeys[i].mTime;
            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (int i = 0; i < m_NumRotations; i++) {
            KeyRotation data;
            data.orientation = GetGLMQuat(channel->mRotationKeys[i].mValue);
            data.timeStamp   = (float)channel->mRotationKeys[i].mTime;
            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (int i = 0; i < m_NumScalings; i++) {
            KeyScale data;
            data.scale     = GetGLMVec(channel->mScalingKeys[i].mValue);
            data.timeStamp = (float)channel->mScalingKeys[i].mTime;
            m_Scales.push_back(data);
        }
    }

    // Calcula m_LocalTransform para el tiempo animationTime
    void Update(float animationTime)
    {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation    = InterpolateRotation(animationTime);
        glm::mat4 scale       = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4   GetLocalTransform() const { return m_LocalTransform; }
    std::string GetBoneName()       const { return m_Name; }
    int         GetBoneID()         const { return m_ID; }

private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale>    m_Scales;
    int m_NumPositions = 0, m_NumRotations = 0, m_NumScalings = 0;

    glm::mat4   m_LocalTransform;
    std::string m_Name;
    int         m_ID;

    // ----------------------------------------------------------------
    // Factor de interpolación entre dos keyframes consecutivos
    // ----------------------------------------------------------------
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff   = nextTimeStamp  - lastTimeStamp;
        return midWayLength / framesDiff;
    }

    // ----------------------------------------------------------------
    // Posición (LERP)
    // ----------------------------------------------------------------
    glm::mat4 InterpolatePosition(float animationTime)
    {
        if (m_NumPositions == 1)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0 = GetPositionIndex(animationTime);
        int p1 = p0 + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0].timeStamp,
                                           m_Positions[p1].timeStamp, animationTime);
        glm::vec3 finalPos = glm::mix(m_Positions[p0].position,
                                      m_Positions[p1].position, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPos);
    }

    // ----------------------------------------------------------------
    // Rotación (SLERP)
    // ----------------------------------------------------------------
    glm::mat4 InterpolateRotation(float animationTime)
    {
        if (m_NumRotations == 1) {
            auto rot = glm::normalize(m_Rotations[0].orientation);
            return glm::toMat4(rot);
        }
        int p0 = GetRotationIndex(animationTime);
        int p1 = p0 + 1;
        float scaleFactor = GetScaleFactor(m_Rotations[p0].timeStamp,
                                           m_Rotations[p1].timeStamp, animationTime);
        glm::quat finalRot = glm::slerp(m_Rotations[p0].orientation,
                                         m_Rotations[p1].orientation, scaleFactor);
        return glm::toMat4(glm::normalize(finalRot));
    }

    // ----------------------------------------------------------------
    // Escala (LERP)
    // ----------------------------------------------------------------
    glm::mat4 InterpolateScaling(float animationTime)
    {
        if (m_NumScalings == 1)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int p0 = GetScaleIndex(animationTime);
        int p1 = p0 + 1;
        float scaleFactor = GetScaleFactor(m_Scales[p0].timeStamp,
                                           m_Scales[p1].timeStamp, animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[p0].scale, m_Scales[p1].scale, scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    // ----------------------------------------------------------------
    // Búsqueda del keyframe anterior al tiempo dado
    // ----------------------------------------------------------------
    int GetPositionIndex(float animationTime)
    {
        for (int i = 0; i < m_NumPositions - 1; i++)
            if (animationTime < m_Positions[i + 1].timeStamp) return i;
        return 0;
    }
    int GetRotationIndex(float animationTime)
    {
        for (int i = 0; i < m_NumRotations - 1; i++)
            if (animationTime < m_Rotations[i + 1].timeStamp) return i;
        return 0;
    }
    int GetScaleIndex(float animationTime)
    {
        for (int i = 0; i < m_NumScalings - 1; i++)
            if (animationTime < m_Scales[i + 1].timeStamp) return i;
        return 0;
    }
};
