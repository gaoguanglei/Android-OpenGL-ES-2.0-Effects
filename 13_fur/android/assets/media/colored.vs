
float shell_distance = 0.3;

uniform mat4 world_view_proj_matrix;
uniform float pass_index;

attribute vec3 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec2 rm_TexCoord0;

varying vec3 vNormal;
varying vec2  vTexCoord;

void main(void)
{
   vTexCoord    = rm_TexCoord0;
   vNormal = rm_Normal;

   vec4 Position = vec4(rm_Vertex,1.0);
  Position.xyz += shell_distance * pass_index * rm_Normal; 
   gl_Position = world_view_proj_matrix * Position;
}
