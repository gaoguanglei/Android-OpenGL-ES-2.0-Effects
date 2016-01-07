#version 150
uniform mat4 world_view_proj_mat;
uniform mat4 world_view_mat;
// Vertex shader for erosion effect

vec3 LightPosition = vec3(0.0,0.0,4.0);
float Scale = 0.02;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;

varying float LightIntensity;
varying vec3 Position;

void main(void) 
{
	vec3 vertex_pos = vec3(-rm_Vertex.x,-rm_Vertex.z,rm_Vertex.y);
	vec3 vertex_normal = vec3(-rm_Normal.x,-rm_Normal.z, rm_Normal.y);
   vec4 pos       = world_view_mat * vec4(vertex_pos,1.0);
   Position       = vec3(rm_Vertex) * Scale;
   vec3 tnorm     = normalize(mat3(world_view_mat) * vertex_normal);
   float dotval   = abs(dot(normalize(LightPosition - vec3(pos)), tnorm));
   LightIntensity = dotval * 1.5;
   gl_Position    = world_view_proj_mat * vec4(vertex_pos,1.0);
}