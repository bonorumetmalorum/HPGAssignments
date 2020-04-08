#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

layout(location = 0) in vec3 eyeVector[];
layout(location = 1) in vec4 normal[];

void main()
{
    //calculate the triangle normal by using adjacent vertices
    
}