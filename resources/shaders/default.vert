#version 330 core

layout(location = 0) in vec3 objectPosition;
layout(location = 1) in vec2 uv;

out vec3 worldPosition;
out vec3 worldNormal;
//out vec4 color;

uniform mat4 modelMatrix;
uniform mat4 MVPMatrix;

uniform sampler2D globe;
uniform sampler2D height_map;

out vec4 height_offset;

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);
    vec4 N4 = normalize(vec4(objectPosition, 0));

    vec4 finalPos = objPos4;

    // check color; if blue, don't offset objpos
    vec4 globe_color = texture(globe, uv);
    // offset object position by height obtained from height map
    // vec4 offset = texture(height_map, uv).r * N4;
    vec4 offset = texture2D(height_map, uv).r * N4;
    if (globe_color.r != 0.0f) {          // mountains!
        offset = 0.1 * offset;
        finalPos = objPos4 + offset; // change this somehow
    }
    else if (globe_color.g != 0.0f) {     // flatlands!
        offset = 0.05f * offset;
        finalPos = objPos4 + offset;
    }

    height_offset = offset;

    vec4 worldPos4 = modelMatrix * finalPos;
    worldPosition = worldPos4.xyz;
    worldNormal = normalize(finalPos.xyz);
//    color = globe_color;
    gl_Position = MVPMatrix * finalPos;
}
