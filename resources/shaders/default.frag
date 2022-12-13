#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in float height_offset;
//in vec4 color;

uniform int shaderType;
uniform vec3 cameraPos;

//Shader enums
const int SHADER_PHONG = 0;
const int SHADER_TOON = 1;

// global phong vars
const float kd = .5;
const float ks = .5;

const vec3 lightPos = vec3(4, -3, 0);
vec4 lightColor = vec4(.5, .5, 1.f, 1.0);

out vec4 fragColor;

void toonColor() {
    vec3 lightDir = normalize(lightPos - worldPosition);
    float intensity = clamp(dot(lightDir, normalize(worldNormal)), 0.f, 1.f);
    //float diffuse = smoothstep(0.f, fwidth(intensity), intensity); //
    float diffuse = clamp(floor(intensity * 4) / 4, .15, 1.f);

    vec3 R = reflect(-lightDir, normalize(worldNormal));
    vec3 toCam = normalize(cameraPos - worldPosition);
    float spec = pow(clamp(dot(toCam, R), 0.f, 1.f), 25);
    spec = floor(spec*2) / 2;

    fragColor += (diffuse + spec)*lightColor;
}

void toonColor2() {
    vec3 lightDir = normalize(vec3(4, 3, 6) - worldPosition);
    float intensity = clamp(dot(lightDir, normalize(worldNormal)), 0.f, 1.f);
    //float diffuse = smoothstep(0.f, fwidth(intensity), intensity); //
    float diffuse = clamp(floor(intensity * 4) / 4, .15, 1.f);

    vec3 R = reflect(-lightDir, normalize(worldNormal));
    vec3 toCam = normalize(cameraPos - worldPosition);
    float spec = pow(clamp(dot(toCam, R), 0.f, 1.f), 25);
    spec = floor(spec*2) / 2;

    vec4 c = (diffuse + spec)*lightColor;

    if (c.x > fragColor.x || c.y > fragColor.y)
        fragColor = (diffuse + spec)*lightColor;
}

void phong() {
    vec3 lightDir = normalize(lightPos - worldPosition);
    float d = clamp(dot(normalize(worldNormal), lightDir), 0.f, 1.f);
    fragColor += d*lightColor*kd;

    vec3 R = reflect(-lightDir, normalize(worldNormal));
    vec3 toCam = normalize(cameraPos - worldPosition);
    float spec = pow(clamp(dot(toCam, R), 0.f, 1.f), 25);
    fragColor += ks*spec*lightColor;
}

void phong2() {
    vec3 lightDir = normalize(vec3(4, 3, 6) - worldPosition);
    float d = clamp(dot(normalize(worldNormal), lightDir), 0.f, 1.f);
    fragColor += d*lightColor*kd;

    vec3 R = reflect(-lightDir, normalize(worldNormal));
    vec3 toCam = normalize(cameraPos - worldPosition);
    float spec = pow(clamp(dot(toCam, R), 0.f, 1.f), 25);
    fragColor += ks*spec*lightColor;
}

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized
    vec3 N3 = normalize(worldNormal);
    vec4 N = vec4(N3, 0.0);

    float water = 0.0f;
    float sky = 0.04f;

    if (height_offset <= water) {
        lightColor = vec4(0.0f, 0.0f, 1.0f, 1.0);
    } else if ((height_offset > water) && (height_offset < sky)) {
        lightColor = vec4(0.0f, 1.0f, 0.0f, 1.0);
    } else {
        lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0);
    }

//    fragColor = N;
//    fragColor = vec4(1);
//    fragColor = color;
    if (shaderType == SHADER_PHONG) phong();
    if (shaderType == SHADER_TOON) toonColor();

}
