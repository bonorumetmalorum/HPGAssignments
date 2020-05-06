#version 450 core
layout(location = 0) out vec4 fColor;

layout (binding = 0) uniform sampler2D fSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

void main()
{
    fColor = inColor * texture(fSampler, inUV.st);
}