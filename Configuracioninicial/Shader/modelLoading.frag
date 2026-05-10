#version 330 core

// =====================================================================
//  modelLoading.frag (modelos .obj con textura)
//  Soporta hasta 16 spotlights simultaneos.
//  CAMBIO IMPORTANTE: spotLightColor ahora es un ARREGLO (uno por luz)
//  para soportar el modo fiesta donde cada luz tiene un color distinto.
// =====================================================================

#define MAX_SPOTLIGHTS 16

in vec2 TexCoords;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform vec3 tintColor;

uniform bool  spotLightOn;
uniform int   numSpotLights;
uniform vec3  spotLightPos[MAX_SPOTLIGHTS];
uniform vec3  spotLightDir;
uniform float spotCutOff;
uniform float spotOuterCutOff;
uniform vec3  spotLightColor[MAX_SPOTLIGHTS];   // <-- ahora es arreglo

void main()
{
    vec4 texColor  = texture(texture_diffuse1, TexCoords);
    vec3 baseColor = texColor.rgb * tintColor;
    vec3 result    = baseColor;

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

            // Cada luz aporta SU color (modo fiesta = colores distintos)
            result += texColor.rgb * spotLightColor[i] * intensity * attenuation;
        }
    }

    FragColor = vec4(result, texColor.a);
}
