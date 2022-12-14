#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
//in vec4 color;

uniform int shaderType;
uniform vec3 cameraPos;

//Shader enums
const int SHADER_PHONG = 0;
const int SHADER_TOON = 1;

// global phong vars
const float ka = .5;
const float kd = .7f;
const float ks = .5;


in vec4 lightColor;
vec4 ambient = ka*lightColor;

out vec4 fragColor;

void toonColor(vec3 lightPos, vec4 lightColor) {
    vec3 lightDir = normalize(cameraPos + vec3(1, 1, 1) - worldPosition); //normalize(lightPos - worldPosition);
    float intensity = clamp(dot(lightDir, normalize(worldNormal)), 0.f, 1.f);
    //float diffuse = smoothstep(0.f, fwidth(intensity), intensity); //
    float diffuse = kd*clamp(floor(intensity * 5) / 5, .15, 1.f);

    vec3 R = reflect(-lightDir, normalize(worldNormal));
    vec3 toCam = normalize(cameraPos - worldPosition);
    float spec = pow(clamp(dot(toCam, R), 0.f, 1.f), 25);
    spec = floor(spec*2) / 2;

    vec4 c = diffuse*lightColor+ambient;

    if (c.x > fragColor.x)
        fragColor.x = c.x;
    if (c.y > fragColor.y)
        fragColor.y = c.y;
    if (c.z > fragColor.z)
        fragColor.z = c.z;
}

void toon() {
    toonColor(vec3(5, 0, 0), lightColor);
//    toonColor(vec3(-5, 0, 0), lightColor);
//    toonColor(vec3(0, 0, 5), lightColor);
//    toonColor(vec3(0, 0, -5), lightColor);
}


void phongColor(vec3 lightPos, vec4 lightColor) {
    fragColor += ambient;

    vec3 lightDir = normalize(cameraPos - worldPosition);
    float d = clamp(dot(normalize(worldNormal), lightDir), 0.f, 1.f);
    fragColor += d*lightColor*kd;

//    vec3 R = reflect(-lightDir, normalize(worldNormal));
//    vec3 toCam = normalize(cameraPos - worldPosition);
//    float spec = pow(clamp(dot(toCam, R), 0.f, 1.f), 25);
//    fragColor += ks*spec*lightColor;
}

void phong() {
    phongColor(vec3(5, 0, 0), lightColor);
//    phongColor(vec3(-5, 0, 0), lightColor);
//    phongColor(vec3(0, 0, 5), lightColor);
//    phongColor(vec3(0, 0, -5), lightColor);
//    phongColor(vec3(0, 5, 0), lightColor);
//    phongColor(vec3(0, -5, 0), lightColor);
}

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized
    vec3 N3 = normalize(worldNormal);
    vec4 N = vec4(N3, 0.0);

    float water = 0.0f;
    float sky = 0.04f;

//    if (height_offset <= water) {
//        lightColor = vec4(.2f, .2f, .9f, 1.0);
//    } else if ((height_offset > water) && (height_offset < sky)) {
//        lightColor = vec4(0.0f, .8f, 0.0f, 1.0);
//    } else {
//        lightColor = vec4(.9f, .9f, .9f, 1.0);
//    }

    if (shaderType == SHADER_PHONG) phong();
    if (shaderType == SHADER_TOON) toon();

}
