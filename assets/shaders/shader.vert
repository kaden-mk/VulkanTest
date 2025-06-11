#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 tangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragWorldNormal;
layout(location = 3) out vec2 fragUV;
layout(location = 4) out mat3 fragTBN;

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

layout(set = 0, binding = 1) uniform sampler2D Sampler2D[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
    uint bufferIndex;
    uint materialIndex;
} push;

void main() {
    vec3 worldNormal = normalize(mat3(push.normalMatrix) * normal);
    vec3 worldTangent = normalize(mat3(push.normalMatrix) * tangent.xyz);
    vec3 worldBitangent = normalize(cross(worldNormal, worldTangent)) * tangent.w;

    fragWorldNormal = worldNormal;
    fragWorldPos = (push.modelMatrix * vec4(position, 1.0)).xyz;
    fragColor = color;
    fragUV = uv;
    fragTBN = mat3(worldTangent, worldBitangent, worldNormal);

    gl_Position = ubo.projection * ubo.view * vec4(fragWorldPos, 1.0);
}