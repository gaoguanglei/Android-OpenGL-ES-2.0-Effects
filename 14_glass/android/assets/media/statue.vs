
#version 150
uniform mat4 u_wvp;
uniform mat4 u_world_mat;
uniform mat4 u_env_inv_world;
uniform vec3 eye;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;


varying vec3 v_normal;
varying vec3 v_view;
void main(void)
{
   gl_Position = u_wvp * vec4(a_position,1);
  vec3 world_normal = mat3(u_world_mat) * a_normal;
  vec4 world_pos =u_world_mat * vec4(a_position,1.0);
  v_normal = mat3(u_env_inv_world) * world_normal;
  
  vec4 inv_world_pos = u_env_inv_world * world_pos;
  v_view = normalize(eye - inv_world_pos.xyz);
}
