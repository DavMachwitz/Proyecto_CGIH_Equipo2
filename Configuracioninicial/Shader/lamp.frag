#version 330 core

// =====================================================================
//  lamp.frag (geometria procedural: piso, edificios, columnas)
//  Soporta hasta 16 spotlights simultaneos.
// =====================================================================

#define MAX_SPOTLIGHTS 16

in vec3 FragPos;

out vec4 FragColor;

uniform vec3 color;

uniform bool  spotLightOn;
uniform int   numSpotLights;
uniform vec3  spotLightPos[MAX_SPOTLIGHTS];
uniform vec3  spotLightDir;
uniform float spotCutOff;
uniform float spotOuterCutOff;
uniform vec3  spotLightColor;

void main()
{
    vec3 baseColor = color;
    vec3 result = baseColor;

    if (spotLightOn)
    {
        for (int i = 0; i < numSpotLights; i++)
        {
            vec3 toLight = normalize(spotLightPos[i] - FragPos);
            float theta = dot(toLight, normalize(-spotLightDir));

            float epsilon   = spotCutOff - spotOuterCutOff;
            float intensity = clamp((theta - spotOuterCutOff) / epsilon, 0.0, 1.0);

            float dist = length(spotLightPos[i] - FragPos);
            float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

            result += baseColor * spotLightColor * intensity * attenuation;
        }
    }

    FragColor = vec4(result, 1.0);
}
