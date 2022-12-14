#version 330 core

layout(location = 0) in vec3 objectPosition;
layout(location = 1) in vec2 uv;

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 lightColor;

uniform mat4 modelMatrix;
uniform mat4 MVPMatrix;

uniform sampler2D globe;
uniform sampler2D height_map;

uniform int shaderType;

float height_offset;
void selectColor() {
    float water = 0.0f;
    float grass = .05f;
    float grass2 = .07f;
    float sky = 0.15f;

    if (height_offset <= water) {
        lightColor = vec4(.2f, .2f, .9f, 1.0);
        if (shaderType == 1) lightColor = vec4(.35f, .48f, 1.f, 1.0);
    } else if ((height_offset > water) && (height_offset <= grass)) {
        lightColor = vec4(0.0f, .8f, 0.0f, 1.0);
        if (shaderType == 1) lightColor = vec4(.35f, 1.f, .35f, 1.f); // .47f
    } else if (height_offset <= grass2 && height_offset > grass) {
        lightColor = vec4(0.0f, .6f, 0.0f, 1.0);
        if (shaderType == 1) lightColor = vec4(.35f, .8f, .35f, 1.f); // .47f
        //lightColor = vec4(0.0f, .6f, 0.0f, 1.0);
    } else if (height_offset <= sky && height_offset > grass2) {
        lightColor = vec4(.57f, .7f, .7f, 1.0);
    }
    else {
        lightColor = vec4(.9f, .9f, .9f, 1.0);
    }
}

void main() {
    vec4 objPos4 = vec4(objectPosition, 1);
    vec4 N4 = normalize(vec4(objectPosition, 0));
    vec4 finalPos = objPos4;

    // check color; if blue, don't offset objpos
    vec4 globe_color = texture(globe, uv);
    
//    if (globe_color.g != 0.0f) {
    height_offset = 0;
    if (globe_color.r != 0.0f) {          // mountains!
        vec4 offset = .3*texture2D(height_map, uv).r * N4;
        finalPos = objPos4 + offset; // change this somehow
        height_offset = length(offset);
    }
    else if (globe_color.g != 0.0f) {     // flatlands!
        vec4 offset = .1 * texture2D(height_map, uv).r * N4;
        finalPos = objPos4 + offset; // change this somehow
        height_offset = length(offset);
    }

    vec4 worldPos4 = modelMatrix * finalPos;
    worldPosition = worldPos4.xyz;
    worldNormal = normalize(finalPos.xyz);
    selectColor();
    gl_Position = MVPMatrix * finalPos;

}
