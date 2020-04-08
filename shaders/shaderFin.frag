#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(location = 0) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
//layout(location = 2) in vec4 fragLightVector;
//layout(location = 3) in vec4 eyeVector;
//layout(location = 4) in vec3 fragSpecularLighting;
//layout(location = 5) in vec3 fragDiffuseLighting;
//layout(location = 6) in vec3 fragAmbientLighting;
//layout(location = 7) in float fragSpecularCoefficient;
//layout(location = 8) in vec4 fragNormal;
//layout(location = 9) in vec3 renderFlags;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
	vec4 tex = texture(texSampler, fragTexCoord);
	outColor = tex;
}

