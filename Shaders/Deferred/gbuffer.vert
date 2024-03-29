#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

void main()
{
    TexCoords = aTexCoords;

    Normal = mat3(transpose(inverse(mMatrix))) * aNormal;

    vec4 worldPos = mMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    gl_Position = pMatrix * vMatrix * worldPos;
}
