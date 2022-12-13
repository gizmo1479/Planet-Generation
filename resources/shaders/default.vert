#version 330 core

layout(location = 0) in vec3 objectPosition;
layout(location = 1) in vec2 uv;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat4 MVPMatrix;

uniform sampler2D height_map;

out vec4 height_offset;

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);
    vec4 N4 = normalize(vec4(objectPosition, 0));

    // offset object position by height obtained from height map
    vec4 offset = 0.05f * texture2D(height_map, uv).r * N4;
    vec4 finalPos = objPos4 + offset;

    height_offset = offset;

    vec4 worldPos4 = modelMatrix * finalPos;
    worldPosition = worldPos4.xyz;
    worldNormal = normalize(finalPos.xyz);

    gl_Position = MVPMatrix * finalPos;
}
