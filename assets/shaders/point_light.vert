#version 450
#extension GL_EXT_scalar_block_layout : require

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;

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

void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    vec4 lightCamPos = ubo.view * vec4(push.position.xyz, 1.0);
    gl_Position = ubo.projection * (lightCamPos + vec4(push.radius * fragOffset, 0.0, 0.0));
}