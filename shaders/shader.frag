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

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
	vec3 ambientLight = fragAmbientLighting * fragColor;
	vec4 normEyeVector = normalize(fragEyeVector);
	vec4 normLightVector = normalize(fragLightVector);
	vec4 normNormal = normalize(fragNormal);

	vec4 textureColor = texture(texSampler, fragTexCoord);

	float diffuseDotProd = dot(normLightVector, normNormal);
	vec3 diffuseLight = ambientLight * fragColor * diffuseDotProd;

	vec4 reflected = reflect(-normLightVector, normNormal);

	//vec4 halfAngle = normalize((normEyeVector + (-normLightVector))/2.0);
	float specularDotProd = max(dot(reflected, normEyeVector), 0.0);
	float specularPower = pow(specularDotProd, fragSpecularCoefficient);
	vec3 specularLight = fragSpecularLighting * fragColor * specularPower;

	vec3 lighting = ambientLight + diffuseLight + specularLight;
	

	outColor = vec4(lighting, 1.0) * textureColor;
}

