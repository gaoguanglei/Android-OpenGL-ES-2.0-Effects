//uniform mat4 view_proj_matrix;

attribute vec4 rm_Vertex;

varying vec2 vTexCoord;

void main(void)
{
 //  gl_Position = view_proj_matrix * rm_Vertex;
   
   // Clean up inaccuracies
   vec2 Pos;
   Pos = sign(rm_Vertex.xy);

   gl_Position = vec4(Pos.xy, 0.0, 1.0);
   // Image-space
   vTexCoord.x = 0.5 * (1.0 + Pos.x);
   vTexCoord.y = 0.5 * (1.0 + Pos.y);

}