#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 uv;

//layout(location = 0) out vec3 fragColor;
layout(location = 2) out vec2 fragTexCoord; //keep this one
//layout(location = 2) out vec4 fragLightVector; //get rid
layout(location = 0) out vec4 eyeVector; //keep this one
//dont need lighting stuff for this pipeline
//layout(location = 4) out vec3 fragSpecularLighting; 
//layout(location = 5) out vec3 fragDiffuseLighting;
//layout(location = 6) out vec3 fragAmbientLighting;
//layout(location = 7) out float fragSpecularCoefficient;
layout(location = 1) out vec4 normal;
//layout(location = 9) out vec3 renderFlags; get rid



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

void main() {
	vec4 VCS_position = ubo.view * ubo.model * vec4(inPosition, 1.0);
	// //already in world coords so no need to multiply by model
	// fragLightVector = (ubo.view * lighting.lightPos) - VCS_position;
	normal = ubo.view * ubo.model * vec4(inNormal, 0.0);
	eyeVector = - VCS_position;
	gl_Position = ubo.proj * VCS_position;
	
	//fragColor = vec3(1.0, 1.0, 1.0);
	fragTexCoord = uv;

	// fragSpecularLighting = lighting.lightSpecular;
	// fragDiffuseLighting = lighting.lightDiffuse;
	// fragAmbientLighting = lighting.lightAmbient;
	// fragSpecularCoefficient = lighting.lightSpecularExponent.x;
	// renderFlags = ubo.renderFlags;

}