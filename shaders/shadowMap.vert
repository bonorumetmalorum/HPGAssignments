#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform ShadowUniformObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 renderFlags;
} suo;
//transform the position to light space and pass it on to the later stages
void main() {
    vec4 WCS_position = suo.model * vec4(inPosition, 1.0);
    vec4 LCS_position = suo.view * WCS_position;
    fragColor = vec4(0.0,0.0,0.0,1.0);
    gl_Position = suo.proj * LCS_position;
}