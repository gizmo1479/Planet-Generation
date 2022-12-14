#version 330 core

layout(location = 0) in vec3 objectPosition;
layout(location = 1) in vec2 uv;

out vec3 worldPosition;
out vec3 worldNormal;
out float height_offset;
out vec4 lightColor;
//out vec4 color;

uniform mat4 modelMatrix;
uniform mat4 MVPMatrix;

uniform sampler2D globe;
uniform sampler2D height_map;

uniform int shaderType;

void selectColor() {
    float water = 0.0f;
    float sky = 0.05f;

    if (height_offset <= water) {
        lightColor = vec4(.2f, .2f, .9f, 1.0);
        if (shaderType == 1) lightColor = vec4(.35f, .48f, 1.f, 1.0);
    } else if ((height_offset > water) && (height_offset < sky)) {
        lightColor = vec4(0.0f, .8f, 0.0f, 1.0);
        if (shaderType == 1) lightColor = vec4(.35f, 1.f, .35f, 1.f); // .47f
    } else {
        lightColor = vec4(.9f, .9f, .9f, 1.0);
    }
}

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);
    vec4 N4 = normalize(vec4(objectPosition, 0));

    vec4 finalPos = objPos4;

    // check color; if blue, don't offset objpos
    vec4 globe_color = texture(globe, uv);
    // offset object position by height obtained from height map
    // vec4 offset = texture(height_map, uv).r * N4;
    vec4 offset = 0.15f * texture2D(height_map, uv).r * N4;
    height_offset = 0.0f;
    
    if (globe_color.g != 0.0f) {
        finalPos = objPos4 + offset;
        height_offset = length(offset);
    }

    vec4 worldPos4 = modelMatrix * finalPos;
    worldPosition = worldPos4.xyz;
    worldNormal = normalize(finalPos.xyz);
    selectColor();
    gl_Position = MVPMatrix * finalPos;

}
