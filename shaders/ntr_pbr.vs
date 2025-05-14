#version 460 core
layout (location = 0) in vec3   aPos;
layout (location = 1) in vec3   aNormal;
layout (location = 2) in vec2   aTexCoords;
layout (location = 3) in vec3   aTangent;
layout (location = 4) in vec3   aBitangent;
layout (location = 5) in ivec4  aBoneIDs;
layout (location = 6) in vec4   aWeights;

layout (location = 7) in mat4   aModel;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool instancing = false;

void main()
{
    mat4 effectiveModel = instancing ? aModel : model;

    TexCoords   = aTexCoords;
    WorldPos    = vec3(effectiveModel * vec4(aPos, 1.0));
    Normal      = transpose(inverse(mat3(effectiveModel))) * aNormal;

    FragPos     = vec3(effectiveModel * vec4(aPos, 1.0));

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}