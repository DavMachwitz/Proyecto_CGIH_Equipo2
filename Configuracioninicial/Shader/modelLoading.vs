#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in ivec4 boneIds;
layout (location = 6) in vec4  weights;

out vec2 TexCoords;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 finalBonesMatrices[100];
uniform bool useSkinning;   // ← NUEVO

void main()
{
    vec4 totalPosition;

    if (useSkinning)   // ← solo para modelos animados
    {
        totalPosition = vec4(0.0);
        for (int i = 0; i < 4; i++)
        {
            if (boneIds[i] == -1) continue;
            if (boneIds[i] >= 100) { totalPosition = vec4(aPos, 1.0); break; }
            totalPosition += finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0) * weights[i];
        }
    }
    else
    {
        totalPosition = vec4(aPos, 1.0);   // ← modelos estáticos sin cambio
    }

    FragPos     = vec3(model * totalPosition);
    TexCoords   = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}