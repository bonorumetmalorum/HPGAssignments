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
layout(location = 9) in vec3 renderFlags; //render flags 0 enable/disable PCF, 1 enable / disable texture with shadow map, 3 disable / enable ambient only, no shadows
layout(location = 10) in vec4 fragShadowCoord; 

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 4) uniform sampler2D shadowSampler;

//because perspective projection skews the depth values (closer together near and farther apart far away)
//we need to linearize the depth information in order to display
float linearize(float depth)
{
  return (2.0 * 0.1) / (10.0 + 0.1 - depth * (10.0 - 0.1));	
}

void main() {
	vec4 textureColor = texture(texSampler, fragTexCoord);
	textureColor.a = 1.0;
	vec4 shadow_coords = fragShadowCoord / fragShadowCoord.w;
	float ambI = 0.25;
	float diffI = 0.9;
	float specI = 4.0;
	//if we want to display the shadow map as a texture
	if(renderFlags[1] > 0.5)
	{
		float depth = texture(shadowSampler, shadow_coords.xy).r;
		vec4 shadow = vec4(vec3(1.0 - linearize(depth), 1.0 - linearize(depth), 1.0 - linearize(depth)), 1.0);
		vec4 col = shadow;
		outColor = col;
		return; //return, no point doing anything else
	}
	//lighting calculations
	vec3 ambientLight = fragAmbientLighting * fragColor * ambI;
	vec4 normEyeVector = normalize(fragEyeVector);
	vec4 normLightVector = normalize(fragLightVector);
	vec4 normNormal = normalize(fragNormal);

	float diffuseDotProd = max(dot(normLightVector, normNormal), 0.0);
	vec3 diffuseLight = fragDiffuseLighting * diffuseDotProd * diffI;

	vec4 halfAngle = normalize((normEyeVector + normLightVector)/2.0);
	float specularDotProd = max(dot(halfAngle, normNormal), 0.0);
	float specularPower = pow(specularDotProd, fragSpecularCoefficient);
	vec3 specularLight = fragSpecularLighting * fragColor * specularPower * specI;

	//add ambient
	vec4 lighting = vec4(0);
	lighting += vec4(ambientLight, 1.0) * vec4(fragColor, 1.0);

	//if we dont want shadow and only ambient
	if(renderFlags[2] > 0.5)
	{
		outColor = lighting * textureColor;
		return; //return no point doing anything more
	}

	//current fragment depth
	float fragdepth = shadow_coords.z;

	float shadowAmount = 0.0;
	if(renderFlags[0] > 0.5)
	{
		//PCF
		ivec2 shadowMapDim = textureSize(shadowSampler, 0);
		vec2 texelSize = 1.0/shadowMapDim;
		int count;
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y < 1; y++)
			{
				float pcf = texture(shadowSampler, shadow_coords.xy + vec2(x, y) * texelSize).r;
				shadowAmount += fragdepth < pcf ? 1.0 : 0.0;
				count++;
			}
		}
		shadowAmount /= 9.0;
		//PCF
	}
	else
	{
		shadowAmount = (texture(shadowSampler, shadow_coords.st).r > fragdepth ? 1.0 : 0.0);
	}

	lighting += vec4(diffuseLight, 1.0) * vec4(fragColor, 1.0) * shadowAmount;
	lighting += vec4(specularLight,1.0) * vec4(fragColor, 1.0) * shadowAmount;

	
	//outColor = vec4(1.0,1.0,1.0, 1.0);
	outColor = lighting * textureColor;
	//outColor = vec4(texture(shadowSampler, fragShadowCoord.st).r, texture(shadowSampler, fragShadowCoord.st).r, texture(shadowSampler, fragShadowCoord.st).r, 1.0);
	//outColor = vec4(.xyz, 1.0);
}

