#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragWorldNormal;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in mat3 fragTBN;

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

layout(set = 0, binding = 1) uniform sampler2D Sampler2D[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
    uint bufferIndex;
    uint materialIndex;
} push;

struct Material {
    uint albedoIndex;
    uint normalIndex;
};

layout(set = 0, binding = 3) buffer SSBO {
    Material materials[];
} ssbo;

vec3 gammaCorrect(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main() {
    Material material = ssbo.materials[push.materialIndex];

    vec3 normalMap = texture(Sampler2D[nonuniformEXT(material.normalIndex)], fragUV).rgb;
    vec3 surfaceNormal = normalize(fragTBN * (normalMap * 2.0 - 1.0));

    vec3 viewDir = normalize(ubo.inverseView[3].xyz - fragWorldPos);
    vec3 diffuseLight = ubo.ambientLightColor.rgb * ubo.ambientLightColor.a;
    vec3 specularLight = vec3(0.0);

    for (int i = 0; i < ubo.lightCount; ++i) {
        PointLight light = ubo.pointLights[i];
        vec3 lightDir = light.position.xyz - fragWorldPos;
        float dist2 = dot(lightDir, lightDir);
        float attenuation = 1.0 / dist2;
        lightDir = normalize(lightDir);

        float diffuse = max(dot(surfaceNormal, lightDir), 0.0);
        vec3 halfVec = normalize(lightDir + viewDir);
        float spec = pow(max(dot(surfaceNormal, halfVec), 0.0), 32.0);

        vec3 lightIntensity = light.color.rgb * light.color.a * attenuation;
        diffuseLight += lightIntensity * diffuse;
        specularLight += lightIntensity * spec;
    }

    vec3 lightDir = normalize(ubo.sunDirection);

    float diffuse = max(dot(surfaceNormal, lightDir), 0.0);
    vec3 halfVec = normalize(lightDir + viewDir);
    float spec = pow(max(dot(surfaceNormal, halfVec), 0.0), 32.0);

    vec3 sunColor = vec3(1.0, 0.95, 0.85);
   
    float angle = dot(surfaceNormal, lightDir);
    float sunFade = smoothstep(cos(ubo.sunSize), 1.0, angle);

    diffuseLight += sunColor * ubo.sunIntensity * diffuse * sunFade;
    specularLight += sunColor * ubo.sunIntensity * spec;
    
    vec3 albedo = texture(Sampler2D[nonuniformEXT(material.albedoIndex)], fragUV).rgb;
    vec3 finalColor = gammaCorrect((diffuseLight + specularLight) * albedo * fragColor);
    outColor = vec4(finalColor, 1.0);
}