#version 450
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 outColor;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0, scalar) buffer GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int lightCount;
    vec3 sunDirection;
    float sunSize;
    float sunIntensity;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

const float PI = 3.1415926538;

vec3 gammaCorrect(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    float dist = length(fragOffset);
    if (dist >= 1.0)
        discard;

    float intensity = 0.5 * (cos(dist * PI) + 1.0);
    outColor = vec4(gammaCorrect(push.color.rgb), intensity);
}