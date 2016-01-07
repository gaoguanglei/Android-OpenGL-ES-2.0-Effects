uniform vec4 vViewPosition;
uniform mat4 world_view_proj_mat;

attribute vec4 rm_Vertex;

varying vec3 verDirection;

void main(void) 
{
   vec4 pos = rm_Vertex;
 
   verDirection = normalize(rm_Vertex.xyz);
   gl_Position = world_view_proj_mat * pos;
}