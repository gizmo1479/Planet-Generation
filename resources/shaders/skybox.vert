#version 330 core

layout (location = 0) in vec3 objPos;

out vec3 texCoords;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    texCoords = objPos;
    vec4 pos = proj * view * vec4(objPos, 1.0);
    gl_Position = pos.xyww;
}
