#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec4 fragLightVector;
layout(location = 3) in vec4 fragEyeVector;
layout(location = 4) in vec3 fragSpecularLighting;
layout(location = 5) in vec3 fragDiffuseLighting;
layout(location = 6) in vec3 fragAmbientLighting;
layout(location = 7) in float fragSpecularCoefficient;
layout(location = 8) in vec4 fragNormal;
layout(location = 9) in vec3 renderFlags;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler3D texSampler;
//layout(binding = 1) uniform sampler2D texSampler;

void main() {
	vec4 skinColor = vec4(0.4941, 0.0392, 0.0392, 1.0);//texture(texSampler, fragTexCoord);

	float ambI = 0.8;
	float diffI = 0.2;

	vec3 ambientLight = fragColor * ambI;
	vec4 normEyeVector = normalize(fragEyeVector);
	vec4 normLightVector = normalize(fragLightVector);
	vec4 normNormal = normalize(fragNormal);

	float diffuseDotProd = max(dot(normLightVector, normNormal), 0.0);
	float diffuseLight = diffuseDotProd * diffI;

	vec4 lighting = vec4(0);
	if(abs(renderFlags.x) > 0.5){
		lighting += vec4(ambientLight, 1.0) * vec4(fragColor, 1.0);
	}
	if(abs(renderFlags.y) > 0.5){
		lighting += diffuseLight * vec4(fragColor, 1.0);
	}
	
	//outColor = vec4(renderFlags, 1.0);
	outColor = lighting * skinColor;
}

