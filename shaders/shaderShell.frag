#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 2) in vec4 fragLightVector;
layout(location = 3) in vec4 fragEyeVector;

layout(location = 4) in vec3 fragSpecularLighting;
layout(location = 5) in vec3 fragDiffuseLighting;
layout(location = 6) in vec3 fragAmbientLighting;
layout(location = 7) in float fragSpecularCoefficient;
layout(location = 8) in vec4 fragNormal;
layout(location = 9) in vec3 renderFlags;
layout(location = 10) in float opacity;
layout(location = 11) in float shellLevel;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler3D texSampler;
//layout(binding = 1) uniform sampler2D texSampler;


void main() {
	vec4 textureColor = texture(texSampler, fragTexCoord);
	//vec4 textureColor = texture(texSampler, fragTexCoord.xy);
	textureColor.rgb = vec3(0.4941, 0.0392, 0.0392);
	textureColor.a *= opacity;
	float ambI = 0.3;
	float diffI = 0.4;
	float specI = 0.2;

	vec4 normEyeVector = normalize(fragEyeVector);
	vec4 normLightVector = normalize(fragLightVector);
	vec4 normNormal = normalize(fragNormal);
	
	//kajiya kay lighting computation for l'oreal hair as seen in nvidia paper
	float tangentdlight = max(dot( normNormal.xyz , normLightVector.xyz), 0.0);
    float tangentdeye = max(dot( normNormal.xyz , normEyeVector.xyz), 0.0);
    float sintangentlight = sqrt( 1 - tangentdlight*tangentdlight );
    float sintangeneye = sqrt( 1 - tangentdeye*tangentdeye );

	vec3 ambientTerm = (ambI*textureColor.xyz);
	vec3 diffterm = (diffI*sintangentlight*textureColor.xyz);
	vec3 specTerm = (specI*pow( abs((tangentdlight*tangentdeye + sintangentlight*sintangeneye)),10.0)) * fragColor;

	textureColor.xyz += vec3(1.0,1.0,1.0) * ambientTerm + diffterm + specTerm;

	//selfshadowing
    float minShadow = 0.8;
    float shadow = (float(shellLevel)/float(8))*(1-minShadow) + minShadow;

	textureColor.xyz *= shadow;

	outColor = textureColor;
}

