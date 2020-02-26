#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec4 fragLightVector;
layout(location = 3) out vec4 fragEyeVector;
layout(location = 4) out vec3 fragSpecularLighting;
layout(location = 5) out vec3 fragDiffuseLighting;
layout(location = 6) out vec3 fragAmbientLighting;
layout(location = 7) out float fragSpecularCoefficient;
layout(location = 8) out vec4 fragNormal;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 2) uniform LightingConstants {
	vec4 lightPos;
	vec3 lightAmbient;
	vec3 lightSpecular;
	vec3 lightDiffuse;
	float lightSpecularExponent;
} lighting;

void main() {
	vec4 VCS_position = ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragLightVector = ubo.view * lighting.lightPos - VCS_position;
	fragNormal = ubo.view * ubo.model * vec4(inNormal, 1.0);
	fragEyeVector = - VCS_position;
	gl_Position = ubo.proj * VCS_position;
	
	fragColor = vec3(1.0, 1.0, 1.0);
	fragTexCoord = uv;

	fragSpecularLighting = lighting.lightSpecular;
	fragDiffuseLighting = lighting.lightDiffuse;
	fragAmbientLighting = lighting.lightAmbient;
}