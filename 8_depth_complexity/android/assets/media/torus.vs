uniform mat4 view_proj_matrix;

attribute vec4 rm_Vertex;

void main(void)
{
   gl_Position = view_proj_matrix * rm_Vertex;
}