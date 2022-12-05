#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;

out vec4 fragColor;

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized
    vec3 N3 = normalize(worldNormal);
    vec4 N = vec4(N3, 0.0);

    fragColor = vec4(abs(N));
}
