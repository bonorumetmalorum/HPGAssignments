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
layout(location = 9) out vec3 renderFlags;
layout(location = 10) out vec4 fragShadowCoord;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 renderFlags;
} ubo;

layout(binding = 2) uniform LightingConstants {
	vec4 lightPos;
	vec3 lightAmbient;
	vec3 lightSpecular;
	vec3 lightDiffuse;
	vec2 lightSpecularExponent;
} lighting;

layout(binding = 3) uniform ShadowUniformObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 renderFlags;
} suo;
//needed for persepctive projection
const mat4 bias = mat4(        
	0.5, 0.0, 0.0, 0.0,       
	 0.0, 0.5, 0.0, 0.0,        
	 0.0, 0.0, 1.0, 0.0,        
	 0.5, 0.5, 0.0, 1.0 );

void main() {
	vec4 VCS_position = ubo.view * ubo.model * vec4(inPosition, 1.0);

	vec4 WCS_position = ubo.model * vec4(inPosition, 1.0);
	vec4 LCS_position = suo.view * WCS_position;

	fragShadowCoord = bias * suo.proj * LCS_position;

	//already in world coords so no need to multiply by model
	fragLightVector = (ubo.view * lighting.lightPos) - VCS_position;
	fragNormal = ubo.view * ubo.model * vec4(inNormal, 0.0);
	fragEyeVector = - VCS_position;
	gl_Position = ubo.proj * VCS_position;
	
	fragColor = vec3(1.0, 1.0, 1.0);
	fragTexCoord = uv;

	fragSpecularLighting = lighting.lightSpecular;
	fragDiffuseLighting = lighting.lightDiffuse;
	fragAmbientLighting = lighting.lightAmbient;
	fragSpecularCoefficient = lighting.lightSpecularExponent.x;
	renderFlags = ubo.renderFlags;
}