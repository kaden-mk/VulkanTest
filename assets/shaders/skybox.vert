#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 localDir;

layout(push_constant) uniform PC {
    mat4 proj;
    mat4 view;
} pc;

void main() {
    localDir = inPosition;
    gl_Position = pc.proj * pc.view * vec4(inPosition, 1.0);
}