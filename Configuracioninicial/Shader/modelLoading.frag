#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos; 

uniform sampler2D texture_diffuse1;
uniform vec3 tintColor; 

// --- VARIABLES DEL REFLECTOR ---
uniform bool spotLightOn;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform float spotCutOff;
uniform float spotOuterCutOff;
uniform vec3 spotLightColor;

void main()
{    
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    
    // Transparencia obligatoria para que la reja se vea bien
    if(texColor.a < 0.1)
        discard;
        
    vec3 finalColor = texColor.rgb * tintColor;

    if(spotLightOn)
    {
        vec3 lightDir = normalize(spotLightPos - FragPos);
        float theta = dot(lightDir, normalize(-spotLightDir)); 
        float epsilon = spotCutOff - spotOuterCutOff;
        float intensity = clamp((theta - spotOuterCutOff) / epsilon, 0.0, 1.0);

        float distance = length(spotLightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * (distance * distance));

        vec3 spotContribution = spotLightColor * texColor.rgb * intensity * attenuation * 3.0; 
        finalColor += spotContribution;
    }

    FragColor = vec4(finalColor, texColor.a);
}