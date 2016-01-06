#version 150

uniform mat4 u_wvp;
uniform mat4 u_world_mat;
uniform mat4 u_env_inv_world;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;


varying vec3 v_normal;
varying vec2 v_texcoord;

void main(void)
{
  gl_Position = u_wvp * vec4(a_position,1);
  vec3 world_normal = mat3(u_world_mat) * a_normal;
  v_normal = mat3(u_env_inv_world) * world_normal;
  v_normal = normalize(v_normal);
  v_texcoord = vec2(a_texcoord.s, 1.0-a_texcoord.t);
}
