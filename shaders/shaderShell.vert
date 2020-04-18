#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;
layout(location = 2) out vec4 fragLightVector;
layout(location = 3) out vec4 fragEyeVector;
layout(location = 4) out vec3 fragSpecularLighting;
layout(location = 5) out vec3 fragDiffuseLighting;
layout(location = 6) out vec3 fragAmbientLighting;
layout(location = 7) out float fragSpecularCoefficient;
layout(location = 8) out vec4 fragNormal;
layout(location = 9) out vec3 renderFlags;
layout(location = 10) out float opacity;
layout(location = 11) out vec4 tangent;
layout(location = 12) out float shellLevel;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 renderFlags;
} ubo;

layout(binding = 3) uniform ShellUniformBufferObject {
	vec3 data;
} shellUbo;

layout(binding = 2) uniform LightingConstants {
	vec4 lightPos;
	vec3 lightAmbient;
	vec3 lightSpecular;
	vec3 lightDiffuse;
	vec2 lightSpecularExponent;
} lighting;

void main() {
	vec3 normNormal = normalize(inNormal);
	vec3 shellVert = inPosition + (normNormal * shellUbo.data.x);
	vec3 tangentVert = shellVert - inPosition;
	tangent = ubo.view * ubo.model * vec4(tangentVert, 1.0);
	vec4 VCS_position = ubo.view * ubo.model * vec4(shellVert, 1.0);
	//already in world coords so no need to multiply by model
	fragLightVector = (ubo.view * lighting.lightPos) - VCS_position;
	fragNormal = ubo.view * ubo.model * vec4(inNormal, 0.0);
	fragEyeVector = - VCS_position;
	gl_Position = ubo.proj * VCS_position;
	
	fragColor = vec3(1.0, 1.0, 1.0);
	fragTexCoord = vec3(uv, shellUbo.data.z);

	fragSpecularLighting = lighting.lightSpecular;
	fragDiffuseLighting = lighting.lightDiffuse;
	fragAmbientLighting = lighting.lightAmbient;
	fragSpecularCoefficient = lighting.lightSpecularExponent.x;
	renderFlags = ubo.renderFlags;
	opacity = shellUbo.data.y;
	shellLevel = shellUbo.data.z;
}