#version 450

// most arent used but to avoid warnings
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 tangent;

layout(location = 0) out vec3 localDir;

layout(push_constant) uniform PC {
    mat4 proj;
    mat4 view;
} pc;

void main() {
    localDir = position;
    gl_Position = pc.proj * pc.view * vec4(position, 1.0);
}