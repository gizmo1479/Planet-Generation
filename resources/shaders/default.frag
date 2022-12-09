#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;

uniform int shader;
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

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized
    vec3 N3 = normalize(worldNormal);
    vec4 N = vec4(N3, 0.0);

   // fragColor = vec4(abs(N));
    if (shader == SHADER_PHONG) phong();
    if (shader == SHADER_TOON) toonColor();
}
