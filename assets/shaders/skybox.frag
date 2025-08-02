#version 450
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in vec3 localDir;
layout(location = 0) out vec4 fragColor;

layout(push_constant, scalar) uniform PC {
    mat4 proj;
    mat4 view;
    vec3 sunDirection;
    float sunSize;   
    bool sunVisible;
} pc;

layout(set = 0, binding = 1) uniform sampler2D Sampler2D[];

float getSunIntensity(vec3 viewDir, vec3 sunDir, float sunSize) {
    float dotVal = dot(viewDir, sunDir);
    float sunFade = smoothstep(cos(sunSize), cos(sunSize * 0.5), dotVal);
    return sunFade;
}

void main() {
    vec3 viewDir = normalize(localDir);

    float t = clamp(viewDir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 baseSky = mix(vec3(0.6, 0.8, 1.0), vec3(0.1, 0.4, 0.8), t);

    float sunIntensity = getSunIntensity(viewDir, normalize(pc.sunDirection), pc.sunSize);
    vec3 sunColor = vec3(1.0, 0.95, 0.85) * sunIntensity;

    vec3 finalColor = baseSky;
    if (pc.sunVisible)
        finalColor = finalColor + sunColor;

    fragColor = vec4(finalColor, 1.0);
}