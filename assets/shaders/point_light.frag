#version 450

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) buffer GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int lightCount;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

const float M_PI = 3.1415926538;

vec3 gamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    float dist = sqrt(dot(fragOffset, fragOffset));

    if (dist >= 1.0) {
        discard;
    }

    outColor = vec4(gamma(push.color.xyz), 0.5 * (cos(dist * M_PI) + 1.0));
}