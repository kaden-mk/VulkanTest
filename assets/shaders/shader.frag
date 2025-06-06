#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragWorldNormal;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

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
    uint textureIndex;
} push;

vec3 gamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

void main()
{
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
    vec3 specularLight = vec3(0.0);
    vec3 surfaceNormal = normalize(fragWorldNormal);

    vec3 cameraWorldPosition = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraWorldPosition - fragWorldPos);

    for (int i = 0; i < ubo.lightCount; i++) {
        PointLight light = ubo.pointLights[i];

        vec3 directionToLight = light.position.xyz - fragWorldPos;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);

        directionToLight = normalize(directionToLight);

        float cosAngle = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngle;

        // specular
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinn = dot(surfaceNormal, halfAngle);
        blinn = clamp(blinn, 0, 1);
        blinn = pow(blinn, 32.0); // higher values = sharper highlight
        specularLight += intensity * blinn;
    }

    vec4 sampledColor = texture(Sampler2D[push.textureIndex], fragUV);
    vec3 imageColor = sampledColor.rgb;
    vec3 color = (diffuseLight * fragColor + specularLight * fragColor) * imageColor;
    vec3 finalColor = gamma(color);

    outColor = vec4(finalColor, 1.0);
}