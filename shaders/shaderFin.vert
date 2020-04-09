#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 uv;

layout(location = 1) out vec4 fragNormal;
layout(location = 2) out float weight;
layout(location = 3) out mat4 geomProj;
layout(location = 7) out mat4 geomModel;
layout(location = 11) out mat4 geomView;


layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 renderFlags;
} ubo;

layout(binding = 3) uniform ShellUniformBufferObject {
	vec2 data;
} shellUbo;

layout(binding = 2) uniform LightingConstants {
	vec4 lightPos;
	vec3 lightAmbient;
	vec3 lightSpecular;
	vec3 lightDiffuse;
	vec2 lightSpecularExponent;
} lighting;

void main() {
	//vec3 shellVert = inPosition + (inNormal * 3.0);
	//vec4 VCS_position = ubo.view * ubo.model * vec4(shellVert, 1.0);
	// //already in world coords so no need to multiply by model
	// fragLightVector = (ubo.view * lighting.lightPos) - VCS_position;
	//fragNormal = ubo.view * ubo.model * vec4(inNormal, 0.0);
	fragNormal = vec4(inNormal, 0.0);
	//fragEyeVector = - VCS_position;
	gl_Position = vec4(inPosition, 1.0);//ubo.proj * VCS_position;
	geomProj = ubo.proj;
	geomModel = ubo.model;
	geomView = ubo.view;
	
	//fragColor = vec3(1.0, 1.0, 1.0);
	//fragTexCoord = uv;

	// fragSpecularLighting = lighting.lightSpecular;
	// fragDiffuseLighting = lighting.lightDiffuse;
	// fragAmbientLighting = lighting.lightAmbient;
	// fragSpecularCoefficient = lighting.lightSpecularExponent.x;
	// renderFlags = ubo.renderFlags;

}