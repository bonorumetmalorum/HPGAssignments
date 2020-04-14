#version 450

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 6) out;

layout(location = 1) in vec4 fragNormal[];
layout(location = 2) in float weight[];
layout(location = 3) in mat4 geomProj[];
layout(location = 7) in mat4 geomModel[];
layout(location = 11) in mat4 geomView[];

layout(location = 0) out vec4 normalOut;
layout(location  = 1) out vec2 texCoord;

void main()
{
    vec4 VCS_position0 = (geomView[0] * geomModel[0] * gl_in[0].gl_Position);
    vec4 VCS_position1 = (geomView[0] * geomModel[0] * gl_in[1].gl_Position);
    vec4 VCS_position2 = (geomView[0] * geomModel[0] * gl_in[2].gl_Position);
    vec4 VCS_position3 = (geomView[0] * geomModel[0] * gl_in[3].gl_Position);

    vec3 eyeVector = -VCS_position1.xyz;
    vec3 N1, N2;

    N1 = normalize(cross(VCS_position2.xyz - VCS_position1.xyz, VCS_position0.xyz - VCS_position1.xyz));
    N2 = normalize(cross(VCS_position3.xyz - VCS_position1.xyz, VCS_position2.xyz - VCS_position1.xyz));
    
    float eyeDotN1, eyeDotN2;
    eyeDotN1 = dot(N1, eyeVector);
    eyeDotN2 = dot(N2, eyeVector);

    if(eyeDotN1 * eyeDotN2 < 0)
    { //1 is right and 2 is left

      vec3 tr = gl_in[1].gl_Position.xyz + (normalize(fragNormal[1].xyz) * weight[0]);
      vec4 temp = geomProj[1] *  geomView[1] * geomModel[1] * vec4(tr, 1.0);
      gl_Position = temp;
      normalOut = geomModel[1] * geomView[1] * fragNormal[1];
      texCoord = vec2(0, 0); 
      EmitVertex();
      temp = geomProj[1] *  geomView[1] * geomModel[1] * gl_in[1].gl_Position;
      gl_Position = temp;
      texCoord = vec2(0,1);
      normalOut = geomModel[0] * geomView[0] * fragNormal[1];
      EmitVertex(); 
      temp = geomProj[1] *  geomView[1] * geomModel[1] * gl_in[2].gl_Position;
      gl_Position = temp;
      texCoord = vec2(1,1); 
      normalOut = geomModel[0] * geomView[0] * fragNormal[2];
      EmitVertex();

      EndPrimitive(); //---------------------------------------------------------------------
      //top left
      vec3 tl = gl_in[2].gl_Position.xyz + (normalize(fragNormal[2].xyz) * weight[0]);
      temp = geomProj[1] *  geomView[1] * geomModel[1] * vec4(tl, 1.0);;
      gl_Position = temp;
      normalOut = geomModel[1] * geomView[1] * fragNormal[2];
      texCoord = vec2(1, 0); 
      EmitVertex();
      //top right
      temp = geomProj[1] *  geomView[1] * geomModel[1] * vec4(tr, 1.0);
      gl_Position = temp;
      normalOut = geomModel[1] * geomView[1] * fragNormal[1];
      texCoord = vec2(0,0); 
      EmitVertex();
      //original left
      temp = geomProj[1] *  geomView[1] * geomModel[1] * gl_in[2].gl_Position;
      gl_Position = temp;
      texCoord = vec2(1,1); 
      normalOut = geomModel[0] * geomView[0] * fragNormal[2];
      EmitVertex();
      EndPrimitive();
  }
}