#version 330 core

layout(location = 0) in vec3 objectPosition;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);
    vec4 worldPos4 = modelMatrix * objPos4;
    worldPosition = worldPos4.xyz;
    worldNormal = transpose(inverse(mat3(modelMatrix))) * normalize(objectPosition.xyz);

    gl_Position = projectionMatrix * (viewMatrix * (modelMatrix * objPos4));
}
