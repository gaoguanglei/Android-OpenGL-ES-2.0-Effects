#version 150
uniform mat4 view_proj_matrix;
uniform mat4 inv_view_matrix;
uniform mat4 light_matrix;
//uniform mat4 inv_world_matrix;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec3 rm_Tangent;
attribute vec3 rm_Binormal;
attribute vec2 rm_TexCoord0;

varying vec2  vTexCoord;
varying vec3  vLightVec;
varying vec3  vViewVec;
varying vec3  vDir;

vec4 lightPos = vec4(0,0,30.0,1);

void main(void)
{
   
   gl_Position = view_proj_matrix * vec4(rm_Vertex,1.0);
   
   vTexCoord = vec2(rm_TexCoord0);

   // Transform light vector into tangent space

   vec3 lightVec = lightPos.xyz - rm_Vertex;
   vLightVec.x = dot(lightVec, rm_Tangent);
   vLightVec.y = dot(lightVec, rm_Binormal);
   vLightVec.z = dot(lightVec, rm_Normal);

   // Transform view vector into tangent space
   vec4 view_position = inv_view_matrix * vec4(0.0,0.0,0.0,1);

   vec3 viewVec = view_position.xyz - rm_Vertex;
   vViewVec.x = dot(viewVec, rm_Tangent);
   vViewVec.y = dot(viewVec, rm_Binormal);
   vViewVec.z = dot(viewVec, rm_Normal);

   // Direction to look up our fancy shading at
   vec3 dir = -lightVec;

   // Create a rotation matrix from three Euler angles.
   // This work is better done on the CPU as it is constant for the
   // whole scene. In RenderMonkey we'll have to do it in the shader though.
  vDir = mat3(light_matrix) * dir;
}
