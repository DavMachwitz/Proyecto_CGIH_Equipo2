#version 330 core
out vec4 outColor;

in vec3 ourColor;

void main()
{
    outColor = vec4(ourColor, 1.0f);
}