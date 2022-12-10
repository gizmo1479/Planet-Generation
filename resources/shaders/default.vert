#version 330 core

layout(location = 0) in vec3 objectPosition;
layout(location = 2) in vec2 uv;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform sampler2D height_map;

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);
    vec4 N4 = normalize(vec4(objectPosition, 0));

    // offset object position by height obtained from height map
//    vec4 offset = texture2D(height_map, uv).r * N4;
    vec4 offset = vec4(1,0,0,0);
    vec4 finalPos = objPos4 + offset;

//    vec4 worldPos4 = modelMatrix * objPos4;
    vec4 worldPos4 = modelMatrix * finalPos;
    worldPosition = worldPos4.xyz;
    worldNormal = transpose(inverse(mat3(modelMatrix))) * normalize(N4.xyz);

    if ((objectPosition.y <= 0.0005f) && (objectPosition.y >= -0.0005f)) {
        gl_Position = projectionMatrix * (viewMatrix * (modelMatrix * finalPos));
    } else {
        gl_Position = projectionMatrix * (viewMatrix * (modelMatrix * objPos4));
    }
    gl_Position = projectionMatrix * (viewMatrix * (modelMatrix * objPos4));

    // TODO: optimise to input MVP matrix instead of computing it here
    gl_Position = projectionMatrix * (viewMatrix * (modelMatrix * finalPos));
}
