#version 330 core
out vec4 FragColor;

in vec3 FragPos; 
uniform vec3 color; 

// --- VARIABLES DEL REFLECTOR ---
uniform bool spotLightOn;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform float spotCutOff;
uniform float spotOuterCutOff;
uniform vec3 spotLightColor;

void main()
{
    vec3 finalColor = color; 

    if(spotLightOn)
    {
        vec3 lightDir = normalize(spotLightPos - FragPos);
        float theta = dot(lightDir, normalize(-spotLightDir));
        float epsilon = spotCutOff - spotOuterCutOff;
        float intensity = clamp((theta - spotOuterCutOff) / epsilon, 0.0, 1.0);

        float distance = length(spotLightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * (distance * distance));

        vec3 spotContribution = spotLightColor * intensity * attenuation * 2.0;
        finalColor += spotContribution;
    }

    FragColor = vec4(finalColor, 1.0f);
}