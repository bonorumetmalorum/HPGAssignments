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
layout(location = 10) in vec4 fragShadowCoord; 

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 4) uniform sampler2D shadowSampler;

//because perspective projection skews the depth values (closer together near and farther apart far away)
//we need to linearize the depth information in order to display
//near = 0.1 far = 1.0 planes
float linearize(float depth)
{
  return (2.0 * 0.1) / (10.0 + 0.1 - depth * (10.0 - 0.1));	
}

void main() {
    float z = texture(shadowSampler, fragTexCoord).r; //get the color of the depth map
    outColor = vec4(vec3(1.0 - linearize(z)), 1.0);  //linearize it so that the colors are represented the right way (would not be needed if light was orthographic since there would be no warping of depth values)
}

