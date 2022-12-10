#version 330 core

layout(location = 0) in vec3 objectPosition;
layout(location = 2) in vec2 uv;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform sampler2D texture_sampler;

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);

    // offset object position by height obtained from height map
    vec4 offset = texture(texture_sampler, uv) * normalize(objPos4);
    vec4 finalPos = objPos4 + offset;

//    vec4 worldPos4 = modelMatrix * objPos4;
    vec4 worldPos4 = modelMatrix * finalPos;
    worldPosition = worldPos4.xyz;
    worldNormal = transpose(inverse(mat3(modelMatrix))) * normalize(objectPosition.xyz);

    gl_Position = projectionMatrix * (viewMatrix * (modelMatrix * objPos4));
}
