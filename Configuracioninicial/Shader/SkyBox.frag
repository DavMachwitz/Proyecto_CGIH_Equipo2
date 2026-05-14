#version 330 core
in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;
uniform vec3 skyTint;   // Color del cielo (ej. Blanco de día, naranja en la tarde, azul oscuro de noche)
uniform float intensity; // Para oscurecerlo en la noche

void main()
{
    vec4 texColor = texture(skybox, TexCoords);
    // Multiplicamos la textura original por el tinte y la intensidad actual
    color = vec4(texColor.rgb * skyTint * intensity, 1.0);
}