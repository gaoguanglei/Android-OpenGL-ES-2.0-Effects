
float shell_distance = 0.3;

uniform mat4 world_view_proj_matrix;
uniform float pass_index;

attribute vec4 rm_Vertex;
attribute vec3 rm_Normal;
attribute vec3 rm_TexCoord0;

varying vec2  vTexCoord;

void main(void)
{
   vTexCoord    = rm_TexCoord0.xy;
   
   vec4 Position = rm_Vertex;
   Position.xyz += shell_distance * pass_index * rm_Normal; 
   gl_Position = world_view_proj_matrix * Position;
}