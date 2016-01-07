uniform mat4 view_matrix;
uniform mat4 view_proj_matrix;

float distanceScale = 0.0035;

attribute vec4 rm_Vertex;
attribute vec2 rm_TexCoord0;
attribute vec3 rm_Normal;

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vViewVec;

void main(void)
{
   gl_Position = view_proj_matrix * rm_Vertex;
   vTexCoord = rm_TexCoord0;
   
   // Eye-space lighting
   vNormal = (view_matrix * vec4 (rm_Normal, 0.0)).xyz;
   
   // We multiply with distance scale in the vertex shader
   // instead of the fragment shader to improve performance.
   vViewVec = -vec3(distanceScale * view_matrix * rm_Vertex);

}