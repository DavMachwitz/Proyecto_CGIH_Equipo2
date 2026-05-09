#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 inNormal;
//layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoords;
layout (location = 5) in ivec4 bone_ids;
layout (location = 6) in vec4 weights;

//out vec3 Color;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];

void main()
{
    mat4 bone_transform = bones[bone_ids[0]] * weights[0];
		bone_transform += bones[bone_ids[1]] * weights[1];
		bone_transform += bones[bone_ids[2]] * weights[2];
		bone_transform += bones[bone_ids[3]] * weights[3];
		
	vec4 boned_position = bone_transform * vec4(position, 1.0); // transformed by bones

    FragPos = vec3(model * bone_transform);

    Normal = mat3(transpose(inverse(model))) * inNormal; // Calcular normales fuera del shader 
	//Normal = vec3 (bone_transform * vec4 (inColor, 0.0)); // A lo mejor se cambia
    gl_Position = projection * view * vec4(FragPos, 1.0); //model; //vec4(position, 1.0f);
    //Color=inColor;
    TexCoords=vec2(inTexCoords.x,inTexCoords.y);
}