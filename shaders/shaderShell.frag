#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2) in vec4 fragLightVector;
layout(location = 3) in vec4 fragEyeVector;
layout(location = 11) in vec4 tangent;

layout(location = 4) in vec3 fragSpecularLighting;
layout(location = 5) in vec3 fragDiffuseLighting;
layout(location = 6) in vec3 fragAmbientLighting;
layout(location = 7) in float fragSpecularCoefficient;
layout(location = 8) in vec4 fragNormal;
layout(location = 9) in vec3 renderFlags;
layout(location = 10) in float opacity;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
	vec4 textureColor = texture(texSampler, fragTexCoord);
	textureColor.rgb = vec3(0.4941, 0.0392, 0.0392);
	textureColor.a *= opacity;
	float ambI = 0.3;
	float diffI = 0.7;
	float specI = 0.2;

	vec4 normEyeVector = normalize(fragEyeVector);
	vec4 normLightVector = normalize(fragLightVector);
	vec4 normNormal = normalize(fragNormal);
	vec3 normTangent = normalize(tangent.xyz);

	float TdotL = max(dot( normTangent , normLightVector.xyz), 0.0);
    float TdotE = max(dot( normTangent , normEyeVector.xyz), 0.0);
    float sinTL = sqrt( 1 - TdotL*TdotL );
    float sinTE = sqrt( 1 - TdotE*TdotE );

	textureColor.xyz += vec3(1.0,1.0,1.0) * (ambI*textureColor.xyz) + (diffI*sinTL*textureColor.xyz) + 
       (specI*pow( abs((TdotL*TdotE + sinTL*sinTE)),20.0));

	outColor = textureColor;
}

